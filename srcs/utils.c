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

char *get_src_addr() {
    struct ifaddrs *ifaddr;
           int family, s;
           char host[NI_MAXHOST];

           if (getifaddrs(&ifaddr) == -1) {
               perror("getifaddrs");
               exit(EXIT_FAILURE);
           }

           /* Walk through linked list, maintaining head pointer so we
              can free list later. */

           for (struct ifaddrs *ifa = ifaddr; ifa != NULL;
                    ifa = ifa->ifa_next) {
               if (ifa->ifa_addr == NULL)
                   continue;

               family = ifa->ifa_addr->sa_family;

               if (family == AF_INET) {
                   s = getnameinfo(ifa->ifa_addr,
                           sizeof(struct sockaddr_in),
                           host, NI_MAXHOST,
                           NULL, 0, NI_NUMERICHOST);
                    if (s != 0) {
                        fprintf(stderr, "ERROR on getnameinfo()\n");
                        return NULL;
                    }
                    if (!strncmp("127.", host, 4) == 0 && !strncmp("0.", host, 2) == 0) {
                        freeifaddrs(ifaddr);
                        return strdup(host);
                    }
               }
           }

           freeifaddrs(ifaddr);
           return NULL;
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