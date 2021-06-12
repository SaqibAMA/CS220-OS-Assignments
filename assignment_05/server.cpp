#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include <stdlib.h>

#include <sstream>

#define PORT 8080
#define READ_BUFFER_MAX_SIZE 1024

#define MAX_MAP_SIZE 65536

using namespace std;



#define MAX_IMBALANCE 1

// - AVL Tree Class -

template <typename T>
class AVL {

	// AVL Node Class

	struct ANode {

		T data;
		int height;
		ANode* right;
		ANode* left;

		ANode(T data = NULL) {
			right = left = nullptr;
			height = 0;
			this->data = data;
		}

	} *root;

public:

	ANode* getRoot() const {
		return root;
	}

	// Constructor

	AVL() {
		root = nullptr;
	}

	// Copy Constructor

	AVL(const AVL& A) {

		if (A.root == nullptr) {

			this->root = nullptr;

		} else {
			
			copyTree(A.root);

		}
	
	}

	// Tree Copying Function

	void copyTree(ANode* curr) {

		if (curr == nullptr) {
			return;
		}
		else {
			this->insertNode(curr->data);
			copyTree(curr->right);
			copyTree(curr->left);
		}

	}

	// Insertion
	bool insertNode(T data) {

		if (root == nullptr) {
			root = new ANode(data);
			return true;
		}

		ANode* prev = nullptr;
		ANode* curr = root;

		return insertNode(prev, curr, data);

	}

	// Recursive Sub-Function
	bool insertNode(ANode* prev, ANode* curr, T data) {

		if (curr == nullptr) {

			curr = new ANode(data);

			if (prev) {

				if (curr->data > prev->data) {

					prev->right = curr;

				}
				else if (curr->data < prev->data) {

					prev->left = curr;

				}

			}

			return true;

		}
		else {


			if (data > curr->data) {
				if (insertNode(curr, curr->right, data)) {
					curr->height = heightofBSTRec(curr);
				}
			}
			else if (data < curr->data) {
				if (insertNode(curr, curr->left, data)) {
					curr->height = heightofBSTRec(curr);
				}
			}
			else {
				return false;
			}

			if (!nodeIsBalanced(curr)) {
				
				if (data > curr->data) {

					if (data > curr->right->data) {
						RRRotation(curr, prev);
					}
					else {
						RLRotation(curr, prev);
					}

				}
				else {

					if (data < curr->left->data) {
						LLRotation(curr, prev);
					}
					else {
						LRRotation(curr, prev);
					}

				}

			}
			else {
				return true;
			}


		}

		return false;

	}


	// Checking Imbalance
	bool nodeIsBalanced(ANode* node) const {

		int rightNodeHeight = 0, leftNodeHeight = 0;

		if (node->right) {
			node->right->height = heightofBSTRec(node->right);
			rightNodeHeight = node->right->height;
		}

		if (node->left) {
			node->left->height = heightofBSTRec(node->left);
			leftNodeHeight = node->left->height;
		}

		return !(abs(rightNodeHeight - leftNodeHeight) > MAX_IMBALANCE);

	}

	// Getting Balance Factor
	int getBalanceFactor(ANode* left, ANode* right) const {

		int leftHeight = 0, rightHeight = 0;

		if (left) {
			leftHeight = left->height;
		}

		if (right) {
			rightHeight = right->height;
		}

		return (leftHeight - rightHeight);

	}

	// LL Rotation

	void LLRotation(ANode*& curr, ANode*& prev) {

		ANode* prevBData = curr->left->right;

		if (curr == root) {
			root = curr->left;
			curr->left->right = curr;
			curr->left = prevBData;
		}
		else {

			if (prev) {

				if (prev->right == curr) {
					prev->right = curr->left;
				}
				else {
					prev->left = curr->left;
				}

			}

			curr->left->right = curr;
			curr->left = prevBData;

		}

	}

	// RR Rotation

	void RRRotation(ANode*& curr, ANode*& prev) {

		ANode* prevBData = curr->right->left;

		if (curr == root) {
			root = curr->right;
			curr->right->left = curr;
			curr->right = prevBData;
		}
		else {

			if (prev) {

				if (prev->right == curr) {

					prev->right = curr->right;
				
				}
				else {

					prev->left = curr->right;
				
				}

			}

			curr->right->left = curr;
			curr->right = prevBData;

		}

	}

	// LR Rotation

	void LRRotation(ANode*& curr, ANode*& prev) {

		ANode* CLeftTree = curr->left->right->left;

		curr->left->right->left = curr->left;
		curr->left = curr->left->right;
		curr->left->left->right = CLeftTree;

		LLRotation(curr, prev);

	}

