#include "network/main.h"
#include "globals.h"

// to anyone reading this: IM sorry BUT IT FUCKING WORKS!!
// todo: make cleaner

static nat32 iobase;
static nat8 RTL8139SLOT;
static nat8 RTL8139BUS;
static nat16 rx_read_ptr; // software RX ring read pointer
static nat8 tx_slot; // 4 TX descriptors to use next

rtl8139 rtl8139Device;

static const byte our_ip[4] = NET_OUR_IP;
static const byte gateway[4] = NET_GATEWAY;

static byte gw_mac[6];
static int gw_mac_valid = 0;

static inline nat16 htons(nat16 x) {
	return (nat16)((x >> 8) | (x << 8));
}
static inline nat32 htonl(nat32 x) {
	return ((x >> 24) & 0xFF)
	     | ((x >> 8) & 0x0000FF00)
	     | ((x << 8) & 0x00FF0000)
	     | ((x << 24) & 0xFF000000);
}

static nat16 ip_checksum(void* data, nat32 len) {
	nat16* ptr = (nat16*)data;
	nat32 sum = 0;
	while (len > 1) {
		sum += *ptr++;
		len -= 2;
	}
	if (len) sum += *(nat8*)ptr;
	while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
	return (nat16)(~sum);
}

static nat32 rtl8139_find() {
	nat32 n = 0;
	for (nat16 bus = 0; bus < 256; bus++) {
		for (nat8 slot = 0; slot < 32; slot++) {
			if (++n > RTL8139_DETECT_TIMEOUT) return 0;
			nat32 vd = pci_config_read_word(bus, slot, 0, 0x00);
			if (vd == (nat32)((RTL8139_DEVICE_ID << 16) | RTL8139_VENDOR_ID)) {
				nat32 bar = pci_config_read_word(bus, slot, 0, 0x10);
				if (bar & 0x01) {
					RTL8139BUS = (nat8)bus;
					RTL8139SLOT = slot;
					// i/o space, memory space, bus mastering
					nat32 cmd = pci_config_read_word(bus, slot, 0, 0x04);
					pci_config_write_word(bus, slot, 0, 0x04, cmd | 0x07);
					return bar & ~0x3U;
				}
			}
		}
	}
	return 0;
}

byte* get_mac() {
	for (int i = 0; i < 6; i++) rtl8139Device.mac[i] = in_byte((nat16)(iobase + i));
	return rtl8139Device.mac;
}

static void init_rtl8139() {
	out_byte((nat16)(iobase + 0x52), 0x00);

	out_byte((nat16)(iobase + RTL8139_REG_COMMAND), RTL8139_CMD_RESET);
	while (in_byte((nat16)(iobase + RTL8139_REG_COMMAND)) & RTL8139_CMD_RESET) {}

	get_mac();

	// split across the ring boundary (WRAP=0 in RCR)
	rtl8139Device.rxBuffer = (nat32)malloc(8192 + 16 + 1500);
	if (!rtl8139Device.rxBuffer) {
		print("rtl8139: failed to alloc rx buffer\n");
		return;
	}
	rx_read_ptr = 0;
	out_long((nat16)(iobase + RTL8139_REG_RBSTART), rtl8139Device.rxBuffer);

	for (int i = 0; i < 4; i++) {
		rtl8139Device.txBuffer[i] = (nat32)malloc(1536);
		if (!rtl8139Device.txBuffer[i]) {
			print("rtl8139: failed to alloc tx buffer\n");
			return;
		}
	}
	tx_slot = 0;

	out_b16((nat16)(iobase + RTL8139_REG_ISR), 0xFFFF);

	out_b16((nat16)(iobase + RTL8139_REG_IMR), 0x0000);

	out_long((nat16)(iobase + RTL8139_REG_RCR), 0x0000000F);

	out_b16((nat16)(iobase + RTL8139_REG_CAPR), 0xFFF0);

	out_byte((nat16)(iobase + RTL8139_REG_COMMAND), RTL8139_CMD_RX_EN | RTL8139_CMD_TX_EN);
}

