# TCP User Texting Application with Pthreads

![GitHub License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)

Welcome to the TCP User Texting Application! This C-based application enables users to send and receive text messages over a server using the TCP protocol. It encompasses a basic client-server architecture, empowering users to interact in real-time. Additionally, users can log in, send messages to users, list all online users, engage in private one-on-one chats, switch between chats, and all of this is managed using pthreads for concurrent communication.

## Features

- **Client-Server Architecture**: The application follows a client-server model, allowing multiple users to connect to a central server and exchange messages.

- **Login and Logout**: Users can log in using their unique usernames. Logging out disconnects the user from the server.

- **Text Messaging**: Logged-in users can send and receive text messages through the server. Messages are relayed in real-time, facilitating interactive communication.

- **Online User List**: The server maintains a list of online users. Clients can query the server to retrieve the list of currently active users.

- **One-on-One Chats**: Users can initiate private conversations with online users. These chats are separate from the main chat and provide a more personalized interaction.

- **Switch Chats**: Users can switch between one-on-one chats, disconnecting from the previous one and creating a new one.

- **List All Users**: Logged-in users can request a list of all online users from the server. This command displays the usernames of active users.

- **Concurrent Communication**: The application utilizes pthreads for concurrent communication. Each one-on-one chat is managed by a separate thread, allowing multiple users to engage in private chats simultaneously.

- **TCP Protocol**: The application utilizes the TCP protocol for reliable and ordered communication. TCP ensures that messages are received in the same order they were sent.

## How to Use

1. **Compile**: Compile the server and client using makefiles.
   ```bash
   makefile server
   makefile client
   ```

2. **Run**: Run the server first and then the clients
   ```bash
   ./server
   ./client <port>
   ```

3. **Login**: Login with the unique username
   ```bash
   ./login <username>
   ```

4. **Choose the action**: After successful login choose your next action
   ```bash
   ./talkto <user> (creates chat request, with other user has to accept for chat to initiate)
   ./users (list all online users)
   ./logout (logs out the user and brings back login screen)
   ```

5. **Chat**: Talk to the selected user in one-on-one chat 
   ```bash
   disconnect (typing this the chat will close and you're back in menu above)
   ```
