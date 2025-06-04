#include "../ft_ping.h"

char *get_ip_address(struct sockaddr *addr) {
    char ipstr[32];
    void *addr_ptr;

    if (addr->sa_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)addr;
        addr_ptr = &(ipv4->sin_addr);
    } else {
        printf("Unsupported address family\n");
        return NULL;
    }

    inet_ntop(addr->sa_family, addr_ptr, ipstr, sizeof(ipstr));
    return strdup(ipstr);
}

char *get_ip_address_from_domain(char *address) {
    struct addrinfo hints, *result, *rp;
    char *res;
    int s;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;

    s = getaddrinfo(address, NULL, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo:\n");
        return NULL;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        res = get_ip_address(rp->ai_addr);
    }

    freeaddrinfo(result);
    return res;
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

unsigned long getTimeStamp(void) {
    struct timeval tv;
    unsigned long time;
    
    gettimeofday(&tv, NULL);

    time = 1000000 * tv.tv_sec + tv.tv_usec;

    return time;
}

bool is_num(char *str) {
    while(*str) {
        if (*str < '0' || *str > '9') {
            return false;
        }
        str++;
    }
    return true;
}

double get_packet_mean(t_ping *ping) {
    double sent = ping->packet_sent;
    double received = ping->packet_received;
    
    double res = (((received / sent) * 100) - 100);

    if (res < 0) {
        res *= -1;
    }

    return res;
}