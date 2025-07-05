# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# List of executables to build
TARGETS = peer_A peer_B

# Default target: build all executables
all: $(TARGETS)

# Rule to build peer_A from udp_client_B.c
# It depends on the source file and the header file
peer_A: udp_client_B.c tcp_scratch.h
	$(CC) $(CFLAGS) -o peer_A udp_client_B.c

# Rule to build peer_B from udp_server_B.c
# It also depends on the source file and the header file
peer_B: udp_server_B.c tcp_scratch.h
	$(CC) $(CFLAGS) -o peer_B udp_server_B.c

# Phony target to clean up compiled files
.PHONY: clean
clean:
	rm -f $(TARGETS)