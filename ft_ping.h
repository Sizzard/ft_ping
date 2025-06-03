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
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <getopt.h>

typedef struct s_stats {
    float        min;
    float        avg;      
    float        max;
    float        std;
    float        sqr_sum;
}   t_stats;


typedef struct s_ping {
    double       packet_mean;
    int          packet_received;
    int          packet_sent;
    int          packet_len;
    int          ttl;
    int          tot_len;
    int          id;
    char         *response;
    char         *src;
    char         *dst;
    t_stats      stats;

}   t_ping;

typedef struct s_response {
    float        time;
    int          type;
    int          id;
    int          sequence;
    char         *string;
    char         *address;
}   t_response;

typedef struct s_args {
    bool              verbose;
    bool              quiet;
    long unsigned int interval;
    int               size;
    int               count;
    int               ttl;
}   t_args;

extern unsigned char    g_sigint;
extern t_args           args;

int         ft_ping(char *real_address, char *address);
void        opt_man(void);

void            sigint_handler_func(int signo);
void            signal_handler(void);

bool            is_num(char *str);

void            print_ip_header(void *packet);
void            print_icmp_header(void *packet, int bytes);
void            dump_packet(char *packet);
void            dump_ip_header(void *packet);
void            print_packet(char *packet, size_t len);
unsigned long   getTimeStamp(void); 
char            *get_src_addr();
char            *get_ip_address_from_domain(char *address);
uint16_t        get_checksum(const void *buf, size_t len);