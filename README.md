## For Rock-Paper-Scissors:
### For TCP:
- 'r' means Rock, 'p' means Paper, 's' means Scissors.
- server indefinitely waits for 2 clients to connect, before proceeding.
- server waits for a response from each client, before responding.
- Server sends "WIN", "LOSE" or "DRAW" to appropriate client.
- if exactly ONE of the clients gives an invalid response (other than 'r', 'p', or 's'), they will receive prompt "invalid response: LOSE", while the other client will receive "WIN", from the server.
- if both clients give invalid response, both will be prompted "invalid response: LOSE" by the server.
- after this, server will wait for both clients to respond 'y'.
- if both respond 'y', then next round will begin.
- if either of them respond with anything other than 'y', prompt "Okay then, goodbye!" will be sent to both clients and both clients will be disconnected. Server will continue to run.

### For UDP:
- similar to TCP.
- when anything other than 'y' is sent via either client (when prompted whether they choose to play again), both clients will be prompted "Okay then, goodbye!" and they will exit. Meanwhile, the server will continue to run as-is.
---
## For PART B:
### File Structure:
- Both `udp_client_B.c` and `udp_server_B.c` are just for namesake and can act as both client and server.
- In `tcp_scatch.h`, there are 3 macros:
  - **buffsize** - denotes the size of chunks to be transmitted.
  - **timeout** - denotes (in ms) the time after which retransmission occurs.
  - **RETRANSMISSION_CHECK** - if it is equal to 1, then you will be able to see retransmission (ack will be delayed)

### Logic:
- first, each program is asked whether it wants to send or receive a message.
- the sender then gets asked for a message to send.
- message is broken down into chunks of length = buffsize. these chunks are sent along with **seq number** and **acknowledgement number**.
  - **sequence number (seq)** denotes total bytes of message sent.
  - **acknowledgement number (ack)** denotes total bytes received as acknowledgement from receiver.
- upon successfully receiving the message chunk, receiver sends back the message "Ack" along with its own seq and ack. here, ack = seq of received message.
- sender receives "Ack" message and checks its ack number, to figure out which chunk of data has been successfully transmitted.
- using this, after transmitting the entire message once, every _timeout_ milliseconds, sender retransmits chunks which have not been acknowledged yet.
- once all chunks have been acknowledged, sender sends total number of chunks to receiver as a message and breaks.
- receiver, upon receiving this message, breaks, aggregates all the chunks received and prints the final, entire message.
- and thus, transmission is completed.

## How it is different from actual TCP:
my implementation differs from TCP in the following ways:
- no handshake is performed to establish/terminate connection.
- no notion of graceful or forceful termination.
- message, along with sequence and acknowledgement numbers are used. other bits such as flags, window size, checksum etc are not used.
- seq and ack numbers are reset after completion of every message transmission.
- acknowledgement of the Kth chunk will always come only after acknowledgement of the (K-1)th chunk.
## How i would implement flow control:
I would try to implement the **sliding window approach**:
- sender would first send a starting message (like "HI!"). it would wait _timeout_ milliseconds for a response before retransmission.
- in response, receiver would send the sliding window in terms of number of additional messages the sender might send as an additional buffer (let this be 'k').
- the sender, would then only send a maximum of 'k' chunks without acknowledgement by the receiver, before stopping indefinitely until acknowledgement is received. 
- the sender's retransmission protocol would also be stopped until acknowledgement is received.
- in this way, `(number of chunks sent) - (number of ack. chunks received)` would always be at most 'k'.
- thus, flow control would be maintained between sender and receiver.