	// RL Rotation

	void RLRotation(ANode*& curr, ANode*& prev) {

		
		ANode* CRightTree = curr->right->left->right;

		curr->right->left->right = curr->right;
		curr->right = curr->right->left;
		curr->right->right->left = CRightTree;


		RRRotation(curr, prev);

	}


	// Tree Height Calculation.
	int heightofBSTRec(ANode* start) const {

		if (root == nullptr) return -1;

		return (1 + max(heightofBSTRecSub(start->left), heightofBSTRecSub(start->right)));

	}

	// Tree Height Sub-Function.
	int heightofBSTRecSub(ANode* curr) const {

		if (curr == nullptr) return 0;

		return (1 + max(heightofBSTRecSub(curr->left),
			heightofBSTRecSub(curr->right)));


	}

	// Simple Inorder Traversal.
	void print() {

		ANode* curr = root;
		print(curr);

	}

	// Sub-function for inorder traversal.
	void print(ANode* curr) const {

		if (curr == nullptr) {
			return;
		}
		else {

			print(curr->left);
			cout << curr->data << endl << endl;
			print(curr->right);
		}


	}


	// Element Exists
	T* search(T key) {

		return search(root, key);

	}

	// Sub-function
	T* search(ANode* curr, T key) {

		if (curr == nullptr) return nullptr;

		if (key == curr->data) {
			return &curr->data;
		}
		else if (key < curr->data) {
            cout << "Going to less" << endl;
			return search(curr->left, key);
		}
		else if (key > curr->data) {
            cout << "Going to more" << endl;
			return search(curr->right, key);
		}

		return nullptr;


	}

	// Destructor
	~AVL() {

		// cleanTree(root);

		if (root) {
			delete root;
			root = nullptr;
		}

	}

	// Helper function for destructor
	void cleanTree(ANode* curr) {

		if (curr == nullptr) return;

		cleanTree(curr->left);
		cleanTree(curr->right);

		delete curr;

	}



};



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

void clean_buffer(char* read_buffer) {
    for (int i = 0; i < READ_BUFFER_MAX_SIZE; i++)
        read_buffer[i] = 0;
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

            friend ostream& operator << (ostream& out, const Record& r) {
                out << "[Record Details]\n\n";
                out << "ID:        " << r.ID << endl;
                out << "Title:     " << r.title << endl;
                out << "Authors:   ";
                for (auto author : r.authors)
                    out << "(" << author << ") ";
                out << endl;
                out << "Published: " << r.publish_date << endl;
                out << "Publisher: " << r.publisher << endl;

                return out;
            }

            bool operator < (const Record& r) {
                return (this->title < r.title);
            }

            bool operator > (const Record& r) {
                return (!(*this < r));
            }

            bool operator == (const Record& r) {
                return (r.title == this->title);
            }

            bool operator != (const Record& r) {
                return (!(*this == r));
            }

    };

    
    AVL <Record>* index;

