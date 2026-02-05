

# ft_traceroute

42 Project: minimalist reimplementation of the traceroute utility.

---

## Goal

This project aims to implement a small, functional version of the `traceroute` utility (called `ft_traceroute`) as a networking and sockets exercise in C. The implementation focuses on the essentials: sending probe packets with increasing TTL values and receiving ICMP responses from intermediate routers and the destination host to map the path to an IP/hostname.

The repository contains the implementation, build and run instructions, and notes about behavior and limitations.

## Requirements

- POSIX-compliant system (Linux recommended)
- C compiler (gcc)
- Sufficient privileges to open raw sockets (usually run as root or with appropriate capabilities)

## Build

Use the provided Makefile. From the project directory run:

```bash
make
```

This should produce the `ft_traceroute` executable.

To remove build artifacts:

```bash
make clean
```

## Usage

Basic syntax:

```bash
./ft_traceroute <destination>
```

Example:

```bash
sudo ./ft_traceroute example.com
```

Notes:
- Some systems require root privileges to send/receive certain packet types (raw sockets). Use `sudo` if necessary.
- This project implements the minimal traceroute behavior: incrementing TTL per iteration and waiting for ICMP replies to map the route.

## Flags and options (generic)

Specific flags depend on your implementation. Common options include:

- `-m <max_ttl>`: set maximum TTL (default 30)
- `-p <port>`: destination port (when applicable)
- `-q <n>`: number of probes per hop
- `-h` or `--help`: display help

If your `ft_traceroute` supports other flags, check `argparse.c`/`argparse.h` for details.

## Expected output

Typical output lists hops with IP (or reverse name) and probe RTTs:

```
traceroute to example.com (93.184.216.34), 30 hops max, 60 byte packets
 1  10.0.0.1 (10.0.0.1)  1.123 ms  0.987 ms  1.045 ms
 2  192.0.2.1 (192.0.2.1)  10.456 ms  10.789 ms  10.512 ms
 3  * * *
 4  93.184.216.34 (93.184.216.34)  50.123 ms  49.987 ms  50.045 ms
```

Lines with `*` indicate that probes for that hop timed out (no reply).

## Implementation notes

- The program sends probe packets with TTL starting at 1 and increments until the maximum TTL or the destination is reached.
- For each TTL, N probes are sent (commonly 3) and latencies are measured using timestamps at send/receive.
- The program listens for ICMP "Time Exceeded" messages (when a router discards a packet because TTL expired) and possibly "Destination Unreachable" messages to identify the destination.
- Reverse DNS lookup (IP↔hostname) is optional but helpful to display hostnames when available.

Common limitations:
- Privilege requirements (raw sockets typically require root)
- Firewalls and network policies may block ICMP/UDP/TCP probes
- RTT accuracy depends on system timing resolution and measurement approach

## Repository layout

- `ft_traceroute.c`, `ft_traceroute.h` — main implementation
- `argparse.c`, `argparse.h` — command-line argument parsing
- `Makefile` — build rules
- `README.md` — this file

## Testing and verification

Manual tests recommended:

1. Build with `make`
2. Run against a known host (e.g. `google.com`, `example.com`) with `sudo` if required
3. Verify multiple hops are shown and that the program ends when the destination is reached or the max TTL is exceeded

Example:

```bash
sudo ./ft_traceroute 8.8.8.8
```

Automation: simple shell scripts can automate runs against controlled targets and validate output.

## Best practices & next steps (optional / bonus)

- Add IPv6 support
- Support multiple probe types (ICMP Echo, UDP, TCP SYN)
- Improve timing/metrics reporting (statistics, percentiles)
- Implement adaptive timeouts and retransmissions

## Author

Implementation by: (your name) — add your name / 42 ID here.

## License

For study purposes. Choose or add a license as needed.

---

Note: This README is written in the 42 project style (clear build/run steps, constraints). If you want, I can update the flags section to reflect exactly what's implemented by reading `argparse.c` and adjusting usage examples.

