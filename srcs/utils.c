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