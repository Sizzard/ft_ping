#include "../ft_ping.h"

extern unsigned char g_sigint;

char *get_icmp_response(int type_nb, int code) {
    char *type[44][16];
    char *res;

    // printf("type_nb : %d, code : %d\n", type_nb, code);

    if (type_nb < 0 || type_nb > 44 || code < 0 || code > 16) {
        return strdup("Bad parsing in response");
    }

    type[0][0] = "%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n";
    type[1][0] = "%d bytes from %s: reserved\n";
    type[2][0] = type[1][0];
    type[3][0] = "%d bytes from %s: Destination network unreachable\n";
    type[3][1] = "%d bytes from %s: Destination host unreachable\n";
    type[3][2] = "%d bytes from %s: Destination protocol unreachable\n";
    type[3][3] = "%d bytes from %s: Destination port unreachable\n";
    type[3][4] = "%d bytes from %s: Fragmentation required, and DF flag set\n";
    type[3][5] = "%d bytes from %s: Source route failed\n";
    type[3][6] = "%d bytes from %s: Destination network unknown\n";
    type[3][7] = "%d bytes from %s: Destination host unknown\n";
    type[3][8] = "%d bytes from %s: Source host isolated\n";
    type[3][9] = "%d bytes from %s: Network administratively prohibited\n";
    type[3][10] = "%d bytes from %s: Host administratively prohibited\n";
    type[3][11] = "%d bytes from %s: Network unreachable for ToS\n";
    type[3][12] = "%d bytes from %s: Host unreachable for ToS\n";
    type[3][13] = "%d bytes from %s: Communication administratively prohibited\n";
    type[3][14] = "%d bytes from %s: Host Precedence Violation\n";
    type[3][15] = "%d bytes from %s: Precedence cutoff in effect\n";
    type[5][0] = "%d bytes from %s: Redirect Datagram for the Network\n";
    type[5][1] = "%d bytes from %s: Redirect Datagram for the Host\n";
    type[5][2] = "%d bytes from %s: Redirect Datagram for the ToS & network\n";
    type[5][3] = "%d bytes from %s: Redirect Datagram for the ToS & host \n";
    type[8][0] = "%d bytes from %s: Echo request (used to ping)\n";
    type[9][0] = "%d bytes from %s: Router Advertisement\n";
    type[10][0] = "%d bytes from %s: Router discovery/selection/solicitation\n";
    type[11][0] = "%d bytes from %s: Time to live (TTL) expired in transit\n";
    type[11][1] =	"Fragment reassembly time exceeded\n";
    type[12][0] = "%d bytes from %s: Bad IP Header\n";
    type[12][1] = type[12][0];
    type[12][2] = type[12][0];
    type[13][0] = "%d bytes from %s: Timestamp\n";
    type[14][0] = "%d bytes from %s: Timestamp reply \n";

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

float parse_time_stamp(char *packet) {
    unsigned long time;
    float res = 0;    

    // for (size_t i = 28; i < 36; i++) {
    //     printf("%x ", (unsigned char)packet[i]);
    // }
    memcpy(&time, &packet[28], 8);

    // printf("time : %ld\n", time);
    
    res = getTimeStamp() - time;

    res /= 1000;
    
    // printf("res = %f\n", res);

    return res;
}

t_response parse_response(void *buf, int bytes) {
    struct iphdr *ip = buf;
    struct icmphdr* icmp = buf + 20;
    t_response response;

    response.time = -1;
    response.string = get_icmp_response(icmp->type, icmp->code);
    response.address = get_dest_address(ip);
    response.type = icmp->type;
    if (bytes >= 24 && icmp->type == 0) {
        response.time = parse_time_stamp(buf);
    }
    if (icmp->type != 0) {
        icmp = buf + 48;
    }
    response.sequence = ntohs(icmp->un.echo.sequence);
    response.id = ntohs(icmp->un.echo.id);

    // for(size_t i = 26; i < 56; i++) {
    //     printf("%02x ", (unsigned char)icmp_2[i]);
    // }

    return response;
}

void craft_ip_packet(char *packet, t_ping *ping) {
    int packet_len;
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
}

int create_socket() {
    int opt_value = 1;
    int sock;

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == -1) {
        fprintf(stderr, "ft_ping: Failed to create a Raw Socket (probably a right issue)\n");
        return -1;
    }

    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &opt_value, sizeof(opt_value))) {
        fprintf(stderr, "ft_ping: Failed to set header value: setsockopt()");
        return -1;
    }
    return sock;
}

t_ping *init_ping_packet(char *dst, int tot_len) {
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
        ping->ttl = args.ttl;
        ping->src = src_address;
        ping->dst = dst;
        ping->tot_len = tot_len;
        ping->id = getpid();
        ping->stats.min = -1;
        ping->stats.max = -1;
        ping->stats.avg = -1;
        ping->stats.std = 0;
    }
    return ping;
}

int craft_icmp_packet(char *packet, t_ping *ping) {
    int packet_len = 0;
    struct icmphdr *icmp;
    char payload[2048] = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    unsigned long time = getTimeStamp();

    icmp = (struct icmphdr *)(packet + sizeof(struct iphdr));

    icmp->type = 8;
    icmp->code = 0;
    icmp->un.echo.id = htons(ping->id);
    icmp->un.echo.sequence = htons(ping->packet_sent);
    icmp->checksum = 0;

    packet_len = ping->tot_len - 20;

    memcpy(payload, (char *)&time, 8);

    memcpy(packet + sizeof(struct iphdr) + (sizeof(icmp)), payload, ping->tot_len - 28);

    icmp->checksum = get_checksum(icmp, packet_len);

    return packet_len;
}

