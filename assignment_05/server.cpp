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

#include <sstream>

#define PORT 8080
#define READ_BUFFER_MAX_SIZE 1024

#define MAX_MAP_SIZE 65536

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

    // Record Node
    class Record {
        
        public:

            unsigned int ID;
            string title;
            vector <string> authors;
            string publish_date;
            string publisher;

            Record() {

                ID = 0;
                title = "Dummy Title";
                authors.push_back("Dummy Author 1");
                authors.push_back("Dummy Author 2");
                publish_date = "12/01/2001";
                publisher = "Dummy Publisher";

            }

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



public:

    DBMS() {

        LOG("(-) Database started...");

    }

    // Inserts record
    bool insert_record(const Record& r) {

        // Parsing record
        string file_insertion = parse_record(r);


        // Opening file
        int fd = open("db.dat", O_RDWR | O_CREAT , 0);

        if (fd < 0) {

            LOG("(!) Could not open database file...", true);
            throw runtime_error("(!) Missing database file...");

        }

        // Extending size
        ftruncate(fd, MAX_MAP_SIZE);

        // Memory Map
        char* ptr = (char*) mmap(0, MAX_MAP_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);

        // Writing into the file
        {

            int i = r.ID * sizeof(Record);

            for (char c : file_insertion) {
                ptr[i] = c;
                ++i;
            }

        }

        // Closing
        close(fd);

        // Logging
        LOG("(!) Successfully inserted record...");

        // Validating
        return true;

    }

    // Gets record
    bool get_record(unsigned int ID) {

        // Opening file
        int fd = open("db.dat", O_RDWR | O_CREAT , 0);

        if (fd < 0) {

            LOG("(!) Could not open database file...", true);
            throw runtime_error("(!) Missing database file...");

        }

        // Extending size
        ftruncate(fd, MAX_MAP_SIZE);

        // Memory Map
        char* ptr = (char*) mmap(0, MAX_MAP_SIZE, PROT_READ, MAP_SHARED, fd, 0);

        // Record retrieval
        string retrieved_record = "";

        // Writing into the file
        {

            int i = ID * sizeof(Record);

            for (int j = 0; j < sizeof(Record); j++) {
                retrieved_record += ptr[i];
                i++;
            }

        }

        parse_string(retrieved_record);

        // Closing
        close(fd);

        // Logging
        LOG("(!) Successfully retrieved record...");

        // Validating
        return true;

    }

    // Converts record into a file writable format
    string parse_record(const Record& r) {

        // Function output
        string parsed_output = "";

        // Parsed ID
        parsed_output += to_string(r.ID);
        parsed_output += ",";

        // Parsed Title
        parsed_output += r.title;
        parsed_output += ",";

        // Parse Authors
        parsed_output += "(";
        for (int i = 0; i < r.authors.size() - 1; i++) {
            parsed_output += r.authors[i];
            parsed_output += ",";
        }
        parsed_output += r.authors[r.authors.size() - 1];
        parsed_output += "),";

        // Parsed Publish Date
        parsed_output += r.publish_date;
        parsed_output += ",";

        // Parsed Publisher
        parsed_output += r.publisher;
        parsed_output += '.';

        return parsed_output;

    }

    Record parse_string(const string& s) {

        vector <string> tokens;
        string token;

        // Tokenizing the string
        for (unsigned int i = 0; i < s.length(); i++) {

            if (s[i] == '(') {

                i++;

                while (s[i] != ')') {
                    token += s[i];
                    i++;
                }

            }
            else if (s[i] == ',' || s[i] == '.') {
                tokens.push_back(token);
                token = "";
            }
            else {
                token += s[i];
            }

        }


        // Creating output record

        // Record output;

        // output.ID = stoi(token[0]);
        // output.title = token[1];

        // return output;

    }


    ~DBMS() {
        LOG("(-) Database closed...");
    }

};


int main() {


    DBMS db;

    vector <string> authors = {
        "Saqib Ali",
        "Farhan Ali",
        "Sajawal Ali",
        "Shmoon Ali"
    };

    DBMS::Record r[3];

    r[0].ID = 7;
    r[1].ID = 21;
    r[2].ID = 0;
    
    db.insert_record(r[0]);
    db.insert_record(r[1]);
    db.insert_record(r[2]);


    db.get_record(7);

    return -1;


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