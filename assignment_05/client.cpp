#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <iostream>

#define PORT 8080
#define READ_BUFFER_MAX_SIZE 1024

using namespace std;

int main () {

    cout << "(+) Starting client..." << endl;

    int socket_fd;
    struct sockaddr_in server_address;
    
    char* message = "Hello from client!";
    
    char read_buffer[READ_BUFFER_MAX_SIZE] = {0};

    // Creating a socket

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (!socket_fd) {
        perror("(*) Could not create socket...");
        exit(EXIT_FAILURE);
    }

    cout << "(+) Socket created..." << endl;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Converting IPv4 and IPv6 addresses from text to binary.
    if ( inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0 ) {

        perror ("(*) Invalid address...");
        exit(EXIT_FAILURE);

    }

    cout << "(+) Address converted..." << endl;

    // Connecting to server
    if (connect(socket_fd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {

        perror("(*) Could not connect...");
        exit(EXIT_FAILURE);

    }

    cout << "(+) Server connected..." << endl;


    // Chat Interface

    while (1) {

        // Taking user input
        string user_input;
        cout << ">> ";
        cin >> user_input;

        // Sending message
        send(socket_fd, user_input.c_str(), user_input.length(), 0);

        // Reading any messages
        int bytes_read = read(socket_fd, read_buffer, READ_BUFFER_MAX_SIZE);
        cout << "Server: " << read_buffer << endl;

    }

    return 0;

}