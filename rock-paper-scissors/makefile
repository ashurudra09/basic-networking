# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# List of executables to build
TARGETS = tcp_server tcp_client udp_server udp_client

# Default target: build all executables
all: $(TARGETS)

# Rule to build the TCP server
tcp_server: tcp_server.c
	$(CC) $(CFLAGS) -o tcp_server tcp_server.c

# Rule to build the TCP client
tcp_client: tcp_client.c
	$(CC) $(CFLAGS) -o tcp_client tcp_client.c

# Rule to build the UDP server
udp_server: udp_server.c
	$(CC) $(CFLAGS) -o udp_server udp_server.c

# Rule to build the UDP client
udp_client: udp_client.c
	$(CC) $(CFLAGS) -o udp_client udp_client.c

# Phony target to clean up compiled files
.PHONY: clean
clean:
	rm -f $(TARGETS)