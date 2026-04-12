#ifndef NETWORK_MAIN_H
#define NETWORK_MAIN_H

#include "types.h"

#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139
#define RTL8139_REG_COMMAND 0x37
#define RTL8139_CMD_RESET 0x10
#define RTL8139_CMD_RX_EN 0x08
#define RTL8139_CMD_TX_EN 0x04
#define RTL8139_REG_IMR 0x3C // interrupt mask register
#define RTL8139_REG_ISR 0x3E // interrupt status register
#define RTL8139_REG_RCR 0x44 // receive configuration register
#define RTL8139_REG_RBSTART 0x30 // receive buffer start
#define RTL8139_REG_CAPR 0x38 // current address of packet read
#define RTL8139_RX_OK 0x0001 // ISR: packet received
#define RTL8139_TX_OK 0x0004 // ISR: packet sent

#define RTL8139_DETECT_TIMEOUT 1000000

#define NET_OUR_IP { 10, 0, 2, 15 }
#define NET_GATEWAY { 10, 0, 2, 2 }
#define NET_NETMASK { 255,255,255,0 }

#define ETHERTYPE_ARP 0x0806
#define ETHERTYPE_IP 0x0800

typedef struct {
	byte dst[6];
	byte src[6];
	nat16 ethertype;
} __attribute__((packed)) eth_header_t;

#define ARP_HTYPE_ETHERNET 0x0001
#define ARP_PTYPE_IPV4 0x0800
#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY 2

typedef struct {
	nat16 htype; // hardware type 
	nat16 ptype; // protocol type 
	byte hlen; // hardware addr length
	byte plen; // protocol addr length
	nat16 oper; // operation 
	byte sha[6]; // sender hardware addr
	byte spa[4]; // sender protocol addr
	byte tha[6]; // target hardware addr
	byte tpa[4]; // target protocol addr
} __attribute__((packed)) arp_packet_t;

#define IP_PROTO_ICMP 1

typedef struct {
	byte version_ihl; // 0x45 for v4, no options
	byte dscp_ecn;
	nat16 total_length;
	nat16 id;
	nat16 flags_frag;
	byte ttl;
	byte protocol;
	nat16 checksum;
	byte src[4];
	byte dst[4];
} __attribute__((packed)) ip_header_t;

#define ICMP_TYPE_ECHO_REQUEST 8
#define ICMP_TYPE_ECHO_REPLY 0

typedef struct {
	byte type;
	byte code;
	nat16 checksum;
	nat16 id;
	nat16 seq;
} __attribute__((packed)) icmp_header_t;

typedef struct {
	nat32 txBuffer[4];
	nat32 rxBuffer;
	byte mac[6];
} rtl8139;

extern rtl8139 rtl8139Device;
extern int net_initialized;

#define IP_PROTO_TCP 6
#define IP_PROTO_UDP 17

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10

typedef struct {
	nat16 src_port, dst_port;
	nat32 seq, ack;
	nat8 offset;
	nat8 flags;
	nat16 window;
	nat16 checksum;
	nat16 urgent;
} __attribute__((packed)) tcp_header_t;

typedef struct {
	nat16 src_port, dst_port;
	nat16 length, checksum;
} __attribute__((packed)) udp_header_t;

typedef struct {
	byte dst_ip[4];
	byte dst_mac[6];
	nat16 src_port, dst_port;
	nat32 seq, ack;
	int established, peer_fin;
} tcp_conn_t;

void init_network();
byte* get_mac();
void send_frame(byte* data, int len);

int net_ping(byte dst_ip[4], nat32 timeout_ms, nat32 *rtt_ms);
int net_dns_resolve(const char* hostname, byte out_ip[4]);
int net_http_get(const char* host, const char* path, char* buf, nat32 buf_size);

int net_tcp_open(tcp_conn_t* c, byte dst_ip[4], nat16 port);
void net_tcp_send(tcp_conn_t* c, byte* data, nat16 len);
int net_tcp_recv(tcp_conn_t* c, byte* buf, nat32 buf_size);
void net_tcp_close(tcp_conn_t* c);

void net_udp_send_to(byte dst_ip[4], nat16 dst_port, byte* data, nat16 len);
int net_udp_recv(nat16 src_port, byte* buf, nat32 buf_size, nat32 timeout_ms);

#endif