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
            cout << buffer << endl;
            memset(buffer, 0, sizeof(buffer)); 
        }
        else if (valread == 0)
        {
            cout << "Server disconnected" << endl;
            break;
        }
    }
}

void handle_send(int socket)
{
    char buffer[1024];
    while (true)
    {
        cin.getline(buffer, 1024);
        send(socket, buffer, strlen(buffer), 0);
        if (strcmp(buffer, "exit") == 0)
        {
            cout << "Closing connection..." << endl;
            break;
        }
    }
}

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cerr << "Socket creation error" << endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);


    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        cerr << "Invalid address/ Address not supported" << endl;
        return -1;
    }

    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        cerr << "Connection Failed" << endl;
        return -1;
    }

    cout << "Enter your name: ";
    string name;
    getline(cin, name);

    
    send(sock, name.c_str(), name.length(), 0);

    cout << "Connected to the server as " << name << "!" << endl;

    
    thread receive_thread(handle_receive, sock);
    thread send_thread(handle_send, sock);

    // Join threads
    receive_thread.join();
    send_thread.join();

    close(sock);
    return 0;
}
