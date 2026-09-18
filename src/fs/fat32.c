#include "globals.h"
#include "fs/fat32.h"

// https://elm-chan.org/fsw/ff/
// https://www.ecma-international.org/publications-and-standards/standards/ecma-107/
// https://en.wikipedia.org/wiki/Design_of_the_FAT_file_sys

// ok i did a huge ass rewrite yay
// LBA 0 - volume boot record (BPB)
// LBA 1 - FSInfo sector
// LBA 6 - backup boot record
// LBA 7 - backup FSInfo esctor
// reserved (32) - first FAT copy
// reserved+fat_size - second FAT copy
// reserved+2*fat_size - data region, cluster 2 = root dir

// roit dir is a normal cluster chain which starts at cluster 2
// files and dirs are stored in cluster chains
// directories are 32 byte entries + VFAt long names

#define MAX_LFN_SETS 20
typedef struct __attribute__((packed)) {
	nat8 jump[3];
	nat8 oem[8];
	nat16 bytes_per_sector;
	nat8 sectors_per_cluster;
	nat16 reserved_sectors;
	nat8 num_fats;
	nat16 root_entry_count; // 0 for FAT32
	nat16 total_sectors16;	// 0 for FAT32
	nat8 media_descriptor;
	nat16 fat_size16; // 0 for FAT32
	nat16 sectors_per_track;
	nat16 num_heads;
	nat32 hidden_sectors;
	nat32 total_sectors32;
	nat32 fat_size32;
	nat16 ext_flags;
	nat16 fs_version;		  // 0
	nat32 root_cluster;		  // 2 for FAT32
	nat16 fsinfo_sector;	  // 1
	nat16 backup_boot_sector; // 6
	nat8 reserved[12];
	nat8 drive_number;
	nat8 reserved2;
	nat8 boot_signature; // 0x29
	nat32 volume_id;
	nat8 volume_label[11];
	nat8 fat_type[8]; // "FAT32   "
	nat8 boot_code[420];
	nat16 signature; // 0x55 0xAA
} fat32_bpb_t;

// driver state

static nat8 fs_ready = 0;
static nat32 spc = 1; // sectors per cluster
static nat32 num_fats = 2;
static nat32 fat_lba = 32;		 // start of the first FAT copy
static nat32 fat_size = 1024;	 // sectors per FAT copy
static nat32 data_lba = 0;		 // start of the data region
static nat32 data_clusters = 0;	 // total data clusters on the disk
static nat32 next_free_hint = 3; // where we last allocated from

// low level IO

static void write_sector_lba(nat32 lba, const nat8* buffer) { ata_write_sector(lba, buffer); }
static nat8 read_sector_lba(nat32 lba, nat8* buffer) { return ata_read_sector(lba, buffer); }
static nat32 cluster_to_lba(nat32 cluster) { return data_lba + (cluster - 2) * spc; }

static nat8 read_cluster(nat32 cluster, nat8* buffer) {
	nat32 lba = cluster_to_lba(cluster);
	for (nat32 s = 0; s < spc; s++)
		if (!read_sector_lba(lba + s, buffer + s * SECTOR_SIZE))
			return 0;
	return 1;
}

static void write_cluster(nat32 cluster, const nat8* buffer) {
	nat32 lba = cluster_to_lba(cluster);
	for (nat32 s = 0; s < spc; s++)
		write_sector_lba(lba + s, buffer + s * SECTOR_SIZE);
}

// FAT table ops

static nat32 fat_read_entry(nat32 cluster) {
	nat32 off = cluster * 4;
	nat32 lba = fat_lba + off / SECTOR_SIZE;
	nat32 in = off % SECTOR_SIZE;

	nat8 buf[SECTOR_SIZE];
	if (!read_sector_lba(lba, buf))
		return FAT32_EOC;

	nat32 val;
	if (in <= SECTOR_SIZE - 4) {
		memcpy(&val, buf + in, 4);
	} else {
		nat8 b2[SECTOR_SIZE];
		if (!read_sector_lba(lba + 1, b2))
			return FAT32_EOC;
		nat8 tmp[4];
		nat32 first = SECTOR_SIZE - in;
		memcpy(tmp, buf + in, first);
		memcpy(tmp + first, b2, 4 - first);
		memcpy(&val, tmp, 4);
	}
	return val & 0x0FFFFFFF;
}

static void fat_write_entry(nat32 cluster, nat32 value) {
	nat32 off = cluster * 4;
	nat32 lba = fat_lba + off / SECTOR_SIZE;
	nat32 in = off % SECTOR_SIZE;

	for (nat32 copy = 0; copy < num_fats; copy++) {
		nat32 base = lba + copy * fat_size;
		nat8 b1[SECTOR_SIZE];
		read_sector_lba(base, b1);
		if (in <= SECTOR_SIZE - 4) {
			memcpy(b1 + in, &value, 4);
			write_sector_lba(base, b1);
		} else {
			nat8 b2[SECTOR_SIZE];
			read_sector_lba(base + 1, b2);
			nat32 first = SECTOR_SIZE - in;
			memcpy(b1 + in, &value, first);
			memcpy(b2, (nat8*)&value + first, 4 - first);
			write_sector_lba(base, b1);
			write_sector_lba(base + 1, b2);
		}
	}
}

