#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>
#include <features.h>
#include <signal.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h> 

typedef struct s_ping {
    int     packet_sent;
    int     packet_received;
    int     packet_mean;
    int     packet_len;
    int     ttl;
    char    *response;
}   t_ping;

typedef struct s_response {
    int     type;
    char    *string;
    char    *address;
}   t_response;

int     ft_ping(char *address);
void    opt_man(void);

void    print_ip_header(struct iphdr *ip);
void    print_icmp_header(struct icmphdr *icmp);
void    print_packet(char *packet, size_t len);
char    *get_ip_address_from_domain(char *address);