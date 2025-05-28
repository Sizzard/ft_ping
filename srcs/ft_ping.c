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

char *get_icmp_response(int type_nb, int code) {
    char *type[44][16];
    char *res;

    // printf("type_nb : %d, code : %d\n", type_nb, code);

    if (type_nb < 0 || type_nb > 44 || code < 0 || code > 16) {
        return strdup("Bad parsing in response");
    }

    type[0][0] = "icmp_seq=%d, ping.ttl=%d, time=%.3f ms\n";
    type[1][0] = "reserved\n";
    type[2][0] = type[1][0];
    type[3][0] = "Destination network unreachable\n";
    type[3][1] = "Destination host unreachable\n";
    type[3][2] = "Destination protocol unreachable\n";
    type[3][3] = "Destination port unreachable\n";
    type[3][4] = "Fragmentation required, and DF flag set\n";
    type[3][5] = "Source route failed\n";
    type[3][6] = "Destination network unknown\n";
    type[3][7] = "Destination host unknown\n";
    type[3][8] = "Source host isolated\n";
    type[3][9] = "Network administratively prohibited\n";
    type[3][10] = "Host administratively prohibited\n";
    type[3][11] = "Network unreachable for ToS\n";
    type[3][12] = "Host unreachable for ToS\n";
    type[3][13] = "Communication administratively prohibited\n";
    type[3][14] = "Host Precedence Violation\n";
    type[3][15] = "Precedence cutoff in effect\n";
    type[5][0] = "Redirect Datagram for the Network\n";
    type[5][1] = "Redirect Datagram for the Host\n";
    type[5][2] = "Redirect Datagram for the ToS & network\n";
    type[5][3] = "Redirect Datagram for the ToS & host \n";
    type[8][0] =  "Echo request (used to ping)\n";
    type[9][0] =  "Router Advertisement\n";
    type[10][0] =  "Router discovery/selection/solicitation\n";
    type[11][0] =  "Time to live (TTL) expired in transit\n";
    type[11][1] =	"Fragment reassembly time exceeded\n";
    type[12][0] =  "Bad IP Header\n";
    type[12][1] = type[12][0];
    type[12][2] = type[12][0];
    type[13][0] =  "Timestamp\n";
    type[14][0] =  "Timestamp reply \n";

    res = strdup(type[type_nb][code]);

    return res;

}

char *get_dest_address(struct iphdr *ip) {
    struct sockaddr_in src;
    char *src_addr;

    src.sin_addr.s_addr = ip->saddr;
    src_addr = inet_ntoa(src.sin_addr);
    
    return strdup(src_addr);
}

t_response parse_response(void *buf, size_t bytes) {
    struct iphdr *ip = buf;
    struct icmphdr* icmp = buf + 20;
    t_response response;
    response.string = get_icmp_response(icmp->type, icmp->code);
    response.address = get_dest_address(ip);
    response.type = icmp->type;
    (void)bytes;
    return response;
}

int craft_echo_packet(char *packet) {
    int packet_len = 0;
    struct icmphdr *icmp;
    char *payload = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

    icmp = (struct icmphdr *) packet;

    icmp->type = 8;
    icmp->code = 0;
    icmp->un.echo.id = 0;
    icmp->un.echo.sequence = 0;
    icmp->checksum = 0;

    packet_len = sizeof(icmp) + strlen(payload);

    memcpy(packet + (sizeof(icmp)), payload, strlen(payload));

    icmp->checksum = get_checksum(icmp, packet_len);

    return packet_len;
}

int ft_ping(char *address) {
    char packet[1024] = {0};
    char recv_buf[1024] = {0};
    t_ping ping = {0};
    struct sockaddr_in addr;
    int sock = -1;
    
    ping.ttl = 5;

    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = inet_addr(address);

    
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == -1) {
        fprintf(stderr, "ft_ping: Failed to create a Raw Socket (probably a right issue)\n");
        return 1;
    }
    
    if (setsockopt(sock, IPPROTO_IP, IP_TTL, &ping.ttl, sizeof(ping.ttl))) {
        fprintf(stderr, "ft_ping: Failed to change ttl value: setsockopt()");
        return 1;
    }

    ping.packet_len = craft_echo_packet(packet);

    signal_handler();

    while (sigint_g != SIGINT) {
        int bytes = sendto(sock, packet, ping.packet_len, 0, (const struct sockaddr *)&addr, sizeof(addr));
        if (bytes > 0) {
            ping.packet_sent++;
        }

        bytes = recvfrom(sock, recv_buf, sizeof(recv_buf), 0, 0, 0);
        if (bytes > 0) {
            bytes -= 20;
            // print_packet(recv_buf + 20, bytes - 20);
            t_response response = parse_response(recv_buf, bytes);
            ping.response = response.string;
            printf("%d bytes : from %s: ", bytes, response.address);
            printf(ping.response , ping.packet_received, ping.ttl, 1.59999999);
            free(response.address);
            free(ping.response);
            if (response.type == 0) {
                ping.packet_received++;
            }
        }

        sleep(1);
    }

    if (ping.packet_received == 0) {
        ping.packet_mean = 100;
    }
    else {
        ping.packet_mean = (1 - ping.packet_sent / ping.packet_received) * 100;
    }

    printf("--- %s ping statistics ---\n", address);
    printf("%d packets transmitted, %d packets received, %d%% packet loss\n", ping.packet_sent, ping.packet_received, ping.packet_mean);
    close(sock);
    //  free() all
    return 0;
}

