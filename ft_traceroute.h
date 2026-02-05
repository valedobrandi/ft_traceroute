#ifndef FT_TRACEROUTE_H
#define FT_TRACEROUTE_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h> 
#include <netinet/ip.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>


struct s_socket_header
{
    struct iphdr *ipHeader;
    struct icmphdr *icmpHeader;
    char *payload;
};

typedef struct s_args
{
    int helper;
    int ttl;
    char *host;
}   t_args;

typedef struct s_opts {
    int helper;
} t_opts;

typedef struct s_icmp
{
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence;
} t_icmp;

#endif