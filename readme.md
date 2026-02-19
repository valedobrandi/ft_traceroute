*SOCK_DGRAM (UDP) - Transport Layer 4*

> features:
Automatically builds UDP & IP headers.
A standard UDP socket cannot see ICMP error messages

*SOCK_RAW (ICMP) - Transport Layer 3*

> features:
OS builds IP header. You build ICMP header.

> Header:
[IP Header][ICMP Header][IP Header + first 8 bytes of the UDP header]


1.1.1.1 (Cloudflare)
9.9.9.9 (Quad9)
127.0.0.1
www.free.fr
www.whitehouse.gov
