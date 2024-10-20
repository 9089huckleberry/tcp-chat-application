# TCP MULTI-CLIENT CHAT Application
This repository contains a TCP-based chat application written in C++. The application supports multiple clients connecting to a single server and communicating with each other in real-time. The server handles incoming client connections and broadcasts messages to all clients except the sender. The clients can send messages and receive messages from the server simultaneously using multithreading.

# Features
- Multi-client support: The server can manage multiple clients simultaneously.
- Real-time messaging: Clients can chat with each other in real-time.
- Threaded implementation: Both server and client use threads to handle multiple connections and simultaneous sending/receiving.
- Named clients: Each client is assigned a name for identification in the chat.

# Clone this repository
git clone https://github.com/9089huckleberry/tcp-chat-application.git