void send_frame(byte* data, int len) {
	if (len > 1500) return;

	nat8 slot = tx_slot & 3;
	tx_slot++;

	memcpy((void*)rtl8139Device.txBuffer[slot], data, (nat32)len);

	out_long((nat16)(iobase + 0x20 + slot * 4), rtl8139Device.txBuffer[slot]);

	out_long((nat16)(iobase + 0x10 + slot * 4), (nat32)len & 0x1FFF);

	nat32 spin = 0;
	while (!(in_long(iobase + 0x10 + slot * 4) & 0x8000) && ++spin < 100000) {}
}

static byte* rtl8139_poll_rx(nat32* len_out) {
	if (in_byte((nat16)(iobase + RTL8139_REG_COMMAND)) & 0x01) return NULL;

	byte* buf = (byte*)rtl8139Device.rxBuffer;
	nat16 offset = rx_read_ptr % 8192;

	nat16 pkt_status = *(nat16*)(buf + offset);
	nat16 pkt_len = *(nat16*)(buf + offset + 2);

	if (!(pkt_status & 0x01)) return NULL;
	if (pkt_len < 4 || pkt_len > 1522) return NULL; // sanity

	nat32 data_len = (nat32)(pkt_len - 4);
	byte* pkt_data = buf + offset + 4;

	rx_read_ptr = (nat16)(((nat32)(offset + pkt_len + 4 + 3) & ~3U) % 8192);

	// Hello Nic
	out_b16((nat16)(iobase + RTL8139_REG_CAPR), (nat16)(rx_read_ptr - 16));

	*len_out = data_len;
	return pkt_data;
}

static void eth_send(byte dst_mac[6], nat16 ethertype, byte* payload, int payload_len) {
	static byte frame[1518];
	eth_header_t* eth = (eth_header_t*)frame;

	memcpy(eth->dst, dst_mac, 6);
	memcpy(eth->src, rtl8139Device.mac, 6);
	eth->ethertype = htons(ethertype);

	memcpy(frame + sizeof(eth_header_t), payload, (nat32)payload_len);
	send_frame(frame, (int)sizeof(eth_header_t) + payload_len);
}

