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

char *get_icmp_response(int type_nb, int code) {
    char *type[44][16];
    char *res;

    // printf("type_nb : %d, code : %d\n", type_nb, code);

    if (type_nb < 0 || type_nb > 44 || code < 0 || code > 16) {
        return strdup("Bad parsing in response");
    }

    type[0][0] = "icmp_seq=%d ttl=%d time=%.3f ms\n";
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

t_response parse_response(void *buf) {
    struct iphdr *ip = buf;
    struct icmphdr* icmp = buf + 20;
    t_response response;
    response.string = get_icmp_response(icmp->type, icmp->code);
    response.address = get_dest_address(ip);
    response.type = icmp->type;
    return response;
}

void craft_ip_packet(char *packet, t_ping *ping) {
    int packet_len = 0;
    struct iphdr *ip = (struct iphdr*)packet;

    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0;
    ip->tot_len = htons(ping->tot_len);
    ip->id = htons(12345);
    ip->frag_off = 0;
    ip->ttl = ping->ttl;
    ip->protocol = IPPROTO_ICMP;
    ip->check = 0;
    ip->saddr = inet_addr(ping->src);
    ip->daddr = inet_addr(ping->dst);

    packet_len = sizeof(struct iphdr);

    ip->check = get_checksum(ip, packet_len);

    dump_ip_header(packet);
}

int craft_icmp_packet(char *packet, t_ping *ping) {
    int packet_len = 0;
    struct icmphdr *icmp;
    char *payload = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    int pid = getpid();

    icmp = (struct icmphdr *)(packet + sizeof(struct iphdr));

    icmp->type = 8;
    icmp->code = 0;
    icmp->un.echo.id = htons(pid);
    icmp->un.echo.sequence = htons(ping->packet_sent);
    icmp->checksum = 0;

    packet_len = sizeof(icmp) + strlen(payload);

    memcpy(packet + sizeof(struct iphdr) + (sizeof(icmp)), payload, strlen(payload));

    icmp->checksum = get_checksum(icmp, packet_len);

    return packet_len;
}

// typedef struct s_ping {
//     int     packet_sent;
//     int     packet_received;
//     int     packet_mean;
//     int     packet_len;
//     int     ttl;
//     char    *response;
//     char    *src;
//     char    *dst;
// }   t_ping;

t_ping *init_ping_packet(char *dst, int tot_len, int ttl) {
    t_ping *ping = malloc(sizeof(t_ping));

    char *src_address =  get_src_addr();
    if (src_address == NULL) {
        return NULL;
    }

    if (ping) {
        ping->packet_sent = 0;
        ping->packet_received = 0;
        ping->packet_mean = 0;
        ping->packet_len = 0;
        ping->ttl = ttl;
        ping->src = src_address;
        ping->dst = dst;
        ping->tot_len = tot_len;
    }
    return ping;
}

int ft_ping(char *real_address, char *address) {
    char packet[1024] = {0};
    char recv_buf[1024] = {0};
    t_ping *ping;
    struct sockaddr_in addr;
    bool verbose = true;
    int sock;
    int one = 1;

    ping = init_ping_packet(address, sizeof(struct iphdr) + 64, 63);
    if (ping == NULL) {
        fprintf(stderr, "ft_ping: Ran out of memory\n");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = inet_addr(address);

    craft_ip_packet(packet, ping);
    
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == -1) {
        fprintf(stderr, "ft_ping: Failed to create a Raw Socket (probably a right issue)\n");
        return 1;
    }

    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one))) {
        fprintf(stderr, "ft_ping: Failed to set header value: setsockopt()");
        return 1;
    }
    
    signal_handler();
    
    if (verbose) {
        printf("PING %s (%s): %d data bytes\n", real_address, address, ping->packet_len - 8);
    }
    else {
        printf("PING %s (%s): %d data bytes\n", real_address, address, ping->packet_len - 8);
    }
    
    while (sigint_g != SIGINT) {

        ping->packet_len = sizeof(struct iphdr) + craft_icmp_packet(packet, ping);

        int bytes = sendto(sock, packet, ping->packet_len, 0, (const struct sockaddr *)&addr, sizeof(addr));
        if (bytes > 0) {
            ping->packet_sent++;
        }

        // dump_packet(packet);

        bytes = recvfrom(sock, recv_buf, sizeof(recv_buf), 0, 0, 0);
        if (bytes > 0) {
            bytes -= 20;
            t_response response = parse_response(recv_buf);
            ping->response = response.string;
            printf("%d bytes : from %s: ", bytes, response.address);
            if (response.type == 0) {
                printf(ping->response , ping->packet_received, ping->ttl, 1.59999999);
                ping->packet_received++;
            }
            else {
                
            }
            free(response.address);
            free(ping->response);
        }
        sleep(1);
    }

    if (ping->packet_received == 0) {
        ping->packet_mean = 100;
    }
    else {
        ping->packet_mean = (1 - ping->packet_sent / ping->packet_received) * 100;
    }

    printf("--- %s ping statistics ---\n", real_address);
    printf("%d packets transmitted, %d packets received, %d%% packet loss\n", ping->packet_sent, ping->packet_received, ping->packet_mean);
    close(sock);
    //  free() all
    return 0;
}

