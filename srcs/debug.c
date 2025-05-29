#include "../ft_ping.h"

void print_ip_header(void *packet) {
    struct iphdr *ip = packet;
    struct sockaddr_in src, dst;
    char *dst_addr;
    char *src_addr;

    puts("\nParsing of ip header :");
    printf("tos :       %d\n", ip->tos);
    printf("tot_len :   %d\n", ip->tot_len);
    printf("id :        %d\n", ip->id);
    printf("frag_off :  %d\n", ip->frag_off);
    printf("ttl :       %d\n", ip->ttl);
    printf("protocol :  %d\n", ip->protocol);
    printf("check :     %d\n", ip->check);

    src.sin_addr.s_addr = ip->saddr;
    src_addr = inet_ntoa(src.sin_addr);
    printf("saddr :     %s\n", src_addr);

    dst.sin_addr.s_addr = ip->daddr;
    dst_addr = inet_ntoa(dst.sin_addr);
    printf("daddr :     %s\n", dst_addr);
    printf("\n");
}

// struct ip_test {
//     uint32_t tos;
//     uint32_t tot_len;
//     uint32_t id;
//     uint32_t frag_off;
//     uint32_t ttl;
//     uint32_t protocol;
//     uint32_t check;
//     uint32_t saddr;
//     uint32_t daddr;
// };

void dump_ip_header(char *packet) {
    struct iphdr *ip = (void *)packet;
    struct sockaddr_in src, dst;
    char *dst_addr;
    char *src_addr;

    printf("IP Hdr Dump:\n");
    for (size_t i = 0; i < 20; i += 2) {
        printf(" %02x%02x", (unsigned char)packet[i], (unsigned char)packet[i + 1]);
    }
    printf("\n");
    printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src	Dst	Data\n");
    printf(" %x", ip->version);
    printf(" %x", ip->ihl);
    printf("   %02x", ip->tos);
    printf(" %04x", htons(ip->tot_len));
    printf(" %x", ip->id);
    printf("   %x", ip->frag_off);
    printf(" %04x", ip->frag_off);
    printf("  %02x", ip->ttl);
    printf("  %02x", ip->protocol);
    printf(" %04x", ip->check);

    src.sin_addr.s_addr = ip->saddr;
    src_addr = inet_ntoa(src.sin_addr);
    printf(" %s", src_addr);

    dst.sin_addr.s_addr = ip->daddr;
    dst_addr = inet_ntoa(dst.sin_addr);
    printf("  %s\n", dst_addr);
}

void dump_packet(char *packet) {
    printf("Packet Dump:\n");
    for (size_t i = 0; i < 1024; i+=2) {
        printf(" %02x%02x", (unsigned char)packet[i], (unsigned char)packet[i + 1]);
    }
    printf("\n");
}

void print_icmp_header(void *packet) {
    struct icmphdr *icmp = packet + 20;
    printf("\nParsing of icmp header :");
    printf("type :      %d\n", icmp->type);
    printf("code :      %d\n", icmp->code);
    printf("checksum :  %d\n\n", icmp->checksum);
}

void print_packet(char *packet, size_t len) {
    printf("Packet_len is %ld\n", len);
    for(size_t i = 0; i < len;i++) {
        printf("%02x ", packet[i]);
    }
    printf("\n");
}
