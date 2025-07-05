# C Networking Mini-Projects

This repository contains two mini-projects demonstrating fundamental networking concepts in C:
1.  **Rock-Paper-Scissors Game**: A client-server application implemented using both TCP and UDP.
2.  **TCP over UDP**: A custom protocol that implements TCP-like reliability (acknowledgements, retransmission) on top of UDP.

---

## Project 1: Rock-Paper-Scissors

This project is a classic 2-player Rock-Paper-Scissors game. The server waits for two clients to connect and then facilitates game rounds until one or both players decide to quit.

### Implementation Details

#### TCP Version (`rock-paper-scissors/tcp_*.c`)

-   **Connection-Oriented**: Uses TCP sockets, ensuring reliable, in-order delivery of messages.
-   **Server Logic**:
    -   The server starts and listens for connections on a fixed port (`5566`).
    -   It uses `poll()` to accept new connections without blocking the main loop, allowing it to handle game logic and new clients efficiently.
    -   It waits until exactly two clients are connected before starting the game.
    -   For each round, it receives one move from each client, determines the winner, and sends the result ("WIN", "LOSE", "DRAW") back to the appropriate client.
    -   It handles invalid inputs from players.
    -   After a round, it asks both players if they want to play again. If both respond with `y`, a new round begins. Otherwise, both clients are disconnected, and the server waits for a new pair of players.
-   **Client Logic**:
    -   The client connects to the server's IP and port.
    -   It enters a loop, prompting the user for input (`r`, `p`, `s`, or `y`) and sending it to the server.
    -   It waits for the server's response and displays it.

#### UDP Version (`rock-paper-scissors/udp_*.c`)

-   **Connectionless**: Uses UDP sockets, which do not guarantee message delivery or order.
-   **Server Logic**:
    -   The server binds to a port specified by a command-line argument.
    -   It does not maintain a persistent connection. Instead, it waits for two incoming packets using blocking `recvfrom` calls. It identifies clients by their `sockaddr_in` structure.
    -   A simple state flag is used to differentiate between an initial "Hello!" packet and a game move packet.
    -   The game logic is similar to the TCP version: it compares moves, determines a result, and sends it back to the clients' respective addresses.
-   **Client Logic**:
    -   The client takes the server port as a command-line argument.
    -   It first sends a "Hello!" packet to announce its presence.
    -   It then enters a loop of receiving a prompt, sending a move, and receiving a result.

---

## Project 2: Implementing TCP over UDP

This project demonstrates how to build a reliable data transfer protocol on top of the unreliable UDP. It implements key features of TCP, such as sequence numbers, acknowledgements, and timeouts for retransmission. The implementation is peer-to-peer, meaning the same program can act as a sender or a receiver.

### Implementation Details

#### Key Files
-   `tcp-from-udp/tcp_scratch.h`: A header file defining the packet structure and protocol constants.
-   `tcp-from-udp/udp_client_B.c` & `udp_server_B.c`: Identical source files for the peer program.

#### Core Concepts

1.  **Custom Packet Structure**: A `Data` struct is defined in `tcp_scratch.h` to represent a packet segment, containing:
    -   `msg`: A small chunk of the message data (`buffsize`).
    -   `seq`: A sequence number, representing the total bytes sent so far.
    -   `ack`: An acknowledgment number, confirming receipt of data from the other peer.

2.  **Sender Logic**:
    -   The user provides a long message to send.
    -   The message is broken down into smaller chunks of size `buffsize`.
    -   Each chunk is put into a packet with a calculated `seq` number and sent to the receiver.
    -   The sender then uses `poll()` with a short timeout to listen for an acknowledgment (ACK) from the receiver.
    -   **Retransmission**: If an ACK for a packet is not received within the `timeout` period, the sender re-transmits the unacknowledged packet. It keeps track of the oldest unacknowledged packet and resends it until it is acknowledged.

3.  **Receiver Logic**:
    -   The receiver listens for incoming packets.
    -   When a packet arrives, it sends back an ACK packet. The `ack` number in this response is set to the `seq` number of the packet it just received, confirming receipt.
    -   It stores the received data chunks in an array.
    -   A basic duplicate detection is in place to ignore re-transmitted packets that have already been received.

4.  **Termination**:
    -   Once the sender has transmitted all chunks and received ACKs for all of them, it sends a special termination packet (`N=<total_chunks>`).
    -   When the receiver gets this packet, it knows the transmission is complete. It then reassembles the message from the stored chunks and prints it.

#### How it Differs from Real TCP

-   **No Handshake**: There is no three-way handshake to establish or terminate a connection.
-   **Simplified Header**: The packet only contains sequence/acknowledgment numbers. It omits other TCP header fields like window size, checksum, and flags (SYN, FIN, RST).
-   **Stop-and-Wait Variant**: The retransmission logic is a simple variant of stop-and-wait, resending the oldest unacknowledged packet on timeout. It does not implement a sliding window for flow control.
-   **Stateless Sequence Numbers**: The sequence numbers are reset for each new message transmission.

---

## How to Compile and Run

`Makefile`s are provided in each project directory for easy compilation and cleanup.

### Project 1: Rock-Paper-Scissors

1.  **Navigate and Compile**:
    ```bash
    cd rock-paper-scissors
    make
    ```
    This will create four executables: `tcp_server`, `tcp_client`, `udp_server`, and `udp_client`.

2.  **Run the TCP Version**:
    -   In one terminal, start the server: `./tcp_server`
    -   In a second terminal, start the first client: `./tcp_client`
    -   In a third terminal, start the second client: `./tcp_client`
    -   Follow the prompts in the client terminals to play.

3.  **Run the UDP Version**:
    -   In one terminal, start the server with a port number (e.g., 8080): `./udp_server 8080`
    -   In a second terminal, start the first client with the same port: `./udp_client 8080`
    -   In a third terminal, start the second client with the same port: `./udp_client 8080`

4.  **Clean Up**:
    To remove the compiled executables, run:
    ```bash
    make clean
    ```
    Then, navigate back to the root directory: `cd ..`

### Project 2: TCP over UDP

1.  **Navigate and Compile**:
    ```bash
    cd tcp-from-udp
    make
    ```
    This will create two executables: `peer_A` and `peer_B`.

2.  **Run the Peers**:
    -   In one terminal, start the first peer (which will be the receiver) with a port number: `./peer_A 8080`
    -   In a second terminal, start the second peer (which will be the sender) with the same port: `./peer_B 8080`

3.  **Interact**:
    -   In the receiver's terminal (`peer_A`), when prompted, enter `0` to receive.
    -   In the sender's terminal (`peer_B`), when prompted, enter `1` to send.
    -   The sender will then be prompted to enter a message. Type a message (without spaces) and press Enter.
    -   Observe the logs in both terminals to see the chunking, sending, ACKs, and retransmissions. The final reassembled message will appear on the receiver's side.

4.  **Clean Up**:
    To remove the compiled executables, run:
    ```bash
    make clean
    ```
    Then, navigate back to the root directory: `cd ..`