#include "ft_ping.h"

int main(int ac, char **av) {
    int res;
    char *ip_address;

    if (ac == 1) {
        fprintf(stderr, "ft_ping: usage error: Destination address required\n");
        return 1;
    }

    ip_address = get_ip_address_from_domain(av[1]);
    res = ft_ping(av[1], ip_address);
    free(ip_address);
    return res;
}