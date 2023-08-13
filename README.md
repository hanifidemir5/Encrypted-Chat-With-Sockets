# Communication Using Socket Programming With AES Encryption

![GitHub last commit](https://img.shields.io/github/last-commit/hanifidemir5/CommunicationUsingSocketProgrammingWithAES)
![GitHub](https://img.shields.io/github/license/hanifidemir5/CommunicationUsingSocketProgrammingWithAES)

This is a communication application implemented in C language using socket programming. The primary goal of this project is to provide secure communication between clients by utilizing AES encryption in counter mode (CTR) for message encryption and decryption. The server acts as a mediator, forwarding messages to the appropriate recipients while maintaining the confidentiality of the messages.

## Features

- Secure communication between clients using AES encryption in counter mode.
- Server-mediated communication for message distribution.
- Support for broadcasting messages to all connected clients.
- Option to send private messages to specific clients.

## Getting Started

To use this application, follow these steps:

1. Clone the repository:

   ```bash
   git clone https://github.com/hanifidemir5/CommunicationUsingSocketProgrammingWithAES.git
   cd CommunicationUsingSocketProgrammingWithAES
   ```

2. Navigate to the server directory:

   ```bash
   cd Communication/Server
   ```

3. Compile the server program:

   ```bash
   gcc -o server.exe server.c aes.c -lws2_32
   ```

4. Run the server:

   ```bash
   ./server.exe
   ```

5. When prompted, enter the server key. This key will be required for clients to connect.

6. Navigate to the client directory:

   ```bash
   cd ../Client
   ```

7. Compile the client program:

   ```bash
   gcc -o client.exe client.c aes.c -lws2_32
   ```

8. Run the client program:

   ```bash
   ./client.exe
   ```

9. When prompted, enter the same server key you used for the server.

10. Follow the on-screen instructions to interact with the application.

## Usage

1. When a client connects to the server, it must enter predetermined server key.
2. If key is correct, server will connect requesting client to network.
3. Connected client should provide a username.
4. Clients can broadcast messages that will be sent to all connected clients.
5. To send a private message, sender must provide '@' with the recipient's username along with the message. For example '@user message'.
6. The server will handle encryption and decryption of messages using AES in counter mode (CTR).
7. The server forwards the encrypted messages to the intended recipients while preserving message confidentiality.

## License

This project is licensed under the [MIT License](LICENSE).
