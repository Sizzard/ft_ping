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

uint16_t get_checksum(const void *buf, size_t len) {
    uint32_t sum = 0;
    const uint16_t *packet = buf;

    while (len > 1) {
        sum += *packet++;
        len -= 2;
    }

    if (len > 0) {
        sum += *((const uint8_t *)packet);
    }

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return (uint16_t)(~sum);
}

void print_icmp_header(struct icmphdr *icmp) {
    puts("Parsing of icmp header :");
    printf("type :      %d\n", icmp->type);
    printf("code :      %d\n", icmp->code);
    printf("checksum :  %d\n\n", icmp->checksum);
}

char *get_local_ip_for_target(const char *target_ip) {
    static char local_ip[INET_ADDRSTRLEN];
    struct sockaddr_in serv;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
        return NULL;

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(53);
    inet_pton(AF_INET, target_ip, &serv.sin_addr);

    connect(sock, (struct sockaddr *)&serv, sizeof(serv));

    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    getsockname(sock, (struct sockaddr *)&name, &namelen);

    inet_ntop(AF_INET, &name.sin_addr, local_ip, sizeof(local_ip));

    close(sock);
    return local_ip;
}

int ft_ping(char *address) {
    int fd = -1;
    int packet_sent = 0;
    int packet_received = 0;
    int packet_mean = 0;
    int packet_len = 0;
    struct icmphdr *icmp;
    struct sockaddr_in addr;
    char *src_address = get_local_ip_for_target(address);
    char *dst_address = address;
    char packet[1024] = {0};
    char recv_buf[1024] = {0};

    char *payload = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

    if (!src_address) {
        fprintf(stderr, "ft_ping: Failed to get hostname\n");
        return 1;
    }

    icmp = (struct icmphdr *) packet;
    
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = inet_addr(dst_address);

    icmp->type = 8;
    icmp->code = 0;
    icmp->un.echo.id = 0;
    icmp->un.echo.sequence = 0;
    icmp->checksum = 0;

    // packet_len = sizeof(icmp);
    packet_len = sizeof(icmp) + sizeof(payload);

    icmp->checksum = get_checksum(icmp, packet_len);

    print_icmp_header(icmp);

    fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (fd == -1) {
        fprintf(stderr, "ft_ping: Failed to create a Raw Socket (probably a right issue)\n");
        return 1;
    }

    signal_handler();
    while (sigint_g != SIGINT) {
        int bytes = sendto(fd, packet, packet_len, 0, (const struct sockaddr *)&addr, sizeof(addr));
        if (bytes > 0) {
            packet_sent++;
        }

        // printf("\nSent %d bytes to %s\n", bytes, dst_address);

        bytes = recvfrom(fd, recv_buf, sizeof(recv_buf), 0, 0, 0);
        if (bytes > 0) {
            printf("%d bytes : from %s: ", bytes, dst_address);
            printf("icmp_seq=%d, ttl=%d, time=%.3f ms\n", packet_received, 64, 1.59999999);
            packet_received++;
        }


        sleep(1);
    }

    packet_mean = (1 - packet_sent / packet_received) * 100;

    printf("--- %s ping statistics ---\n", address);
    printf("%d packets transmitted, %d packets received, %d%% packet loss\n", packet_sent, packet_received, packet_mean);
    //  free() all
    return 0;
}
