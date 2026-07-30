# TCP vs UDP

TCP and UDP are transport protocols. They sit above IP and decide how application data moves between two programs.

Use **TCP** when correctness and ordering matter more than minimum delay.
Use **UDP** when low latency, simple messages, or custom reliability matter more than built-in guarantees.

## Quick Choice Table

| Need | Prefer | Why |
| --- | --- | --- |
| Web pages, APIs, file transfer, SSH | TCP | Reliable ordered byte stream |
| Live voice/video, games, telemetry, DNS-style queries | UDP | Lower overhead and no waiting for retransmission |
| Every byte must arrive in order | TCP | Retransmits lost data and reorders received segments |
| Late data is useless | UDP | The application can ignore old packets instead of waiting |
| Broadcast or multicast | UDP | TCP is connection-oriented and one-to-one |
| Custom retry, ordering, or congestion behavior | UDP | The application controls the policy |

## What Is A Packet?

A packet is a small unit of data sent across a network.
Large data is split into many packets because networks have maximum packet sizes.

In everyday speech, people often say "packet" for everything.
More precisely:

| Layer | Unit Name | Example |
| --- | --- | --- |
| Link layer | frame | Ethernet or Wi-Fi frame |
| Internet layer | packet or datagram | IP packet |
| Transport layer | segment or datagram | TCP segment, UDP datagram |

When an application sends data, it is wrapped in headers as it moves down the network stack:

```text
Application data
    |
    v
TCP or UDP header + application data
    |
    v
IP header + TCP/UDP header + application data
    |
    v
Ethernet/Wi-Fi header + IP packet
```

## How A Packet Looks

A simplified IP packet carrying TCP data looks like this:

```text
+----------------+----------------+--------------------------+
| IP header      | TCP header     | Application data         |
+----------------+----------------+--------------------------+
| from/to IP     | from/to port   | bytes from the program   |
| protocol = TCP | sequence nums  | HTTP, SSH, file data...  |
| packet length  | acknowledgments|                          |
+----------------+----------------+--------------------------+
```

A simplified IP packet carrying UDP data looks like this:

```text
+----------------+----------------+--------------------------+
| IP header      | UDP header     | Application data         |
+----------------+----------------+--------------------------+
| from/to IP     | from/to port   | one message/datagram     |
| protocol = UDP | length         | DNS, game update, audio  |
| packet length  | checksum       |                          |
+----------------+----------------+--------------------------+
```

The important difference is that TCP has more state in the header and in both endpoints.
It tracks byte order, acknowledgments, retransmissions, flow control, and connection state.
UDP is mostly just ports, length, checksum, and data.

## TCP

TCP means **Transmission Control Protocol**.
It creates a connection between two endpoints and gives the application a reliable ordered stream of bytes.

TCP provides:

- **Connection setup** with a handshake before data starts.
- **Ordering** with sequence numbers.
- **Reliability** with acknowledgments and retransmission.
- **Duplicate detection** so repeated segments do not become repeated application data.
- **Flow control** so a fast sender does not overwhelm a slow receiver.
- **Congestion control** so traffic backs off when the network looks overloaded.

TCP does not preserve message boundaries.
If an app writes 3 chunks, the receiver might read them as 1 chunk, 2 chunks, or many smaller chunks.
The bytes stay ordered, but the application must define its own message framing.

## UDP

UDP means **User Datagram Protocol**.
It sends independent datagrams without creating a connection first.

UDP provides:

- **Ports** so data reaches the correct application.
- **Datagram boundaries** so one send is received as one datagram if it arrives.
- **Checksum protection** against corrupted packets.
- **Low protocol overhead** because there is no built-in connection, retry, ordering, or congestion behavior.

UDP does not guarantee that a datagram arrives.
It also does not guarantee order, uniqueness, or delivery speed.
The application must handle those rules if it needs them.

## Why TCP Is Slower

TCP is usually slower in latency-sensitive cases because it does extra work to protect correctness.

| Cost | Why It Happens |
| --- | --- |
| Handshake | TCP usually needs a connection setup before sending application data |
| Acknowledgments | Receivers report what data arrived |
| Retransmission | Lost data is sent again |
| Ordered delivery | Later bytes may wait behind one missing earlier segment |
| Congestion control | TCP deliberately slows down when the network seems busy |
| Flow control | Sender speed is limited by receiver capacity |
| More state | Both endpoints track connection and sequence information |

The biggest practical issue is often **head-of-line blocking**.
If TCP segment 5 is lost but segments 6, 7, and 8 arrive, TCP cannot deliver 6, 7, and 8 to the application yet.
It waits until segment 5 is retransmitted, because TCP promises ordered bytes.

UDP can let the application receive newer data even if an older datagram disappeared.
For voice, video, or games, that can feel faster because old missing data may no longer be useful.

## What Can Be Lost?

With UDP, these can happen directly:

- A datagram can be lost.
- Datagrams can arrive out of order.
- The same datagram can arrive more than once.
- A datagram can arrive too late to be useful.
- A datagram can be dropped if it is too large or the network is congested.

With TCP, packets can still be lost on the network, but TCP hides that from the application when possible.
TCP retransmits missing data and delivers bytes in order.

However, TCP can still lose useful things:

- **Time**: waiting for retransmission adds latency.
- **Throughput**: congestion control may reduce sending speed.
- **Connection state**: if a connection breaks, unsent or unacknowledged data may not reach the peer.
- **Message boundaries**: TCP is a byte stream, so the original application writes are not preserved.
- **Freshness**: data can arrive correctly but too late for real-time use.

TCP should not silently lose acknowledged bytes during a healthy connection.
If it cannot continue, the application usually sees an error, timeout, or closed connection.

## Small Example

Imagine sending positions in an online game:

```text
Player at x=10
Player at x=11
Player at x=12
Player at x=13
```

With TCP, if `x=11` is lost, `x=12` and `x=13` wait until `x=11` is resent.
That preserves order, but the game may feel delayed.

With UDP, `x=11` may disappear, but `x=12` and `x=13` can still be processed.
That loses one old update, but the game can keep showing the newest state.

## Rule Of Thumb

Choose TCP when missing or reordered bytes would be a bug.
Choose UDP when waiting for old data would be worse than losing it.
