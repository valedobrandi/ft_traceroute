#include "ft_traceroute.h"
#include "argparse.h"

t_opts options;

static t_arg42 opts[] = {
    {NULL, '?', NULL, "Display help information"},
    {0, 0, 0, 0}
};

static int handle_options(int key, char *arg, void *user)
{
    t_args *args = user;

    args->ttl = 1;
    
    switch (key)
    {
    case '?':
        args->helper = 1;
    case 0:
        if (args->host)
            break;
        args->host = arg;
        break;
    default:
        return 1;
    }
    return 0;
}

struct sockaddr_in ft_getaddrinfo(char *host)
{
    struct sockaddr_in dest;
    struct addrinfo addressInfoHints, *addrInfoPointer;
    memset(&addressInfoHints, 0, sizeof(addressInfoHints));
    addressInfoHints.ai_family = AF_INET;
    addressInfoHints.ai_socktype = SOCK_RAW;

    int getaddrinfo_result = getaddrinfo(host, NULL, &addressInfoHints, &addrInfoPointer);
    if (getaddrinfo_result != 0)
    {
        fprintf(stderr, "ft_ping: unknown host\n");
        exit(1);
    }
    struct sockaddr *addr = addrInfoPointer->ai_addr;
    freeaddrinfo(addrInfoPointer);
    return *(struct sockaddr_in *)addr;
}

void setttl(int sockfd, int ttl)
{
    if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
    {
        perror("setsockopt"); 
        exit(1);
    }
    
}

int build_packet(char *buffer, int seq) {
    // Build ICMP Header
    struct s_icmp *icmp = (struct s_icmp *)buffer;
    memset(icmp, 0, sizeof(*icmp));
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->identifier= htons(getpid() & 0xFFFF);
    icmp->sequence = htons(seq);
    
    memset(buffer + sizeof(*icmp), 0, 56);

    size_t packet = sizeof(*icmp) + 56;
    icmp->checksum = 0;
    icmp->checksum = checksum(buffer, packet);
    return packet;
}

int main(int argc, char **argv)
{
    int sockfd;

    t_args args = {0};
    t_arg42_args main_args = {argc, argv};
    memset(&options, 0, sizeof(options));

    if (ft_argp(&main_args, opts, handle_options, &args) != 0 || args.helper) {
            print_help(opts); 
            exit(0);
    }

    if (!args.host) {
        fprintf(stderr, "Host not specified.\n");
        exit(1);
    }

    struct sockaddr_in dest = ft_getaddrinfo(args.host);

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    setttl(sockfd, args.ttl);

    return (0);
}