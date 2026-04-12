#include "network/main.h"
#include "globals.h"

// to anyone reading this: IM sorry BUT IT FUCKING WORKS!!
// todo: make cleaner

static nat32 iobase;
static nat8  RTL8139SLOT;
static nat8  RTL8139BUS;
static nat16 rx_read_ptr; // software RX ring read pointer
static nat8  tx_slot; // which of the 4 TX descriptors to use next

rtl8139 rtl8139Device;

static const byte our_ip[4]   = NET_OUR_IP;
static const byte gateway[4]  = NET_GATEWAY;

static byte gw_mac[6];
static int  gw_mac_valid = 0;

static inline nat16 htons(nat16 x) {
	return (nat16)((x >> 8) | (x << 8));
}
static inline nat32 htonl(nat32 x) {
	return ((x >> 24) & 0xFF)
	     | ((x >>  8) & 0x0000FF00)
	     | ((x <<  8) & 0x00FF0000)
	     | ((x << 24) & 0xFF000000);
}

static nat16 ip_checksum(void* data, nat32 len) {
	nat16* ptr = (nat16*)data;
	nat32  sum = 0;
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
					RTL8139BUS  = (nat8)bus;
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

	memcpy(eth->dst, dst_mac,           6);
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
}