// scan [from, to) for a free FAT entry, caching the current sector
static nat32 fat_scan_free(nat32 from, nat32 to) {
	nat8 buf[SECTOR_SIZE];
	nat32 cur_lba = 0xFFFFFFFF;

	for (nat32 c = from; c < to; c++) {
		nat32 lba = fat_lba + (c * 4) / SECTOR_SIZE;
		if (lba != cur_lba) {
			if (!read_sector_lba(lba, buf))
				return 0;
			cur_lba = lba;
		}
		nat32 in = (c * 4) % SECTOR_SIZE;
		nat32 val;
		if (in <= SECTOR_SIZE - 4) {
			memcpy(&val, buf + in, 4);
		} else {
			nat8 b2[SECTOR_SIZE];
			if (!read_sector_lba(lba + 1, b2))
				return 0;
			nat8 tmp[4];
			nat32 first = SECTOR_SIZE - in;
			memcpy(tmp, buf + in, first);
			memcpy(tmp + first, b2, 4 - first);
			memcpy(&val, tmp, 4);
		}
		if ((val & 0x0FFFFFFF) == 0)
			return c;
	}
	return 0;
}

static nat32 alloc_cluster(void) {
	nat32 end = data_clusters + 2;
	nat32 from = next_free_hint;
	if (from < 2 || from >= end)
		from = 2;

	nat32 found = fat_scan_free(from, end);
	if (!found && from > 2)
		found = fat_scan_free(2, from);
	if (!found)
		return 0; // disk full

	fat_write_entry(found, FAT32_EOC);
	next_free_hint = found + 1;
	if (next_free_hint >= end)
		next_free_hint = 2;
	return found;
}

static void free_cluster_chain(nat32 start) {
	if (start < 2)
		return;
	nat32 c = start;
	for (nat32 guard = 0; guard < data_clusters + 2; guard++) {
		nat32 next = fat_read_entry(c);
		fat_write_entry(c, 0);
		if (next >= 0x0FFFFFF8)
			break;
		if (next < 2 || next >= data_clusters + 2)
			break;
		c = next;
	}
}

// directory entry helpers

#define DIR_MAX_CLUSTERS 256
#define ENTRIES_PER_SECTOR (SECTOR_SIZE / 32)

static nat32 entry_cluster(const fat32_dir_entry_t* e) {
	return ((nat32)e->first_cluster_hi << 16) | e->first_cluster_lo;
}

static void entry_set_cluster(fat32_dir_entry_t* e, nat32 c) {
	e->first_cluster_lo = (nat16)(c & 0xFFFF);
	e->first_cluster_hi = (nat16)((c >> 16) & 0xFFFF);
}

// whole directory loaded into RAM: entries + the cluster chain it lives on, so we can write it all back out..
typedef struct {
	fat32_dir_entry_t* entries; // malloc'd, capacity entries
	nat32 count;				// entries in use (before 0x00)
	nat32 capacity;				// clusters * ENTRIES_PER_SECTOR
	nat32 clusters[DIR_MAX_CLUSTERS];
	nat32 num_clusters;
} dir_t;

// 8.3 short names + checksum

static nat8 upchar(nat8 c) { return (c >= 'a' && c <= 'z') ? (c - 'a' + 'A') : c; }

static nat8 legal_short_char(char c) {
	if (c >= 'A' && c <= 'Z')
		return 1;
	if (c >= '0' && c <= '9')
		return 1;
	switch (c) {
	case '$':
	case '%':
	case '\'':
	case '-':
	case '_':
	case '@':
	case '~':
	case '`':
	case '!':
	case '(':
	case ')':
	case '{':
	case '}':
	case '^':
	case '#':
	case '&':
		return 1;
	}
	return 0;
}

// LFN checksum over the 11 bytes of the short name
static nat8 short_name_checksum(const nat8* short11) {
	nat8 sum = 0;
	for (int i = 0; i < 11; i++)
		sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + short11[i];
	return sum;
}

// uppercase 8.3 version of a long name
static void make_short_name(const char* name, nat8* out) {
	const char* dot = NULL;
	for (const char* p = name; *p; p++)
		if (*p == '.')
			dot = p;

	nat32 base_len = dot ? (nat32)(dot - name) : strlen(name);

	nat8 base[8];
	nat32 bn = 0;
	for (nat32 i = 0; i < base_len && bn < 8; i++) {
		nat8 c = upchar((nat8)name[i]);
		if (legal_short_char((char)c))
			base[bn++] = c;
	}
	if (bn == 0)
		base[bn++] = 'F'; // dotfiles: ".hidden" = "F" base

	nat8 ext[3];
	nat32 en = 0;
	if (dot)
		for (const char* p = dot + 1; *p && en < 3; p++) {
			nat8 c = upchar((nat8)*p);
			if (legal_short_char((char)c))
				ext[en++] = c;
		}

	for (nat32 i = 0; i < 8; i++)
		out[i] = i < bn ? base[i] : ' ';
	for (nat32 i = 0; i < 3; i++)
		out[8 + i] = i < en ? ext[i] : ' ';
	if (out[0] == FAT32_DELETED)
		out[0] = 0x05; // Guess what this means
}

