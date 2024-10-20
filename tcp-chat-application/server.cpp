#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <mutex>
#include <unordered_map>

#define PORT 8082

std::vector<int> client_sockets;                   // Store all connected clients
std::unordered_map<int, std::string> client_names; // Map client socket to client name
std::mutex clients_mutex;                          // Mutex for thread safety

// Function to handle communication with each client
void handle_client(int client_socket)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    // Read the client's name initially
    int valread = read(client_socket, buffer, 1024);
    if (valread <= 0)
    {
        close(client_socket);
        return;
    }

    std::string client_name(buffer);
    clients_mutex.lock();
    client_names[client_socket] = client_name; // Store the client name
    clients_mutex.unlock();

    std::cout << client_name << " has joined the chat." << std::endl;

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(client_socket, buffer, 1024);

        if (valread > 0)
        {
            std::string message = client_name + ": " + buffer;
            std::cout << message << std::endl;

            // Broadcast the message to all other clients
            clients_mutex.lock();
            for (int sock : client_sockets)
            {
                if (sock != client_socket)
                { // Do not send back to the same client
                    send(sock, message.c_str(), message.size(), 0);
                }
            }
            clients_mutex.unlock();
        }
        else if (valread == 0)
        {
            std::cout << client_name << " disconnected." << std::endl;
            close(client_socket);

            // Remove the client socket and name from the lists
            clients_mutex.lock();
            client_sockets.erase(std::remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
            client_names.erase(client_socket);
            clients_mutex.unlock();
            break;
        }
    }
}

int main()
{
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is waiting for connections..." << std::endl;

    while (true)
    {
        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        std::cout << "New client connected!" << std::endl;

        // Add the new socket to the list of clients
        clients_mutex.lock();
        client_sockets.push_back(new_socket);
        clients_mutex.unlock();

        // Create a new thread for the new client
        std::thread(handle_client, new_socket).detach();
    }

    close(server_fd);
    return 0;
}
