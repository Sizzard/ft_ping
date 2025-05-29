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
#include <ifaddrs.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/select.h>
#include <errno.h> 

typedef struct s_ping {
    double       packet_sent;
    double       packet_received;
    double       packet_mean;
    int          packet_len;
    int          ttl;
    int          tot_len;
    char         *response;
    char         *src;
    char         *dst;
}   t_ping;

typedef struct s_response {
    int     type;
    char    *string;
    char    *address;
}   t_response;

int         ft_ping(char *real_address, char *address);
void        opt_man(void);

void        print_ip_header(void *packet);
void        print_icmp_header(void *packet);
void        dump_packet(char *packet);
void        dump_ip_header(void *packet);
void        print_packet(char *packet, size_t len);
char        *get_src_addr();
char        *get_ip_address_from_domain(char *address);
uint16_t    get_checksum(const void *buf, size_t len);