static nat8 short_name_in_use(const dir_t* d, const nat8* short11) {
	for (nat32 i = 0; i < d->count; i++) {
		const fat32_dir_entry_t* e = &d->entries[i];
		if (e->name[0] == 0x00 || e->name[0] == FAT32_DELETED)
			continue;
		if (e->attr == FAT32_ATTR_LFN)
			continue;
		if (memcmp(e->name, short11, 11) == 0)
			return 1;
	}
	return 0;
}

static void build_short_name(const dir_t* d, const char* name, nat8* short11) {
	make_short_name(name, short11);
	nat32 alt = 0;
	while (short_name_in_use(d, short11)) {
		alt++;
		if (alt > 9)
			break;
		// first 6 chars + ~N
		short11[6] = '~';
		short11[7] = (nat8)('0' + alt);
	}
}

// decode an 11 byte short name into NAME.EXT
static void short_decode(const nat8* n, char* out) {
	char base[9];
	nat32 bn = 0;
	while (bn < 8 && n[bn] != ' ') {
		base[bn] = (char)n[bn];
		bn++;
	}
	base[bn] = 0;

	char ext[4];
	nat32 en = 0;
	while (en < 3 && n[8 + en] != ' ') {
		ext[en] = (char)n[8 + en];
		en++;
	}
	ext[en] = 0;

	if (en) {
		strcpy(out, base);
		strcat(out, ".");
		strcat(out, ext);
	} else
		strcpy(out, base);
}

static int strcasecmp_os(const char* a, const char* b) {
	while (*a && *b) {
		nat8 ca = (nat8)*a, cb = (nat8)*b;
		ca = upchar(ca);
		cb = upchar(cb);
		if (ca != cb)
			return (int)ca - (int)cb;
		a++;
		b++;
	}
	return (int)(nat8)*a - (int)(nat8)*b;
}

// LFN (VFAT) ops

// char index 0 to 12 to byte offset in a 32 byte LFN entry
static nat32 lfn_char_offset(nat32 i) {
	if (i < 5)
		return 1 + i * 2;
	if (i < 11)
		return 14 + (i - 5) * 2;
	return 28 + (i - 11) * 2;
}

// write ordinal th chunk (1 = last) of a long name into raw entry
static void lfn_encode_entry(nat8* e, nat32 ordinal, const nat16* chars, nat32 count, nat8 checksum) {
	memset(e, 0, 32);
	e[0] = (nat8)(ordinal | (ordinal == 1 ? 0x40 : 0));
	e[11] = FAT32_ATTR_LFN;
	e[12] = 0x00;	  // type
	e[13] = checksum; // short name checksum

	for (nat32 i = 0; i < 13; i++) { // pad with 0xFFFF
		nat32 o = lfn_char_offset(i);
		e[o] = 0xFF;
		e[o + 1] = 0xFF;
	}
	for (nat32 i = 0; i < count && i < 13; i++) {
		nat32 o = lfn_char_offset(i);
		e[o] = (nat8)(chars[i] & 0xFF);
		e[o + 1] = (nat8)((chars[i] >> 8) & 0xFF);
	}
	if (count < 13) {
		nat32 o = lfn_char_offset(count);
		e[o] = 0;
		e[o + 1] = 0;
	}
}

// extract up to 13 UTF16 units from an LFN entry. stops at 0x0000
static nat32 lfn_decode_entry(const nat8* e, nat16* out) {
	nat32 n = 0;
	for (nat32 i = 0; i < 13; i++) {
		nat32 o = lfn_char_offset(i);
		nat16 u = (nat16)(e[o] | (e[o + 1] << 8));
		if (u == 0)
			break;
		out[n++] = u;
	}
	return n;
}

// rebuild the long name from a physically last to ffirst LFN chunk set
static void lfn_assemble(nat8 pend[][32], const nat32* ord, nat32 n, char* out) {
	char tmp[256];
	nat8 filled[256];
	memset(tmp, 0, sizeof(tmp));
	memset(filled, 0, sizeof(filled));

	for (nat32 i = 0; i < n; i++) {
		nat16 u[13];
		nat32 cnt = lfn_decode_entry(pend[i], u);
		// ordinal counts from the END (1 = last chunk). so the chunk with ordinal n holds chars[0..12] and sits FIRST
		// on disk
		nat32 pos = (n - ord[i]) * 13;
		for (nat32 j = 0; j < cnt && pos + j < 255; j++) {
			tmp[pos + j] = (u[j] < 0x80) ? (char)u[j] : '?';
			filled[pos + j] = 1;
		}
	}

	nat32 k = 0;
	while (k < 255 && filled[k] && tmp[k])
		k++;
	memcpy(out, tmp, k);
	out[k] = 0;
}

