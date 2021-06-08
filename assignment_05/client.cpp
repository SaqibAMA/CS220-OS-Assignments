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


// Simple Logging Routine

#define GREEN "\u001b[32m"
#define RED "\u001b[31m"
#define RESET "\u001b[0m"

void LOG(const char* message, bool error_flag = false) {

    if (error_flag)
        cout << RED;
    else
        cout << GREEN;

    cout << message;

    cout << RESET << endl;

}

int main () {

    LOG("(+) Starting client...");

    int socket_fd;
    struct sockaddr_in server_address;
    
    char read_buffer[READ_BUFFER_MAX_SIZE] = {0};

    // Creating a socket

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (!socket_fd) {
        perror("(*) Could not create socket...");
        exit(EXIT_FAILURE);
    }

    LOG("(+) Socket created...");

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Converting IPv4 and IPv6 addresses from text to binary.
    if ( inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0 ) {

        perror ("(*) Invalid address...");
        exit(EXIT_FAILURE);

    }

    LOG("(+) Address converted...");

    // Connecting to server
    if (connect(socket_fd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {

        perror("(*) Could not connect...");
        exit(EXIT_FAILURE);

    }

    LOG("(+) Server connected...");


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
        
        cout << "[RESPONSE]" << read_buffer << endl;

    }

    return 0;

}