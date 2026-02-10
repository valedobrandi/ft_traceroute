#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>

#define ICMP_MINLEN 8
#define CAPTURE_LEN 136

typedef struct icmphdr {
    uint8_t  type;      
    uint8_t  code;
    uint16_t checksum;
    union {
        struct {
            uint16_t id;
            uint16_t sequence;
        } echo;
        uint32_t gateway;
        struct {
            uint16_t __unused;
            uint16_t mtu;
        } frag;
    } un;
} icmphdr_t;   

int
icmp_generic_decode(unsigned char *buffer, size_t bufsize, struct ip **ipp, icmphdr_t **icmpp)
{
    size_t hlen;
    unsigned short cksum;
    struct ip *ip;
    icmphdr_t *icmp;

    /* IP header */
    ip = (struct ip *) buffer;
    hlen = ip->ip_hl << 2;
    if (bufsize < hlen + ICMP_MINLEN)
        return -1;
    /* ICMP header */
    icmp = (icmphdr_t *) (buffer + hlen);

    *ipp = ip;
    *icmpp = icmp;

    return 0;
}

int main(int argc, char **argv)
{
    char *host;

    int send_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    int recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	
	if ((getaddrinfo(argv[1], NULL, &hints, &res)) != 0)
	{
		fprintf(stderr, "ft_traceroute: unknown host\n");
		exit(1);
	}
    struct sockaddr_in dest = *(struct sockaddr_in *)res->ai_addr;
	freeaddrinfo(res);

    printf("traceroute to %s\n", inet_ntoa(dest.sin_addr));

    struct timeval start, now;
    gettimeofday(&start, NULL);
    for (int ttl = 1; ttl <= 30; ttl++)
    {
        struct timeval timeout = {5,0};
        setsockopt(send_sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
        dest.sin_port = htons(33434 + ttl);

        char payload[56] = {0};
        sendto(send_sock, payload, sizeof(payload), 0, (struct sockaddr *)&dest, sizeof(dest));

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(recv_sock, &fds);

        int ret = select(recv_sock + 1, &fds, NULL, NULL, &timeout);
        if (ret == 0)
        {
            printf("%2d ***\n", ttl);
            continue;
        }

        unsigned char buf[CAPTURE_LEN] = {0};
        struct sockaddr_in reply_addr;
        socklen_t reply_len = sizeof(reply_addr);

        ssize_t n = recvfrom(recv_sock, buf, sizeof(buf), 0, 
                                (struct sockaddr *)&reply_addr, &reply_len);
        if (n < 0)
            exit(EXIT_FAILURE);

        struct ip *ip;
        icmphdr_t *ic;

        icmp_generic_decode (buf, sizeof (buf), &ip, &ic);

         int type = ic->type;
         int code = ic->code;
        
        gettimeofday(&now, NULL);

        long rtt = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_usec - start.tv_usec) / 1000;

        printf("%2d %s %ld ms\n", ttl , inet_ntoa(reply_addr.sin_addr), rtt);

        if (type == 3) {
            break;
        }
    }
    close(send_sock);
    close(recv_sock);
    return 0;
}