// directory scan / enumeration
typedef struct {
	dir_t* d;
	nat32 cursor;
	nat8 pend[MAX_LFN_SETS][32];
	nat32 pend_ord[MAX_LFN_SETS];
	nat32 pend_n;
} dir_scan_t;

static void scan_init(dir_scan_t* s, dir_t* d) {
	s->d = d;
	s->cursor = 0;
	s->pend_n = 0;
}

// skip "." and ".." and other non-entries
static nat8 entry_visible(const fat32_dir_entry_t* e) {
	if (e->name[0] == 0x00 || e->name[0] == FAT32_DELETED)
		return 0;
	if (e->attr == FAT32_ATTR_LFN)
		return 0;
	if (e->attr & FAT32_ATTR_VOLUME_ID)
		return 0;
	if (e->attr & FAT32_ATTR_DIRECTORY) {
		if (e->name[0] == '.' && e->name[1] == ' ')
			return 0;
		if (e->name[0] == '.' && e->name[1] == '.' && e->name[2] == ' ')
			return 0;
	}
	return 1;
}

// next visible entry + its decoded (long or short) name
static nat8 scan_next(dir_scan_t* s, fat32_dir_entry_t** entry_out, char* name_out) {
	while (s->cursor < s->d->count) {
		fat32_dir_entry_t* e = &s->d->entries[s->cursor++];

		if (e->name[0] == 0x00) {
			s->pend_n = 0;
			return 0;
		} // end
		if (e->name[0] == FAT32_DELETED)
			continue;
		if (e->attr == FAT32_ATTR_LFN) {
			if (s->pend_n < MAX_LFN_SETS) {
				memcpy(s->pend[s->pend_n], e, 32);
				s->pend_ord[s->pend_n] = e->name[0] & 0x1F;
				s->pend_n++;
			}
			continue;
		}

		// regular entry: decode name
		nat8 used_lfn = 0;
		if (s->pend_n > 0) {
			// the last LFN chunk carries the short name checksum
			if (s->pend[s->pend_n - 1][13] == short_name_checksum(e->name))
				used_lfn = 1;
		}
		if (used_lfn)
			lfn_assemble(s->pend, s->pend_ord, s->pend_n, name_out);
		else
			short_decode(e->name, name_out);
		s->pend_n = 0;

		if (entry_visible(e)) {
			*entry_out = e;
			return 1;
		}
	}
	return 0;
}

// directory load / save

static nat8 dir_load(nat32 dir_cluster, dir_t* d) {
	memset(d, 0, sizeof(dir_t));

	nat8 buf[SECTOR_SIZE];
	nat32 c = dir_cluster;
	nat32 cluster_count = 0;

	while (c >= 2 && c < data_clusters + 2 && cluster_count < DIR_MAX_CLUSTERS) {
		if (!read_cluster(c, buf))
			return 0;
		d->clusters[cluster_count++] = c;
		nat32 next = fat_read_entry(c);
		if (next >= 0x0FFFFFF8)
			break;
		c = next;
	}

	if (cluster_count == 0)
		return 0;
	d->num_clusters = cluster_count;
	d->capacity = cluster_count * ENTRIES_PER_SECTOR;
	d->entries = calloc(d->capacity, sizeof(fat32_dir_entry_t));
	if (!d->entries)
		return 0;

	nat32 idx = 0;
	for (nat32 i = 0; i < cluster_count; i++) {
		if (!read_cluster(d->clusters[i], buf)) {
			free(d->entries);
			d->entries = NULL;
			return 0;
		}
		for (nat32 e = 0; e < ENTRIES_PER_SECTOR; e++, idx++)
			memcpy(&d->entries[idx], buf + e * 32, 32);
	}

	d->count = 0;
	while (d->count < d->capacity && d->entries[d->count].name[0] != 0x00)
		d->count++;
	return 1;
}

static void dir_save(const dir_t* d) {
	nat8 buf[SECTOR_SIZE];
	for (nat32 i = 0; i < d->num_clusters; i++) {
		memset(buf, 0, SECTOR_SIZE);
		for (nat32 e = 0; e < ENTRIES_PER_SECTOR; e++) {
			nat32 idx = i * ENTRIES_PER_SECTOR + e;
			if (idx < d->capacity)
				memcpy(buf + e * 32, &d->entries[idx], 32);
		}
		write_cluster(d->clusters[i], buf);
	}
}

// add one more zeroed cluster onto the directory chain
static nat8 dir_extend(dir_t* d) {
	if (d->num_clusters >= DIR_MAX_CLUSTERS)
		return 0;
	nat32 c = alloc_cluster();
	if (!c)
		return 0;

	nat8 z[SECTOR_SIZE];
	memset(z, 0, SECTOR_SIZE);
	write_cluster(c, z);

	nat32 prev = d->clusters[d->num_clusters - 1];
	fat_write_entry(prev, c);
	fat_write_entry(c, FAT32_EOC);

	fat32_dir_entry_t* ne = calloc(d->capacity + ENTRIES_PER_SECTOR, sizeof(fat32_dir_entry_t));
	if (!ne) {
		free_cluster_chain(c);
		return 0;
	}
	memcpy(ne, d->entries, d->capacity * sizeof(fat32_dir_entry_t));
	free(d->entries);
	d->entries = ne;

	d->clusters[d->num_clusters++] = c;
	d->capacity += ENTRIES_PER_SECTOR;
	return 1;
}

