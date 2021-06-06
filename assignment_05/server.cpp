#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>

#define PORT 8080
#define READ_BUFFER_MAX_SIZE 1024

using namespace std;

int main() {


    cout << "(+) Starting server..." << endl;

    int server_fd, accept_socket;
    struct sockaddr_in address;
    int opt = 1;
    int address_len = sizeof(address);

    char read_buffer[READ_BUFFER_MAX_SIZE] = {0};

    char* message = "Hello, this is coming from the server...";


    // Creating a socket

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (!server_fd) {
        perror("(*) Could not create socket...");
        exit(EXIT_FAILURE);
    }


    cout << "(+) Socket created..." << endl;

    // Attaching socket with PORT

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {

        perror("(*) Something went wrong with SETSOCKETOPT()...");
        exit(EXIT_FAILURE);

    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind (server_fd, (struct sockaddr*) &address, sizeof(address)) < 0) {
        
        perror("(*) Something went wrong while binding the socket...");
        exit(EXIT_FAILURE);

    }

    cout << "(+) Socket attached with port..." << endl;

    // Listening to the client
    if (listen(server_fd, 3) < 0) {

        perror("(*) Something went wrong in LISTEN()...");
        exit(EXIT_FAILURE);

    }

    cout << "(+) Listening..." << endl;

    // Starting to accept the client input
    
    accept_socket = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &address_len);

    if (accept_socket < 0) {
        
        perror("(*) Something went wrong in ACCEPT()...");
        exit(EXIT_FAILURE);

    }

    cout << "(+) Accepted..." << endl;

    while (1) {
    
        string server_input;
        cout << ">> ";
        cin >> server_input;

        // Reading the values that have been sent by the client
        int bytes_read = read(accept_socket, read_buffer, READ_BUFFER_MAX_SIZE);
        cout << "Client: " << read_buffer << endl;

        // Sending a message to client
        send(accept_socket, server_input.c_str(), server_input.length(), 0);
    
    }

    return 0;
}