static void arp_request(const byte target_ip[4]) {
	arp_packet_t arp;
	arp.htype = htons(ARP_HTYPE_ETHERNET);
	arp.ptype = htons(ARP_PTYPE_IPV4);
	arp.hlen = 6;
	arp.plen = 4;
	arp.oper = htons(ARP_OP_REQUEST);
	memcpy(arp.sha, rtl8139Device.mac, 6);
	memcpy(arp.spa, our_ip, 4);
	memset(arp.tha, 0x00, 6);
	memcpy(arp.tpa, target_ip, 4);

	byte bcast[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	eth_send(bcast, ETHERTYPE_ARP, (byte*)&arp, (int)sizeof(arp_packet_t));
}

static int arp_handle_reply(byte* pkt, nat32 len, const byte wanted_ip[4], byte out_mac[6]) {
	if (len < sizeof(eth_header_t) + sizeof(arp_packet_t)) return 0;
	eth_header_t* eth = (eth_header_t*)pkt;
	if (htons(eth->ethertype) != ETHERTYPE_ARP) return 0;

	arp_packet_t* arp = (arp_packet_t*)(pkt + sizeof(eth_header_t));
	if (htons(arp->oper) != ARP_OP_REPLY) return 0;
	if (memcmp(arp->spa, wanted_ip, 4) != 0) return 0;

	memcpy(out_mac, arp->sha, 6);
	return 1;
}

static int arp_resolve(const byte target_ip[4], byte out_mac[6], nat32 timeout_ms) {
	nat32 start = timer_ticks;
	nat32 ticks = timeout_ms / 20;
	if (ticks == 0) ticks = 1;

	arp_request(target_ip);

	while (timer_ticks - start < ticks) {
		nat32 len;
		byte* pkt = rtl8139_poll_rx(&len);
		if (!pkt) continue;
		if (arp_handle_reply(pkt, len, target_ip, out_mac)) return 1;
	}
	return 0;
}

static nat16 icmp_id_counter = 0x1234;

static void ip_send(byte dst_ip[4], byte proto, byte* payload, nat16 payload_len, byte dst_mac[6]) {
	static byte pkt[1500];
	ip_header_t* iph = (ip_header_t*)pkt;

	iph->version_ihl = 0x45;
	iph->dscp_ecn = 0;
	iph->total_length = htons((nat16)(sizeof(ip_header_t) + payload_len));
	iph->id = htons(icmp_id_counter++);
	iph->flags_frag = 0;
	iph->ttl = 64;
	iph->protocol = proto;
	iph->checksum = 0;
	memcpy(iph->src, our_ip, 4);
	memcpy(iph->dst, dst_ip, 4);
	iph->checksum = ip_checksum(iph, sizeof(ip_header_t));

	memcpy(pkt + sizeof(ip_header_t), payload, payload_len);
	eth_send(dst_mac, ETHERTYPE_IP, pkt, (int)(sizeof(ip_header_t) + payload_len));
}

int net_ping(byte dst_ip[4], nat32 timeout_ms, nat32* rtt_ms) {
	byte nexthop_ip[4];
	int same_subnet = 1;
	const byte nm[4] = NET_NETMASK;
	for (int i = 0; i < 4; i++) {
		if ((dst_ip[i] & nm[i]) != (our_ip[i] & nm[i])) {
			same_subnet = 0;
			break;
		}
	}

	if (same_subnet) {
		memcpy(nexthop_ip, dst_ip, 4);
	} else {
		memcpy(nexthop_ip, gateway, 4);
	}

	byte nexthop_mac[6];
	if (!same_subnet && gw_mac_valid) {
		memcpy(nexthop_mac, gw_mac, 6);
	} else {
		printf("ARP: resolving %d.%d.%d.%d ...\n", nexthop_ip[0], nexthop_ip[1], nexthop_ip[2], nexthop_ip[3]);
		if (!arp_resolve(nexthop_ip, nexthop_mac, 3000)) {
			print("ARP timed out\n");
			return 0;
		}
		if (!same_subnet) {
			memcpy(gw_mac, nexthop_mac, 6);
			gw_mac_valid = 1;
		}
		printf("ARP reply: %p:%p:%p:%p:%p:%p\n", nexthop_mac[0], nexthop_mac[1], nexthop_mac[2], nexthop_mac[3], nexthop_mac[4], nexthop_mac[5]);
	}

	static byte icmp_buf[64];
	icmp_header_t* icmp = (icmp_header_t*)icmp_buf;
	icmp->type = ICMP_TYPE_ECHO_REQUEST;
	icmp->code = 0;
	icmp->checksum = 0;
	icmp->id = htons(0xBEEF);
	icmp->seq = htons(1);

	nat32 data_len = 32;
	for (nat32 i = 0; i < data_len; i++)
		icmp_buf[sizeof(icmp_header_t) + i] = (byte)('0' + i % 10);

	nat16 icmp_len = (nat16)(sizeof(icmp_header_t) + data_len);
	icmp->checksum = ip_checksum(icmp_buf, icmp_len);

	nat32 t0 = timer_ticks;
	ip_send(dst_ip, IP_PROTO_ICMP, icmp_buf, icmp_len, nexthop_mac);

	nat32 ticks = timeout_ms / 20;
	if (ticks == 0) ticks = 1;

	while (timer_ticks - t0 < ticks) {
		nat32 len;
		byte* pkt = rtl8139_poll_rx(&len);
		if (!pkt) continue;

		if (len < sizeof(eth_header_t) + sizeof(ip_header_t) + sizeof(icmp_header_t))
			continue;

		eth_header_t* eth = (eth_header_t*)pkt;
		if (htons(eth->ethertype) != ETHERTYPE_IP) continue;

		ip_header_t* iph = (ip_header_t*)(pkt + sizeof(eth_header_t));
		if (iph->protocol != IP_PROTO_ICMP) continue;
		if (memcmp(iph->src, dst_ip, 4) != 0) continue;

		nat32 ip_hdr_len = (iph->version_ihl & 0x0F) * 4;
		icmp_header_t* reply = (icmp_header_t*)((byte*)iph + ip_hdr_len);
		if (reply->type != ICMP_TYPE_ECHO_REPLY) continue;
		if (htons(reply->id) != 0xBEEF) continue;

		nat32 t1 = timer_ticks;
		if (rtt_ms) *rtt_ms = (t1 - t0) * 20;
		return 1;
	}
	return 0;
}

static int get_nexthop_mac(byte dst_ip[4], byte out_mac[6]) {
	(void)dst_ip;
	if (gw_mac_valid) { memcpy(out_mac, gw_mac, 6); return 1; }
	if (!arp_resolve(gateway, out_mac, 3000)) return 0;
	memcpy(gw_mac, out_mac, 6);
	gw_mac_valid = 1;
	return 1;
}

static nat16 transport_checksum(byte src[4], byte dst[4], byte proto, byte* seg, nat16 seg_len) {
	byte ph[12];
	memcpy(ph, src, 4);
	memcpy(ph + 4, dst, 4);
	ph[8] = 0;
	ph[9] = proto;
	ph[10] = (seg_len >> 8) & 0xFF;
	ph[11] = seg_len & 0xFF;

	nat32 sum = 0;
	nat16* p;

	p = (nat16*)ph;
	for (int i = 0; i < 6; i++) sum += p[i];

	p = (nat16*)seg;
	nat16 n = seg_len;
	while (n > 1) { sum += *p++; n = (nat16)(n - 2); }
	if (n) sum += *(nat8*)p;

	while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
	return (nat16)(~sum);
}

// UDP!!
static void udp_send(byte dst_ip[4], byte dst_mac[6], nat16 src_port, nat16 dst_port, byte* data, nat16 data_len) {
	static byte seg[1472];
	udp_header_t* udp = (udp_header_t*)seg;
	nat16 udp_len = (nat16)(sizeof(udp_header_t) + data_len);

	udp->src_port = htons(src_port);
	udp->dst_port = htons(dst_port);
	udp->length = htons(udp_len);
	udp->checksum = 0;
	memcpy(seg + sizeof(udp_header_t), data, data_len);
	udp->checksum = transport_checksum((byte*)our_ip, dst_ip, IP_PROTO_UDP, seg, udp_len);

	ip_send(dst_ip, IP_PROTO_UDP, seg, udp_len, dst_mac);
}

static int dns_encode_name(const char* name, byte* out) {
	int pos = 0;
	while (*name) {
		const char* dot = name;
		while (*dot && *dot != '.') dot++;
		int len = (int)(dot - name);
		out[pos++] = (byte)len;
		for (int i = 0; i < len; i++) out[pos++] = (byte)name[i];
		if (*dot == '.') name = dot + 1; else break;
	}
	out[pos++] = 0;
	return pos;
}

int net_dns_resolve(const char* hostname, byte out_ip[4]) {
	static byte dns_ip[4] = {10, 0, 2, 3};
	byte dns_mac[6];
	if (!get_nexthop_mac(dns_ip, dns_mac)) return 0;

	static byte qbuf[512];
	int pos = 0;

	// header: id=0xAABB, RD=1, 1 question
	qbuf[pos++] = 0xAA; qbuf[pos++] = 0xBB;
	qbuf[pos++] = 0x01; qbuf[pos++] = 0x00;
	qbuf[pos++] = 0x00; qbuf[pos++] = 0x01;
	qbuf[pos++] = 0x00; qbuf[pos++] = 0x00;
	qbuf[pos++] = 0x00; qbuf[pos++] = 0x00;
	qbuf[pos++] = 0x00; qbuf[pos++] = 0x00;

	pos += dns_encode_name(hostname, qbuf + pos);
	qbuf[pos++] = 0x00; qbuf[pos++] = 0x01; // QTYPE A
	qbuf[pos++] = 0x00; qbuf[pos++] = 0x01; // QCLASS IN

	udp_send(dns_ip, dns_mac, 54321, 53, qbuf, (nat16)pos);
	printf("DNS query sent (%d bytes)\n", pos);

	nat32 start = timer_ticks, ticks = 5000 / 20;
	while (timer_ticks - start < ticks) {
		nat32 len;
		byte* pkt = rtl8139_poll_rx(&len);
		if (!pkt) continue;

		if (len < sizeof(eth_header_t) + sizeof(ip_header_t)) continue;

		eth_header_t* eth = (eth_header_t*)pkt;
		nat16 etype = htons(eth->ethertype);
		if (etype != ETHERTYPE_IP) continue;

		ip_header_t* iph = (ip_header_t*)(pkt + sizeof(eth_header_t));
		printf("DNS poll: proto=%d src=%d.%d.%d.%d len=%d\n", iph->protocol, iph->src[0], iph->src[1], iph->src[2], iph->src[3], len);

		if (iph->protocol != IP_PROTO_UDP) continue;
		if (len < sizeof(eth_header_t) + sizeof(ip_header_t) + sizeof(udp_header_t) + 12)
			continue;

		nat32 ihl = (iph->version_ihl & 0x0F) * 4;
		udp_header_t* udp = (udp_header_t*)((byte*)iph + ihl);
		printf("  UDP dst_port=%d\n", htons(udp->dst_port));
		if (htons(udp->dst_port) != 54321) continue;

		byte* resp = (byte*)udp + sizeof(udp_header_t);
		printf("  DNS id=%p%p ancount=%d\n", resp[0], resp[1], (resp[6]<<8)|resp[7]);
		if (resp[0] != 0xAA || resp[1] != 0xBB) continue;

		nat16 ancount = (nat16)((resp[6] << 8) | resp[7]);
		if (!ancount) return 0;

		// header(12) + qname + qtype(2) + qclass(2)
		byte* p = resp + 12;
		while (*p) p += *p + 1;
		p += 5; // null label + qtype + qclass

		for (nat16 i = 0; i < ancount; i++) {
			if ((*p & 0xC0) == 0xC0) p += 2; // pointer compression
			else { while (*p) p += *p + 1; p++; }
			nat16 rtype = (nat16)(((nat16)p[0] << 8) | p[1]);
			nat16 rdlen = (nat16)(((nat16)p[8] << 8) | p[9]);
			p += 10;
			if (rtype == 1 && rdlen == 4) { // A record
				memcpy(out_ip, p, 4);
				return 1;
			}
			p += rdlen;
		}
	}
	return 0;
}

static void tcp_emit(tcp_conn_t* c, nat8 flags, byte* data, nat16 data_len) {
	static byte seg[1480];
	tcp_header_t* tcp = (tcp_header_t*)seg;
	nat16 hdr_len = (nat16)sizeof(tcp_header_t);

	tcp->src_port = htons(c->src_port);
	tcp->dst_port = htons(c->dst_port);
	tcp->seq = htonl(c->seq);
	tcp->ack = htonl(c->ack);
	tcp->offset = (nat8)((hdr_len / 4) << 4);
	tcp->flags = flags;
	tcp->window = htons(4096);
	tcp->checksum = 0;
	tcp->urgent = 0;

	if (data_len) memcpy(seg + hdr_len, data, data_len);
	nat16 seg_len = (nat16)(hdr_len + data_len);
	tcp->checksum = transport_checksum((byte*)our_ip, c->dst_ip, IP_PROTO_TCP, seg, seg_len);

	ip_send(c->dst_ip, IP_PROTO_TCP, seg, seg_len, c->dst_mac);
}

// gibt 1 zruck wenn des pakerl a TCP segment fia unsane vabindung is füllt flags/seq/ack/data
static int tcp_match(tcp_conn_t* c, byte* pkt, nat32 pkt_len, nat8* out_flags, nat32* out_seq, nat32* out_ack, byte** out_data, nat16* out_data_len) {
	if (pkt_len < sizeof(eth_header_t) + sizeof(ip_header_t) + sizeof(tcp_header_t))
		return 0;
	eth_header_t* eth = (eth_header_t*)pkt;
	if (htons(eth->ethertype) != ETHERTYPE_IP) return 0;

	ip_header_t* iph = (ip_header_t*)(pkt + sizeof(eth_header_t));
	if (iph->protocol != IP_PROTO_TCP) return 0;
	if (memcmp(iph->src, c->dst_ip, 4) != 0) return 0;

	nat32 ihl = (iph->version_ihl & 0x0F) * 4;
	tcp_header_t* tcp = (tcp_header_t*)((byte*)iph + ihl);
	if (htons(tcp->src_port) != c->dst_port) return 0;
	if (htons(tcp->dst_port) != c->src_port) return 0;

	*out_flags = tcp->flags;
	*out_seq = htonl(tcp->seq);
	*out_ack = htonl(tcp->ack);

	nat32 tcp_hdr_len = (nat32)((tcp->offset >> 4) * 4);
	nat32 ip_total = htons(iph->total_length);
	nat32 tcp_data_len = ip_total > ihl + tcp_hdr_len ? ip_total - ihl - tcp_hdr_len : 0;
	*out_data = (byte*)tcp + tcp_hdr_len;
	*out_data_len = (nat16)tcp_data_len;
	return 1;
}

static int tcp_connect(tcp_conn_t* c, byte dst_ip[4], nat16 dst_port, byte dst_mac[6]) {
	memcpy(c->dst_ip, dst_ip, 4);
	memcpy(c->dst_mac, dst_mac, 6);
	c->src_port = 49152;
	c->dst_port = dst_port;
	c->seq = 0xCAFEBABE;
	c->ack = 0;
	c->established = 0;
	c->peer_fin = 0;

	tcp_emit(c, TCP_SYN, NULL, 0);
	c->seq++;

	nat32 start = timer_ticks, ticks = 5000 / 20;
	while (timer_ticks - start < ticks) {
		nat32 len; byte* pkt = rtl8139_poll_rx(&len);
		if (!pkt) continue;
		nat8 flags; nat32 rseq, rack; byte* d; nat16 dl;
		if (!tcp_match(c, pkt, len, &flags, &rseq, &rack, &d, &dl)) continue;
		if ((flags & (TCP_SYN | TCP_ACK)) == (TCP_SYN | TCP_ACK)) {
			c->ack = rseq + 1;
			tcp_emit(c, TCP_ACK, NULL, 0);
			c->established = 1;
			return 1;
		}
	}
	return 0;
}

static int tcp_request(tcp_conn_t* c, byte* req, nat16 req_len, char* buf, nat32 buf_size, nat32 timeout_ms) {
	tcp_emit(c, TCP_PSH | TCP_ACK, req, req_len);
	c->seq += req_len;

	nat32 total = 0;
	nat32 start = timer_ticks, ticks = timeout_ms / 20;

	while (timer_ticks - start < ticks && !c->peer_fin) {
		nat32 len; byte* pkt = rtl8139_poll_rx(&len);
		if (!pkt) continue;
		nat8 flags; nat32 rseq, rack; byte* d; nat16 dl;
		if (!tcp_match(c, pkt, len, &flags, &rseq, &rack, &d, &dl)) continue;

		if (dl > 0) {
			nat32 copy = dl;
			if (total + copy >= buf_size - 1) copy = buf_size - 1 - total;
			memcpy(buf + total, d, copy);
			total += copy;
			c->ack = rseq + dl;
			tcp_emit(c, TCP_ACK, NULL, 0);
		}
		if (flags & TCP_FIN) {
			c->ack++;
			c->peer_fin = 1;
			tcp_emit(c, TCP_ACK, NULL, 0);
			tcp_emit(c, TCP_FIN | TCP_ACK, NULL, 0);
			c->seq++;
		}
	}

	buf[total] = 0;
	return (int)total;
}

// HTTP
int net_http_get(const char* host, const char* path, char* buf, nat32 buf_size) {
	byte dst_ip[4];
	printf("resolving %s ...\n", host);
	if (!net_dns_resolve(host, dst_ip)) {
		print("DNS failed\n");
		return -1;
	}
	printf("  -> %d.%d.%d.%d\n", dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3]);

	byte dst_mac[6];
	if (!get_nexthop_mac(dst_ip, dst_mac)) {
		print("ARP failed\n");
		return -1;
	}

	tcp_conn_t conn;
	print("connecting...\n");
	if (!tcp_connect(&conn, dst_ip, 80, dst_mac)) {
		print("TCP connect failed\n");
		return -1;
	}
	print("connected\n");

	static byte req[256];
	int rlen = 0;
	// "GET /path HTTP/1.0\r\nHost: host\r\nConnection: close\r\n\r\n"
	const char* parts[] = { "GET ", path, " HTTP/1.0\r\nHost: ", host, "\r\nConnection: close\r\n\r\n" };
	for (int i = 0; i < 5; i++) {
		const char* s = parts[i];
		while (*s) req[rlen++] = (byte)*s++;
	}

	return tcp_request(&conn, req, (nat16)rlen, buf, buf_size, 8000);
}