static nat32 find_free_run(const dir_t* d, nat32 need) {
	nat32 start = 0xFFFFFFFF, run = 0;
	for (nat32 i = 0; i < d->capacity; i++) {
		nat8 first = d->entries[i].name[0];
		if (first == 0x00 || first == FAT32_DELETED) {
			if (run == 0)
				start = i;
			if (++run >= need)
				return start;
		} else {
			run = 0;
		}
	}
	return 0xFFFFFFFF;
}

static nat8 dir_find(dir_t* d, const char* name, nat32* out_idx) {
	dir_scan_t s;
	scan_init(&s, d);
	fat32_dir_entry_t* e;
	char nm[256];
	while (scan_next(&s, &e, nm)) {
		if (strcasecmp_os(nm, name) == 0) {
			*out_idx = s.cursor - 1;
			return 1;
		}
	}
	return 0;
}

// insert a new entry (LFN set + short entry) into an open directory
static nat8 dir_insert(dir_t* d, const char* name, nat8 attr, nat32 cluster, nat32 size) {
	nat32 name_len = strlen(name);
	nat32 n_lfn = (name_len + 12) / 13;

	nat8 short11[11];
	build_short_name(d, name, short11);
	nat8 checksum = short_name_checksum(short11);

	// only write VFAT entries when the long name differs from the 8.3 short form (lowercase names, long names)
	char shortdisp[16];
	short_decode(short11, shortdisp);
	if (strcmp(shortdisp, name) == 0)
		n_lfn = 0;

	nat32 need = n_lfn + 1;
	nat32 run = find_free_run(d, need);
	if (run == 0xFFFFFFFF) {
		if (!dir_extend(d))
			return 0;
		run = find_free_run(d, need);
		if (run == 0xFFFFFFFF)
			return 0;
	}

	// LFN chunks, highest ordinal first
	for (nat32 k = 0; k < n_lfn; k++) {
		nat32 ordinal = n_lfn - k;
		nat32 cstart = k * 13;
		nat32 cend = cstart + 13;
		if (cend > name_len)
			cend = name_len;

		nat16 u[13];
		for (nat32 j = 0; j < 13; j++)
			u[j] = (j + cstart < cend) ? (nat16)(nat8)name[cstart + j] : 0;

		lfn_encode_entry((nat8*)&d->entries[run + k], ordinal, u, cend - cstart, checksum);
	}

	// short entry
	fat32_dir_entry_t* e = &d->entries[run + need - 1];
	memset(e, 0, sizeof(fat32_dir_entry_t));
	memcpy(e->name, short11, 11);
	e->attr = attr;
	entry_set_cluster(e, cluster);
	e->file_size = size;

	if (run + need > d->count)
		d->count = run + need;
	return 1;
}

// format / init

static nat32 disk_total_sectors(void) {
	if (ata_identify()) {
		nat32 sz = ata_get_drive_size();
		if (sz > 4096)
			return sz;
	}
	return 131072; // 64 MB default
}

static void compute_layout(nat32 total_sectors) {
	// literally the FAT32 spec calculation lol
	nat32 reserved = 32;
	nat32 fats = 2;
	nat32 fsz = 1;
	for (int i = 0; i < 16; i++) {
		nat32 clusters = total_sectors > reserved + fats * fsz ? total_sectors - reserved - fats * fsz : 0;
		nat32 nf = (clusters * 4 + SECTOR_SIZE - 1) / SECTOR_SIZE + 1;
		if (nf < 1)
			nf = 1;
		if (nf == fsz)
			break;
		fsz = nf;
	}
	spc = 1;
	num_fats = fats;
	fat_lba = reserved;
	fat_size = fsz;
	data_lba = reserved + fats * fsz;
	data_clusters = data_lba < total_sectors ? total_sectors - data_lba : 0;
}

