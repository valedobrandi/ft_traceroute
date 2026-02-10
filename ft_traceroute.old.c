#include "argparse.h"
#include "ft_traceroute.h"

t_opts				options;

static t_arg42		opts[] = {{NULL, '?', NULL, "Display help information"}, {0,
			0, 0, 0}};

static int	handle_options(int key, char *arg, void *user)
{
	t_args	*args;

	args = user;
	args->ttl = 1;
	switch (key)
	{
	case '?':
		args->helper = 1;
		break ;
	case 0:
		if (args->host)
			break ;
		args->host = arg;
		break ;
	default:
		return (1);
	}
	return (0);
}

struct sockaddr_in	ft_getaddrinfo(char *host)
{
	struct sockaddr_in	dest;
	int					getaddrinfo_result;

	struct addrinfo addressInfoHints, *addrInfoPointer;
	memset(&addressInfoHints, 0, sizeof(addressInfoHints));
	addressInfoHints.ai_family = AF_INET;
	addressInfoHints.ai_socktype = SOCK_RAW;
	getaddrinfo_result = getaddrinfo(host, NULL, &addressInfoHints,
			&addrInfoPointer);
	if (getaddrinfo_result != 0)
	{
		fprintf(stderr, "ft_traceroute: unknown host\n");
		exit(1);
	}
	memcpy(&dest, addrInfoPointer->ai_addr, sizeof(struct sockaddr_in));
	freeaddrinfo(addrInfoPointer);
	return (dest);
}

struct sockaddr_in	ft_getnameinfo(char *host)
{
	struct sockaddr_in	dest;
	int					getaddrinfo_result;

	struct addrinfo addressInfoHints, *addrInfoPointer;
	memset(&addressInfoHints, 0, sizeof(addressInfoHints));
	addressInfoHints.ai_family = AF_INET;
	addressInfoHints.ai_socktype = SOCK_RAW;
	getaddrinfo_result = getaddrinfo(host, NULL, &addressInfoHints,
			&addrInfoPointer);
	if (getaddrinfo_result != 0)
	{
		fprintf(stderr, "ft_traceroute: unknown host\n");
		exit(1);
	}
	memcpy(&dest, addrInfoPointer->ai_addr, sizeof(struct sockaddr_in));
	freeaddrinfo(addrInfoPointer);
	return (dest);
}

void	set_ttl(int sockfd, int ttl)
{
	if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
	{
		printf("setsockopt");
		exit(1);
	}
}

unsigned short	checksum(void *b, int len)
{
	uint16_t	*buf;
	uint32_t	sum;

	buf = b;
	sum = 0;
	for (sum = 0; len > 1; len -= 2)
		sum += *buf++;
	if (len == 1)
		sum += *(uint8_t *)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return (uint16_t)(~sum);
}

int	build_packet(char *buffer, int seq)
{
	struct s_icmp	*icmp;
	size_t			packet;

	// Build ICMP Header
	icmp = (struct s_icmp *)buffer;
	memset(icmp, 0, sizeof(*icmp));
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->identifier = htons(getpid() & 0xFFFF);
	icmp->sequence = htons(seq);
	memset(buffer + sizeof(*icmp), 0, 56);
	packet = sizeof(*icmp) + 52;
	icmp->checksum = 0;
	icmp->checksum = checksum(buffer, packet);
	return (packet);
}

struct s_socket_header parse_header(char *buffer)
{
    struct iphdr *ipHeader = (struct iphdr *)buffer;
    struct icmphdr *icmp = (struct icmphdr *)(buffer + (ipHeader->ihl * 4));
    char *payload = buffer + (ipHeader->ihl * 4) + sizeof(struct icmphdr);

    struct s_socket_header header = {ipHeader, icmp, payload};
    return header;
}

char *
get_hostname (struct in_addr *addr)
{
  struct hostent *info =
    gethostbyaddr ((char *) addr, sizeof (*addr), AF_INET);
	if (info != NULL)
		return info->h_name;
	return inet_ntoa (*addr);
}

int	main(int argc, char **argv)
{
	int sockfd;
	char sender[1024];
	char receiver[1024];

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
		fprintf(stderr, "Host not specified.\n");
		exit(1);
	}

	struct sockaddr_in dest = ft_getaddrinfo(args.host);

	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0)
	{
		perror("socket");
		exit(1);
	}


	printf("traceroute to %s (%s), %d hops max %d bytes packets\n", args.host,
		inet_ntoa(dest.sin_addr), 30, 60);

    int seq = 1;
	while (seq <= 30)
	{
		int packet_size = build_packet(sender, seq);
        set_ttl(sockfd, seq);

        int packet = 3;

        while (1) {
            struct timeval start, now, timeout;

            // Set timeout for select 0.5 second timeout
            timeout.tv_sec = 0; 
            timeout.tv_usec = 500000;

            gettimeofday(&start, NULL);
            gettimeofday(&now, NULL);

            sendto(sockfd, sender, packet_size, 0, (struct sockaddr *)&dest, sizeof(dest));
            
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(sockfd, &rfds);
            
            int retval = select(sockfd + 1, &rfds, NULL, NULL, &timeout);
            if (retval == 0) {
                printf("%d ***\n", seq);
            } else if (retval < 0) {
                perror("select");
                exit(1);
            } 
			
            
            struct sockaddr_in r_addr;
            socklen_t len = sizeof(r_addr);
            
            recvfrom(sockfd, receiver, sizeof(receiver), 0, (struct sockaddr *)&r_addr, &len);
            size_t elapsed_time = (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0;
            
            struct s_socket_header packet_recv = parse_header(receiver);
            
            int type = packet_recv.icmpHeader->type;
			int code = packet_recv.icmpHeader->code;
            
            // struct sockaddr_in dest = ft_getaddrinfo(args.host);
            if (type == ICMP_TIME_EXCEEDED) {
                // if (packet == 3) {
                //     printf("%2d  %s ", seq, inet_ntoa(r_addr.sin_addr));
                // }
                // printf(" %.3ld ms", elapsed_time);
				char ip_str[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &r_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
				printf("TRACE - %s ", ip_str);
            } else if (type == ICMP_ECHOREPLY) {
                // if (packet == 3) {
                //     printf("%2d  %s", seq, inet_ntoa(r_addr.sin_addr));
                // }
                // printf(" %.3ld ms", elapsed_time);
				char ip_str[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &r_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
				printf("DESTINE - %s ", ip_str);
            }
            packet--;
        }
		printf("\n");
		seq++;
		
	}
	return (0);
}