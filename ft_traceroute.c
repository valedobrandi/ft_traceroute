#include "argparse.h"
#include "ft_traceroute.h"

t_opts options;

static t_arg42 opts[] = {{"help", '?', NULL, "give this help list"}, {0, 0, 0, 0}};

static int handle_options(int key, char *arg, void *user)
{
    t_args *args;

    args = user;
    switch (key)
    {
    case '?':
    case 'h':
        args->helper = 1;
        break;
    case 0:
        if (args->host)
            break;
        args->host = arg;
        break;
    default:
        return (1);
    }
    return (0);
}

int icmp_generic_decode(unsigned char *buffer, struct udphdr **orig_udp, struct ip **ipp, icmphdr_t **icmpp)
{
    struct ip *ip = (struct ip *)buffer;
    size_t hlen = ip->ip_hl << 2;

    icmphdr_t *icmp = (icmphdr_t *)(buffer + hlen);
    *ipp = ip;
    *icmpp = icmp;

    if (icmp->type == 11 || icmp->type == 3)
    {
        struct ip *inner_ip = (struct ip *)(buffer + hlen + 8);
        int inner_hlen = inner_ip->ip_hl << 2;
        *orig_udp = (struct udphdr *)((unsigned char *)inner_ip + inner_hlen);
        return 0;
    }

    *orig_udp = NULL;
    return -1;
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        fprintf(stderr, "ft_traceroute: missing host operand.\n");
        printf("Try 'ft_traceroute --help' for more information.\n");
        exit(0);
    }

    t_args args = {0};
    t_arg42_args main_args = {argc, argv};
    memset(&options, 0, sizeof(options));

    if (ft_argp(&main_args, opts, handle_options, &args) != 0 || args.helper)
    {
        print_help(opts);
        exit(0);
    }

    if (!args.host)
    {
        fprintf(stderr, "ft_traceroute: missing host operand.\n");
        exit(1);
    }

    int send_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    int recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((getaddrinfo(args.host, NULL, &hints, &res)) != 0)
    {
        fprintf(stderr, "ft_traceroute: unknown host\n");
        exit(1);
    }
    struct sockaddr_in dest = *(struct sockaddr_in *)res->ai_addr;

    printf("traceroute to %s (%s), %d hops max\n",
           res->ai_canonname ? res->ai_canonname : args.host, inet_ntoa(dest.sin_addr), 64);

    freeaddrinfo(res);
    struct timeval start, now;

    for (int ttl = 1; ttl <= 64; ttl++)
    {
        int type = 0;
        struct udphdr *orig_udp = NULL;
        printf(" %2d  ", ttl);
        uint32_t prev_addr = 0;
        for (int probe = 0; probe < 3; probe++)
        {
            struct timeval timeout = {3, 0};
            setsockopt(send_sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
            static int dest_port = 33434;
            dest.sin_port = htons(dest_port++);

            char payload[56] = {0};
            gettimeofday(&start, NULL);
            sendto(send_sock, payload, sizeof(payload), 0, (struct sockaddr *)&dest, sizeof(dest));

            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(recv_sock, &fds);

            int ret = select(recv_sock + 1, &fds, NULL, NULL, &timeout);
            if (ret == 0)
            {
                printf(" * ");
                fflush(stdout);
                prev_addr = 0;
                continue;
            }

            unsigned char buf[CAPTURE_LEN] = {0};
            struct sockaddr_in from;
            socklen_t reply_len = sizeof(from);

            ssize_t n = recvfrom(recv_sock, buf, sizeof(buf), 0,
                                 (struct sockaddr *)&from, &reply_len);
            if (n < 0)
                exit(EXIT_FAILURE);

            struct ip *ip;
            icmphdr_t *ic;

            if(icmp_generic_decode(buf, &orig_udp, &ip, &ic) != 0 || !orig_udp)
                continue;
            if (orig_udp->uh_dport != dest.sin_port)
                continue;

            type = ic->type;

            gettimeofday(&now, NULL);

            double triptime = (now.tv_sec - start.tv_sec) * 1000.0 + (now.tv_usec - start.tv_usec) / 1000.0;

            if (prev_addr != from.sin_addr.s_addr)
                printf(" %s ", inet_ntoa(from.sin_addr));

            printf(" %.3fms ", triptime);
            prev_addr = from.sin_addr.s_addr;
        }
        printf("\n");
        if (type == 3)
            break;
    }
    close(send_sock);
    close(recv_sock);
    return 0;
}