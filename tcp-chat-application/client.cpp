#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

#define PORT 8082

void handle_receive(int socket)
{
    char buffer[1024] = {0};
    while (true)
    {
        int valread = read(socket, buffer, 1024);
        if (valread > 0)
        {
            std::cout << buffer << std::endl;
            memset(buffer, 0, sizeof(buffer)); // Clear buffer after reading
        }
        else if (valread == 0)
        {
            std::cout << "Server disconnected" << std::endl;
            break;
        }
    }
}

void handle_send(int socket)
{
    char buffer[1024];
    while (true)
    {
        std::cin.getline(buffer, 1024);
        send(socket, buffer, strlen(buffer), 0);
        if (strcmp(buffer, "exit") == 0)
        {
            std::cout << "Closing connection..." << std::endl;
            break;
        }
    }
}

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    std::cout << "Enter your name: ";
    std::string name;
    std::getline(std::cin, name);

    // Send the client name to the server
    send(sock, name.c_str(), name.length(), 0);

    std::cout << "Connected to the server as " << name << "!" << std::endl;

    // Create threads for sending and receiving
    std::thread receive_thread(handle_receive, sock);
    std::thread send_thread(handle_send, sock);

    // Join threads
    receive_thread.join();
    send_thread.join();

    close(sock);
    return 0;
}