static void build_boot_sector(nat8* buf, nat32 total_sectors) {
	memset(buf, 0, SECTOR_SIZE);
	fat32_bpb_t* b = (fat32_bpb_t*)buf;

	b->jump[0] = 0xEB;
	b->jump[1] = 0x58;
	b->jump[2] = 0x90;
	memcpy(b->oem, "MSWIN4.1", 8);
	b->bytes_per_sector = 512;
	b->sectors_per_cluster = (nat8)spc;
	b->reserved_sectors = (nat16)fat_lba;
	b->num_fats = (nat8)num_fats;
	b->root_entry_count = 0;
	b->total_sectors16 = 0;
	b->media_descriptor = 0xF8;
	b->fat_size16 = 0;
	b->sectors_per_track = 63;
	b->num_heads = 255;
	b->hidden_sectors = 0;
	b->total_sectors32 = total_sectors;
	b->fat_size32 = fat_size;
	b->ext_flags = 0;
	b->fs_version = 0;
	b->root_cluster = FAT32_ROOT_CLUSTER;
	b->fsinfo_sector = 1;
	b->backup_boot_sector = 6;
	b->drive_number = 0x80;
	b->boot_signature = 0x29;
	b->volume_id = 0x53484759; // "SHGY"
	memcpy(b->volume_label, "SHIGGY OS  ", 11);
	memcpy(b->fat_type, "FAT32   ", 8);

	// default boot code + message, never actually booted from
	const char* msg = "This is not a bootable disk.";
	nat32 n = strlen(msg);
	if (n > 420)
		n = 420;
	memcpy(b->boot_code, msg, n);

	buf[510] = 0x55;
	buf[511] = 0xAA;
}

static void build_fsinfo(nat8* buf) {
	memset(buf, 0, SECTOR_SIZE);
	memcpy(buf + 0, "\x52\x52\x61\x41", 4);	  // "RRaA" lead signature
	memcpy(buf + 484, "\x72\x72\x41\x61", 4); // "rrAa" struct signature
	nat32 free = data_clusters - 1;			  // cluster 2 (root) used
	memcpy(buf + 488, &free, 4);
	nat32 hint = 3;
	memcpy(buf + 492, &hint, 4);
	memcpy(buf + 508, "\x00\x00\x55\xAA", 4); // trailing signature
}

static void format_fs(void) {
	nat32 total = disk_total_sectors();
	compute_layout(total);

	// boot sectors + fsinfo
	nat8 boot[SECTOR_SIZE];
	build_boot_sector(boot, total);
	write_sector_lba(0, boot);
	write_sector_lba(6, boot);

	nat8 fi[SECTOR_SIZE];
	build_fsinfo(fi);
	write_sector_lba(1, fi);
	write_sector_lba(7, fi);

	// zero everything then fix up the reserved FAT entries
	nat8 z[SECTOR_SIZE];
	memset(z, 0, SECTOR_SIZE);
	for (nat32 copy = 0; copy < num_fats; copy++)
		for (nat32 s = 0; s < fat_size; s++)
			write_sector_lba(fat_lba + copy * fat_size + s, z);
	fat_write_entry(0, 0x0FFFFFF8);
	fat_write_entry(1, 0xFFFFFFFF);
	fat_write_entry(FAT32_ROOT_CLUSTER, FAT32_EOC);

	// root directory: .  ..  volume label
	nat8 rbuf[SECTOR_SIZE];
	memset(rbuf, 0, SECTOR_SIZE);
	{
		fat32_dir_entry_t* dot = (fat32_dir_entry_t*)rbuf;
		memset(dot->name, ' ', 8);
		memset(dot->ext, ' ', 3);
		dot->name[0] = '.';
		dot->attr = FAT32_ATTR_DIRECTORY;
		entry_set_cluster(dot, FAT32_ROOT_CLUSTER);

		fat32_dir_entry_t* dotdot = (fat32_dir_entry_t*)(rbuf + 32);
		memset(dotdot->name, ' ', 8);
		memset(dotdot->ext, ' ', 3);
		dotdot->name[0] = '.';
		dotdot->name[1] = '.';
		dotdot->attr = FAT32_ATTR_DIRECTORY;
		entry_set_cluster(dotdot, FAT32_ROOT_CLUSTER);

		fat32_dir_entry_t* vol = (fat32_dir_entry_t*)(rbuf + 64);
		memset(vol->name, ' ', 8);
		memset(vol->ext, ' ', 3);
		memcpy(vol->name, "SHIGGY OS", 9);
		vol->attr = FAT32_ATTR_VOLUME_ID;
	}
	write_cluster(FAT32_ROOT_CLUSTER, rbuf);

	next_free_hint = 3;
	fs_ready = 1;
}

static void init_from_disk(void) {
	nat8 buf[SECTOR_SIZE];
	if (!read_sector_lba(0, buf))
		return;
	fat32_bpb_t* b = (fat32_bpb_t*)buf;

	if (b->bytes_per_sector != SECTOR_SIZE)
		return; // no stop it cant do that
	spc = b->sectors_per_cluster ? b->sectors_per_cluster : 1;
	num_fats = b->num_fats ? b->num_fats : 2;
	fat_lba = b->reserved_sectors;
	fat_size = b->fat_size32 ? b->fat_size32 : 1024;
	data_lba = fat_lba + num_fats * fat_size;
	nat32 total = b->total_sectors32 ? b->total_sectors32 : (nat32)b->total_sectors16;
	data_clusters = total > data_lba ? total - data_lba : 0;
	next_free_hint = 3;
	fs_ready = 1;
}

// public API yay