void update_avg(t_ping *ping, float response_time) {
    float delta = response_time - ping->stats.avg;
    ping->stats.avg = ping->stats.avg + (delta / ping->packet_received);
    float delta2 = response_time - ping->stats.avg;
    ping->stats.sqr_sum += delta * delta2;
}

float get_stddev(t_ping *ping) {
    if (ping->packet_received < 2) return 0.0;
    return sqrt(ping->stats.sqr_sum / (ping->packet_received - 1));
}

void update_ping_stats(t_ping *ping, float response_time) {
    if (ping->stats.min == -1) {
        ping->stats.min = response_time;
        ping->stats.avg = response_time;
        ping->stats.max = response_time;
        ping->stats.sqr_sum = 0;
        return;
    }
    else if (ping->stats.min > response_time) {
        // printf("New min : %f\n", response_time);
        ping->stats.min = response_time;
    }
    else if (ping->stats.max < response_time){
        // printf("New max : %f\n", response_time);
        ping->stats.max = response_time;
    }
    update_avg(ping, response_time);
    ping->stats.std = get_stddev(ping);

}

int ft_ping(char *real_address, char *address) {
    char packet[2048] = {0};
    char recv_buf[2048] = {0};
    t_ping *ping;
    struct sockaddr_in addr;
    int sock;

    ping = init_ping_packet(address, sizeof(struct iphdr) + args.size + 8);
    if (ping == NULL) {
        fprintf(stderr, "ft_ping: Ran out of memory\n");
        free(ping);
        return 1;
    }
    // printf("address : %s\n", address);

    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = inet_addr(address);

    
    sock = create_socket();
    if (sock == -1) {
        free(ping);
        return 1;
    }
    
    struct pollfd popol;
    
    popol.fd = sock;
    popol.events = POLLIN | POLLOUT;
    
    fcntl(sock, F_SETFL, O_NONBLOCK);
    
    if (args.verbose) {
        printf("PING %s (%s): %d data bytes, id 0x%04x = %04d\n", real_address, address, args.size, ping->id, ping->id);
    }
    else {
        printf("PING %s (%s): %d data bytes\n", real_address, address, args.size);
    }

    signal_handler();

    craft_ip_packet(packet, ping);

    poll(&popol, 1, 0);
    
    while (g_sigint != SIGINT && ping->packet_sent != args.count) {

        ping->packet_len = sizeof(struct iphdr) + craft_icmp_packet(packet, ping);

        int bytes = 0;
        if (!(popol.revents & POLLOUT) ) {
            fprintf(stderr, "ft_ping: sending packet: No buffer space available\n");
            close(sock);
            free(ping->src);
            free(ping);
            return 1;
        }
        bytes = sendto(sock, packet, ping->packet_len, 0, (const struct sockaddr *)&addr, sizeof(addr));
        // printf("Sended %d bytes\n", bytes);
        if (bytes <= 0) {
            fprintf(stderr, "ft_ping: sending packet: No buffer space available\n");
            close(sock);
            free(ping->src);
            free(ping);
            return 1;
        }
        ping->packet_sent++;

        unsigned long og_time;
        og_time = getTimeStamp();

        do {
            poll(&popol, 1, 0);
            // dump_packet(packet);
            if (popol.revents & POLLIN) {
                bytes = recvfrom(sock, recv_buf, sizeof(recv_buf), 0, 0, NULL);
                if (bytes > 0) {
                    // printf("Received %d bytes\n", bytes);
                    bytes -= 20;
                    t_response response = parse_response(recv_buf, bytes);
                    if (response.id != ping->id) {
                        printf("OTHER ICMP PACKET\n");
                        free(response.address);
                        free(response.string);
                        continue;
                    }
                    ping->response = response.string;
                    if (response.time == -1) {
                        ping->response[36] = '\n';
                        ping->response[37] = 0;
                    }
                    if (response.type == 0) {
                        ping->packet_received++;
                        update_ping_stats(ping, response.time);
                        if (args.quiet == false) {
                            printf(ping->response , bytes, response.address, response.sequence, ping->ttl, response.time);
                        }
                    }
                    else {
                        printf(ping->response , bytes, response.address, response.sequence, ping->ttl, response.time);
                        if (args.verbose) {
                            dump_ip_header(recv_buf);
                            print_icmp_header(packet, bytes);
                        }
                    }
                    free(response.address);
                    free(ping->response);
                }
            }
            usleep(200);
        }
        while(getTimeStamp() - og_time < 1000000 * args.interval && g_sigint != SIGINT);
        usleep(200);
    }

    if (ping->packet_received == 0) {
        ping->packet_mean = 100;
    }
    else if (ping->packet_received == ping->packet_sent) {
        ping->packet_mean = 0;
    }
    else {
        ping->packet_mean = (ping->packet_received / ping->packet_sent) * 100;
    }

    printf("--- %s ping statistics ---\n", real_address);
    printf("%d packets transmitted, %d packets received, %.0f%% packet loss\n", ping->packet_sent, ping->packet_received, ping->packet_mean);
    if (ping->packet_received != 0 && ping->stats.min != -1) {
        printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", ping->stats.min, ping->stats.avg, ping->stats.max, ping->stats.std);
    }
    close(sock);
    free(ping->src);
    free(ping);
    return 0;
}