int net_initialized = 0;

void init_network() {
	iobase = rtl8139_find();

	if (!iobase) {
		print("RTL8139 not found\n");
		return;
	}

	print("RTL8139 found\n");
	init_rtl8139();

	byte irq = (byte)(pci_config_read_word(RTL8139BUS, RTL8139SLOT, 0x00, 0x3C) & 0xFF);
	printf("RTL8139 IRQ: %d\n", IRQ_BASE + irq);

	byte* mac = get_mac();
	printf("MAC: %p:%p:%p:%p:%p:%p\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	printf("IP : %d.%d.%d.%d\n\n", our_ip[0], our_ip[1], our_ip[2], our_ip[3]);

	net_initialized = 1;
}

int net_tcp_open(tcp_conn_t* c, byte dst_ip[4], nat16 port) {
	byte mac[6];
	if (!get_nexthop_mac(dst_ip, mac)) return 0;
	return tcp_connect(c, dst_ip, port, mac);
}

void net_tcp_send(tcp_conn_t* c, byte* data, nat16 len) {
	tcp_emit(c, TCP_PSH | TCP_ACK, data, len);
	c->seq += len;
}

int net_tcp_recv(tcp_conn_t* c, byte* buf, nat32 buf_size) {
	nat32 pkt_len;
	byte* pkt = rtl8139_poll_rx(&pkt_len);
	if (!pkt) return 0;

	nat8 flags; nat32 rseq, rack; byte* d; nat16 dl;
	if (!tcp_match(c, pkt, pkt_len, &flags, &rseq, &rack, &d, &dl)) return 0;

	int written = 0;
	if (dl > 0) {
		nat32 copy = dl < buf_size - 1 ? dl : buf_size - 1;
		memcpy(buf, d, copy);
		buf[copy] = 0;
		written = (int)copy;
		c->ack = rseq + dl;
		tcp_emit(c, TCP_ACK, NULL, 0);
	}
	if (flags & TCP_FIN) {
		c->ack++;
		c->peer_fin = 1;
		tcp_emit(c, TCP_ACK, NULL, 0);
		return -1;
	}
	return written;
}

void net_tcp_close(tcp_conn_t* c) {
	tcp_emit(c, TCP_FIN | TCP_ACK, NULL, 0);
	c->seq++;
}

void net_udp_send_to(byte dst_ip[4], nat16 dst_port, byte* data, nat16 len) {
	byte mac[6];
	if (!get_nexthop_mac(dst_ip, mac)) return;
	udp_send(dst_ip, mac, 54321, dst_port, data, len);
}

int net_udp_recv(nat16 src_port, byte* buf, nat32 buf_size, nat32 timeout_ms) {
	nat32 start = timer_ticks, ticks = timeout_ms / 20;
	if (ticks == 0) ticks = 1;
	while (timer_ticks - start < ticks) {
		nat32 pkt_len;
		byte* pkt = rtl8139_poll_rx(&pkt_len);
		if (!pkt) continue;
		if (pkt_len < sizeof(eth_header_t) + sizeof(ip_header_t) + sizeof(udp_header_t)) continue;
		eth_header_t* eth = (eth_header_t*)pkt;
		if (htons(eth->ethertype) != ETHERTYPE_IP) continue;
		ip_header_t* iph = (ip_header_t*)(pkt + sizeof(eth_header_t));
		if (iph->protocol != IP_PROTO_UDP) continue;
		nat32 ihl = (iph->version_ihl & 0x0F) * 4;
		udp_header_t* udp = (udp_header_t*)((byte*)iph + ihl);
		if (htons(udp->dst_port) != src_port) continue;
		byte* d = (byte*)udp + sizeof(udp_header_t);
		nat32 dl = (nat32)(htons(udp->length) - sizeof(udp_header_t));
		nat32 copy = dl < buf_size - 1 ? dl : buf_size - 1;
		memcpy(buf, d, copy);
		buf[copy] = 0;
		return (int)copy;
	}
	return 0;
}

// todo: https maybe?
// would be cool