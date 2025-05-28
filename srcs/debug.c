#include "../ft_ping.h"

void print_ip_header(struct iphdr *ip) {
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

void print_icmp_header(struct icmphdr *icmp) {
    puts("\nParsing of icmp header :");
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
