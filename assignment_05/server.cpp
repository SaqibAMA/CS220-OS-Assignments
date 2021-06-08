#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <stdlib.h>

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

// DBMS
class DBMS {

public:
    DBMS() {

        write_dummy_data();
    
    }

    void write_dummy_data() {

        vector <string> dummyAuthors = {
            "Jacob Simons",
            "Alan Turing",
            "John Doe",
            "Theta Omega"
        };

        Record _dRec (
            1,
            "Introduction to Programming",
            dummyAuthors,
            "12/01/2001",
            "Penguin"
        );

        Record _dRec2 (
            7,
            "Introduction to Programming",
            dummyAuthors,
            "12/01/2001",
            "Penguin"
        );

        int fd = open("data.dat", O_RDWR | O_APPEND, 0);

        lseek(fd, _dRec2.ID * sizeof(Record), SEEK_SET);

        string output = _dRec2.title;

        write(fd, output.c_str(), sizeof(Record));

        close(fd);

        cout << "File written" << endl;

    }

public:

    // Record Node
    struct Record {
        
        unsigned int ID;
        string title;
        vector <string> authors;
        string publish_date;
        string publisher;

        Record(
            unsigned int ID,
            string title,
            vector <string> authors,
            string publish_date,
            string publisher) {

                this->ID = ID;
                this->title = title;
                this->authors = authors;
                this->publish_date = publish_date;
                this->publisher = publisher;

        }

    };

    vector <Record> index;

};



int main() {

    LOG("(+) Starting server...");

    int server_fd, accept_socket;
    struct sockaddr_in address;
    int opt = 1;
    int address_len = sizeof(address);

    char read_buffer[READ_BUFFER_MAX_SIZE] = {0};

    // Creating a socket

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (!server_fd) {
        perror("(*) Could not create socket...");
        exit(EXIT_FAILURE);
    }


    LOG("(+) Socket created...");

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

    LOG("(+) Socket attached with port...");

    // Listening to the client
    if (listen(server_fd, 3) < 0) {

        perror("(*) Something went wrong in LISTEN()...");
        exit(EXIT_FAILURE);

    }

    LOG("(+) Listening...");

    // Starting to accept the client input
    
    accept_socket = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &address_len);

    if (accept_socket < 0) {
        
        perror("(*) Something went wrong in ACCEPT()...");
        exit(EXIT_FAILURE);

    }

    LOG("(+) Accepted...");


    DBMS db;

    while (1) {
    
        // Reading the values that have been sent by the client
        int bytes_read = read(accept_socket, read_buffer, READ_BUFFER_MAX_SIZE);
        cout << "Client: " << read_buffer << endl;

        string server_input = "Request received...";

        // Sending a message to client
        send(accept_socket, server_input.c_str(), server_input.length(), 0);
    
    }


    return 0;
}