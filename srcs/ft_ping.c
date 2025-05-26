#include "../ft_ping.h"

static unsigned char sigint_g = 0;

void sigint_handler_func(int signo) {
    if (signo == SIGINT) {
        sigint_g = signo;
        printf("\nCtrl-C pressed quitting ...\n");
    }
}

void signal_handler() {
    struct sigaction act;
    
    bzero(&act, sizeof(act));
    act.sa_handler = &sigint_handler_func;
    sigaction(SIGINT, &act, NULL);
}

int ft_ping(char *address) {
    int fd = -1;
    int optval = 0;
    struct icmphdr *icmp;
    struct sockaddr_in addr;
    struct iphdr *ip;
    char *src_address = "0.0.0.0";
    char *dst_address = address;
    (void)address;
    char *packet;

    packet = malloc(1024);

    ip = (struct iphdr *)packet;
    icmp = (struct icmphdr *) (packet + sizeof(struct iphdr));

    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->un.echo.id = 123;
    icmp->un.echo.sequence = 1;

    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = inet_addr(dst_address);

    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
    ip->id = htons(0);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = 1;
    ip->saddr = inet_addr(src_address);
    ip->daddr = inet_addr(dst_address);
    ip->check = 0;

    fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (fd == -1) {
        fprintf(stderr, "ft_ping: Failed to create a Raw Socket (probably a right issue)\n");
        return 1;
    }

    if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(int) != 0)) {
        fprintf(stderr, "ft_ping: Failed setsockopt\n");
        return 1;
    }

    int res = sendto(fd, &packet, ip->tot_len, 0, (const struct sockaddr *)&addr, sizeof(addr));

    printf("sent %d bytes to %s\n", res, dst_address);

    for (int i = 0; i < res; i++) {
        printf("%d ",packet[i]);
    }

    printf("\n");

    char buffer[1024] = {0};
    res = recvfrom(fd, buffer, sizeof(buffer), 0, 0, 0);
    printf("Received %d bytes : from %s\n", res, dst_address);

    for (int i = 0; i < res; i++) {
        printf("%d ", buffer[i]);
    }

    printf("\n");

    signal_handler();
    while (sigint_g != SIGINT) {

    }

    //  free() all
    return 0;
}
