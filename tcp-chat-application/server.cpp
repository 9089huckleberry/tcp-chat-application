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

vector<int> client_sockets;                   
unordered_map<int, string> client_names; 
mutex clients_mutex;                          

void handle_client(int client_socket)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

   
    int valread = read(client_socket, buffer, 1024);
    if (valread <= 0)
    {
        close(client_socket);
        return;
    }

    string client_name(buffer);
    clients_mutex.lock();
    client_names[client_socket] = client_name; 
    clients_mutex.unlock();

    cout << client_name << " has joined the chat." << endl;

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(client_socket, buffer, 1024);

        if (valread > 0)
        {
            string message = client_name + ": " + buffer;
            cout << message << endl;


            clients_mutex.lock();
            for (int sock : client_sockets)
            {
                if (sock != client_socket)
                { 
                    send(sock, message.c_str(), message.size(), 0);
                }
            }
            clients_mutex.unlock();
        }
        else if (valread == 0)
        {
            cout << client_name << " disconnected." << endl;
            close(client_socket);

            clients_mutex.lock();
            client_sockets.erase(remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
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

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    cout << "Server is waiting for connections..." << endl;

    while (true)
    {
        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        cout << "New client connected!" << endl;

        clients_mutex.lock();
        client_sockets.push_back(new_socket);
        clients_mutex.unlock();

        thread(handle_client, new_socket).detach();
    }

    close(server_fd);
    return 0;
}