nat8 is_formatted(void) {
	nat8 buf[SECTOR_SIZE];
	if (!read_sector_lba(0, buf))
		return 0;
	if (buf[510] != 0x55 || buf[511] != 0xAA)
		return 0;

	fat32_bpb_t* b = (fat32_bpb_t*)buf;
	if (b->bytes_per_sector != SECTOR_SIZE)
		return 0;
	if (b->sectors_per_cluster == 0)
		return 0;
	if (b->root_entry_count != 0)
		return 0; // FAT16/12 have these
	if (b->fat_size16 != 0 || b->fat_size32 == 0)
		return 0;
	return memcmp(b->fat_type, "FAT32   ", 8) == 0;
}

nat8 is_hdd_present(void) {
	nat8 buffer[SECTOR_SIZE];
	return ata_read_sector(0, buffer);
}

void fat32_fs_init(void) {
	if (!is_hdd_present()) {
		print("No HDD detected!\n");
		return;
	}

	if (is_formatted())
		init_from_disk();
	else
		format_fs();

	if (!fs_ready) {
		print("FAT32: unsupported disk, reformatting\n");
		format_fs();
	}

	//printf("FAT32: %d KB, %d clusters, %d sectors/cluster\n", (int)((data_clusters * spc * SECTOR_SIZE) / 1024), (int)data_clusters, (int)spc);
}

nat8 fat32_create_file(nat32 dir_cluster, const char* name, const char* content) {
	if (!fs_ready)
		return 0;

	nat32 len = strlen(content);
	nat32 cluster_bytes = spc * SECTOR_SIZE;
	nat32 needed = (len + cluster_bytes - 1) / cluster_bytes;

	dir_t d;
	if (!dir_load(dir_cluster, &d))
		return 0;

	nat32 existing;
	if (dir_find(&d, name, &existing)) {
		free(d.entries);
		return 0;
	}

	nat32 first = 0;
	if (needed > 0) {
		if (needed > 512) {
			free(d.entries);
			return 0;
		}
		nat32 chain[512];
		nat32 n = 0;
		first = alloc_cluster();
		if (!first) {
			free(d.entries);
			return 0;
		}
		chain[n++] = first;
		while (n < needed) {
			nat32 c = alloc_cluster();
			if (!c) {
				free_cluster_chain(first);
				free(d.entries);
				return 0;
			}
			chain[n++] = c;
		}
		for (nat32 k = 0; k + 1 < n; k++)
			fat_write_entry(chain[k], chain[k + 1]);

		// write the content across the cluster chain
		nat32 written = 0;
		for (nat32 k = 0; k < n; k++) {
			nat32 lba = cluster_to_lba(chain[k]);
			for (nat32 s = 0; s < spc; s++) {
				nat8 buf[SECTOR_SIZE];
				memset(buf, 0, SECTOR_SIZE);
				if (written < len) {
					nat32 chunk = len - written;
					if (chunk > SECTOR_SIZE)
						chunk = SECTOR_SIZE;
					memcpy(buf, content + written, chunk);
					written += chunk;
				}
				write_sector_lba(lba + s, buf);
			}
		}
	}

	if (!dir_insert(&d, name, FAT32_ATTR_ARCHIVE, first, len)) {
		if (first)
			free_cluster_chain(first);
		free(d.entries);
		return 0;
	}
	dir_save(&d);
	free(d.entries);
	return 1;
}

nat8 fat32_create_dir(nat32 dir_cluster, const char* name) {
	if (!fs_ready)
		return 0;

	dir_t d;
	if (!dir_load(dir_cluster, &d))
		return 0;

	nat32 existing;
	if (dir_find(&d, name, &existing)) {
		free(d.entries);
		return 0;
	}

	nat32 nc = alloc_cluster();
	if (!nc) {
		free(d.entries);
		return 0;
	}

	// initialize the new directory with . and ..
	nat8 buf[SECTOR_SIZE];
	memset(buf, 0, SECTOR_SIZE);
	fat32_dir_entry_t* dot = (fat32_dir_entry_t*)buf;
	memset(dot->name, ' ', 8);
	memset(dot->ext, ' ', 3);
	dot->name[0] = '.';
	dot->attr = FAT32_ATTR_DIRECTORY;
	entry_set_cluster(dot, nc);

	fat32_dir_entry_t* dotdot = (fat32_dir_entry_t*)(buf + 32);
	memset(dotdot->name, ' ', 8);
	memset(dotdot->ext, ' ', 3);
	dotdot->name[0] = '.';
	dotdot->name[1] = '.';
	dotdot->attr = FAT32_ATTR_DIRECTORY;
	entry_set_cluster(dotdot, dir_cluster);
	write_cluster(nc, buf);

	if (!dir_insert(&d, name, FAT32_ATTR_DIRECTORY, nc, 0)) {
		free_cluster_chain(nc);
		free(d.entries);
		return 0;
	}
	dir_save(&d);
	free(d.entries);
	return 1;
}

