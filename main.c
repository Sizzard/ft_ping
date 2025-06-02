#include "ft_ping.h"

unsigned char g_sigint = 0;
t_args args;

void print_args() {
    printf("verbose : %d\n", args.verbose);
    printf("quiet : %d\n", args.quiet);
    printf("size : %d\n", args.size);
    printf("count : %d\n", args.count);
    printf("interval : %ld\n", args.interval);
    printf("ttl : %d\n", args.ttl);
}

void init_args() {
    int max_size = 65399;
    (void)max_size;
    args.verbose    =  0;
    args.quiet      =  0;
    args.size       =  56;
    args.count      =  -1;
    args.interval   =  1;
    args.ttl        =  64;
}

int main(int ac, char **av) {
    int res;
    char *ip_address;

    if (ac == 1) {
        fprintf(stderr, "ft_ping: usage error: Destination address required\n");
        return 1;
    }

    init_args();

    int c;
    int option_index = 0;
    static struct option long_options[] =
        {
            {"verbose",   no_argument,            0,     'v'},
            {"quiet",     no_argument,            0,     'q'},
            {"size",      required_argument,      0,     's'},
            {"count",     required_argument,      0,     'c'},
            {"interval",  required_argument,      0,     'i'},
            {"ttl",       required_argument,      0,      0},
            {0, 0, 0, 0}
        };

    while ( (c = getopt_long(ac, av, "vqs:c:i:", long_options, &option_index)) != -1) {
        switch (c) {
            case 0 :
                if (strcmp(long_options[option_index].name, "ttl") == 0) {
                    args.ttl = atoi(optarg);
                    // printf("option ttl with value %d\n", args.ttl);
                }
                break;
            case 'v' :
                // puts("option -v\n");
                args.verbose = true;
                break;
            case 'q' :
                // puts("option -q\n");
                args.quiet = true;
                break;
            case 's' :
                // printf("option -s with value %s\n", optarg);
                args.size = atoi(optarg);
                break;
            case 'c' :
                // printf("option -c with value %s\n", optarg);
                args.count = atoi(optarg);
                break;
            case 'i' :
                // printf("option -i with value %s\n", optarg);
                args.interval = atoi(optarg);
                break;
            default :
                return 1;
        }
    }

    if (args.count == 0) {
        args.count = -1;
    }

    ip_address = get_ip_address_from_domain(av[ac - 1]);
    // print_args();
    res = ft_ping(av[ac - 1], ip_address);
    free(ip_address);
    return res;
}