public:

    DBMS() {

        LOG("(-) Database started...");
        index = nullptr;

    }


    // Build and update the index for searching.
    void build_index() {

        if (index != nullptr)
            free(index);

        index = new AVL<Record>;

        LOG("(!) Building index. This might take some time...");

        for (unsigned int i = 0; i < MAX_MAP_SIZE / sizeof(Record); i++) {

            Record r = get_record(i);
            if (r.ID != -1) {
                index->insertNode(r);
            }

        }

        LOG("(!) Index built...");

    }


    void run_query(const string& query, string& query_output) {

        /*
        
        DOCUMENTATION

        select *
        select id x
        select name x
        update id x
        update name x
        delete id x
        delete name x

        */

        struct Query {
            string operation;
            string attribute;
            string value;
        } parsed_query;

        unsigned int queryIter = 0;

        // Extracting operation
        while (query[queryIter] != ' ') {
            parsed_query.operation += query[queryIter];
            queryIter++;
        }

        // Extracting attribute
        queryIter++;            // accomodate the space

        while (query[queryIter] != ' ') {
            parsed_query.attribute += query[queryIter];
            queryIter++;
        }

        queryIter++;

        // Extracting value
        if (parsed_query.attribute != "*") {

            while (query[queryIter] != 0) {
                parsed_query.value += query[queryIter];
                queryIter++;
            }

        }





        // Running Queries
        if (parsed_query.operation == "select") {

            if (parsed_query.attribute[0] == '*') {

                int output[2];
                pipe(output);

                char console_buffer[READ_BUFFER_MAX_SIZE] = {0};

                if (fork() == 0) {

                    dup2(output[1], 1);
                    index->print();

                    exit(0);

                }

                wait(NULL);

                close(output[1]);
                read(output[0], console_buffer, READ_BUFFER_MAX_SIZE);
                close(output[0]);

                query_output = console_buffer;

                return;

            }
            else if (parsed_query.attribute == "id") {

                
                Record search_result = get_record(stoi(parsed_query.value));
                if (search_result.ID == -1) {

                    query_output = "(!) Record not found";

                }
                else {

                    int output[2];
                    pipe(output);

                    char console_buffer[READ_BUFFER_MAX_SIZE] = {0};

                    if (fork() == 0) {

                        dup2(output[1], 1);
                        cout << search_result << endl;

                        exit(0);

                    }

                    wait(NULL);

                    close(output[1]);
                    read(output[0], console_buffer, READ_BUFFER_MAX_SIZE);
                    close(output[0]);

                    query_output = console_buffer;

                }

                return;
                

            }
            else if (parsed_query.attribute == "name") {


                Record search_record;
                search_record.title = parsed_query.value;

                Record* search_result = index->search( search_record );

                cout << "Result: " << search_result << endl;

                int output[2];
                pipe(output);

                char console_buffer[READ_BUFFER_MAX_SIZE] = {0};

                if (fork() == 0) {

                    dup2(output[1], 1);
                    cout << *search_result << endl;

                    exit(0);

                }

                wait(NULL);

                close(output[1]);
                read(output[0], console_buffer, READ_BUFFER_MAX_SIZE);
                close(output[0]);

                query_output = console_buffer;


                return;

            }

        }
        else if (parsed_query.operation == "insert") {

            Record new_record = parse_string(parsed_query.value);

            cout << new_record << endl;

            insert_record( new_record );

            if (parsed_query.attribute == "update") {

                Record* search_record = index->search( new_record );
                search_record->title = new_record.title;
                search_record->authors = new_record.authors;
                search_record->publisher = new_record.publisher;
                search_record->publish_date = new_record.publish_date;


            }
            else {
                index->insertNode( new_record );            
            }

            return;


        }
        else if (parsed_query.operation == "delete") {

            unsigned int ID = -1;

            if (parsed_query.attribute == "name") {

                Record search_record;
                search_record.title = parsed_query.value;

                Record* search_result = index->search( search_record );

                if (search_result == nullptr) {
                    return;
                }

                ID = search_result->ID;

            }
            else {

                ID = stoi(parsed_query.value);
            
            }

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

                int i = ID * sizeof(Record);

                for (int c = 0; c < sizeof(Record); c++) {
                    ptr[i] = 0;
                    ++i;
                }

            }

            // Closing
            close(fd);


            // Updating index
            build_index();

            return;


        }
        
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
    Record get_record(unsigned int ID) {

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

        bool stringIsNull = true;
        for (char c : retrieved_record)
            if (c != 0)
                stringIsNull = false;


        // Closing
        close(fd);

        // If record is not found

        if (stringIsNull) {

            // LOG("(!) Record could not be found!", true);
            
            Record dummy;
            dummy.ID = -1;

            return dummy;
            
        }


        // Logging
        // LOG("(!) Successfully retrieved record...");

        // Validating
        return parse_string(retrieved_record);;

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

        Record output;

        
        output.ID = stoi(tokens[0]);
        output.title = tokens[1];
        

        token = "";
        vector <string> author_tokens;
        for (char c : tokens[2]) {
            
            if (c == ',' || c == '\0' || c == ')') {
                author_tokens.push_back(token);
                token = "";
            }
            else {
                if (c != '(')
                    token += c;
            }
        
        }
        
        author_tokens.push_back(token);

        output.authors = author_tokens;

        output.publish_date = tokens[3];
        output.publisher = tokens[4];

        return output;

    }


    ~DBMS() {
        LOG("(-) Database closed...");
    }

};


int main() {


    DBMS::Record r[3];
    
    r[0].ID = 1;
    r[1].ID = 2;
    r[2].ID = 3;

    r[0].title = "Operating Systems";
    r[1].title = "Probability";

    DBMS db;

    db.insert_record(r[0]);
    db.insert_record(r[1]);

    db.build_index();

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

        string client_input = read_buffer;
        string query_output = "(!) Query executed...";
        db.run_query(client_input, query_output);
        clean_buffer(read_buffer);

        // Sending a message to client
        send(accept_socket, query_output.c_str(), query_output.length(), 0);
    
    }


    return 0;
}