nat8 fat32_read_file(nat32 dir_cluster, const char* name, char* buffer, nat32 max_size) {
	if (!fs_ready)
		return 0;

	dir_t d;
	if (!dir_load(dir_cluster, &d))
		return 0;

	nat32 idx;
	if (!dir_find(&d, name, &idx)) {
		free(d.entries);
		return 0;
	}
	fat32_dir_entry_t* e = &d.entries[idx];
	if (e->attr & FAT32_ATTR_DIRECTORY) {
		free(d.entries);
		return 0;
	}

	nat32 sz = e->file_size;
	nat32 out_sz = max_size > 0 ? max_size - 1 : 0;
	if (sz > out_sz)
		sz = out_sz;

	nat32 c = entry_cluster(e);
	nat32 read = 0;
	while (read < sz && c >= 2) {
		nat8 cb[SECTOR_SIZE];
		for (nat32 s = 0; s < spc && read < sz; s++) {
			read_sector_lba(cluster_to_lba(c) + s, cb);
			nat32 chunk = sz - read;
			if (chunk > SECTOR_SIZE)
				chunk = SECTOR_SIZE;
			memcpy(buffer + read, cb, chunk);
			read += chunk;
		}
		c = fat_read_entry(c);
		if (c >= 0x0FFFFFF8)
			break;
	}
	buffer[read] = 0;
	free(d.entries);
	return 1;
}

nat32 fat32_file_size(nat32 dir_cluster, const char* name) {
	if (!fs_ready)
		return 0;

	dir_t d;
	if (!dir_load(dir_cluster, &d))
		return 0;

	nat32 idx;
	if (!dir_find(&d, name, &idx)) {
		free(d.entries);
		return 0;
	}
	nat32 sz = d.entries[idx].file_size;
	free(d.entries);
	return sz;
}

nat8 fat32_delete_file(nat32 dir_cluster, const char* name) {
	if (!fs_ready)
		return 0;

	dir_t d;
	if (!dir_load(dir_cluster, &d))
		return 0;

	nat32 idx;
	if (!dir_find(&d, name, &idx)) {
		free(d.entries);
		return 0;
	}
	fat32_dir_entry_t* e = &d.entries[idx];
	if (e->attr & FAT32_ATTR_DIRECTORY) {
		free(d.entries);
		return 0;
	}

	free_cluster_chain(entry_cluster(e));

	// mark the short entry deleted an then all of its LFN chunks
	e->name[0] = FAT32_DELETED;
	nat32 i = idx;
	while (i > 0 && d.entries[i - 1].attr == FAT32_ATTR_LFN) {
		d.entries[--i].name[0] = FAT32_DELETED;
	}

	dir_save(&d);
	free(d.entries);
	return 1;
}

nat32 fat32_file_count(nat32 dir_cluster) {
	if (!fs_ready)
		return 0;

	dir_t d;
	if (!dir_load(dir_cluster, &d))
		return 0;

	dir_scan_t s;
	scan_init(&s, &d);
	fat32_dir_entry_t* e;
	char nm[256];
	nat32 n = 0;
	while (scan_next(&s, &e, nm))
		if (!(e->attr & FAT32_ATTR_DIRECTORY))
			n++;

	free(d.entries);
	return n;
}

nat32 fat32_dir_count(nat32 dir_cluster) {
	if (!fs_ready)
		return 0;

	dir_t d;
	if (!dir_load(dir_cluster, &d))
		return 0;

	dir_scan_t s;
	scan_init(&s, &d);
	fat32_dir_entry_t* e;
	char nm[256];
	nat32 n = 0;
	while (scan_next(&s, &e, nm))
		if (e->attr & FAT32_ATTR_DIRECTORY)
			n++;

	free(d.entries);
	return n;
}

nat8 fat32_dir_get_entry(nat32 dir_cluster, nat32 index, fat32_entry_info_t* out) {
	if (!fs_ready)
		return 0;

	dir_t d;
	if (!dir_load(dir_cluster, &d))
		return 0;

	dir_scan_t s;
	scan_init(&s, &d);
	fat32_dir_entry_t* e;
	char nm[256];
	nat32 vis = 0;
	while (scan_next(&s, &e, nm)) {
		if (vis == index) {
			memset(out, 0, sizeof(fat32_entry_info_t));
			strncpy(out->name, nm, 255);
			out->name[255] = 0;
			out->attr = e->attr;
			out->first_cluster = entry_cluster(e);
			out->file_size = e->file_size;
			free(d.entries);
			return 1;
		}
		vis++;
	}

	free(d.entries);
	return 0;
}

nat8 fat32_dir_parent(nat32 dir_cluster, nat32* parent_out) {
	if (!fs_ready)
		return 0;

	dir_t d;
	if (!dir_load(dir_cluster, &d))
		return 0;

	nat8 found = 0;
	for (nat32 i = 0; i < d.count; i++) {
		fat32_dir_entry_t* e = &d.entries[i];
		if (e->name[0] == 0x00)
			break;
		if (e->attr == FAT32_ATTR_LFN)
			continue;
		if (e->name[0] == '.' && e->name[1] == '.') {
			*parent_out = entry_cluster(e);
			found = 1;
			break;
		}
	}

	free(d.entries);
	return found;
}