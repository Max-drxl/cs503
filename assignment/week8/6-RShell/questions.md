1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

> **Answer**: We use a `recv()` loop to ensure a commands' output is fully received, but that alone is not enough. We also combine this with a check for a given final character using the `is_last_chunk()` function that looks for either the `/0` character or the `0x04` ASCII character depending on whether we are receiving from the client or the server to know that our output is fully sent. This ensures we do not have partial reads as we will continue to receive more bytes until we find our last character determined by our contract between the client and server.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

> **Answer**: A networked shell protocol could detect start of message by sitting in a `recv()` loop as we implemented, but that may not always be the best solution. Generally a contract should be arranged between all connections to define at least an end of transmission character that can be searched for. A more robust solution would be to send not just the message itself but start with the length of the message, then the message, then the end of message character. Then the receiver could measure their received message against the expected length. Without these checks it would be impossible to properly parse the messages back and forth as there would be consistent data loss and potentially data commingling too.

3. Describe the general differences between stateful and stateless protocols.

> **Answer**: Stateful protocols have more robust capabilities such as reliability which is critical for applications but will generally be slower with more overhead. Lower-level network protocols (with the exception of TCP) tend to be stateless which are generally faster and have less overhead but are not fully reliable, like a web browser.

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

> **Answer**: UDP is a Datagram protocol which is a Message-Oriented protocol typically used in load balancers, network hardware, messaging applications, and also DNS resolution. Messages can get lost and you would never know but certain applications do not need 100% of each message to be sent and received particularly when low latency is a higher priority.

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

> **Answer**: Sockets are the programming interface provided by the operating system to abstract and enabling applications to write network programs. They provide a "file" abstraction over a network connection (using file descriptors).