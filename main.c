#include "ft_ping.h"

int main(int ac, char **av) {
    if (ac == 1) {
        fprintf(stderr, "ft_ping: usage error: Destination address required\n");
        return 1;
    }
    return ft_ping(av[1]);
}