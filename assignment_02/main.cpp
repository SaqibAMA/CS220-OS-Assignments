#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <cstdlib>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

using namespace std;

class command {

public:
    string name;              // keeps command
    vector <string> argv;     // keeps argv and argc

public:

    // constructor
    command() {
        name = "";              // no command
        argv.resize(0);         // no arguments
    }

    // copy constructor
    command(const command& c) {
        this->name = c.name;
        this->argv = c.argv;
    }

    // assignment operator
    command& operator = (const command& c) = default;

    // input operator
    friend istream& operator >> (istream& in, command& c) {

        cout << "shell> ";

        char buffer[1000];                      // io buffer
        in.getline(buffer, 1000);

        unsigned int bufferIterator = 0;        // tracking buffer index


        // extracting command


        while (
                buffer[bufferIterator] != 0 &&
                buffer[bufferIterator] != '\n' &&
                buffer[bufferIterator] != ' ') {

            c.name += buffer[bufferIterator++];   // storing the command

        }
        ++ bufferIterator;                      // to accomodate space

        // extracting arguments

        string argument;

        while (bufferIterator < strlen(buffer)) {

            argument += buffer[bufferIterator++];

            if (buffer[bufferIterator] == '\n' ||
                buffer[bufferIterator] == ' ' ||
                buffer[bufferIterator] == 0) {

                c.argv.push_back(argument);
                argument = "";
                ++bufferIterator;

            }

        }


        return in;                              // returning the output stream

    }

    // input parsing function
    void parseCommand (const char* buffer)
    {

        unsigned int bufferIterator = 0;        // tracking buffer index


        // extracting command

        while (
                buffer[bufferIterator] != 0 &&
                buffer[bufferIterator] != '\n' &&
                buffer[bufferIterator] != ' ') {

            name += buffer[bufferIterator++];   // storing the command

        }
        ++ bufferIterator;                      // to accomodate space

        // extracting arguments

        string argument;

        while (bufferIterator < strlen(buffer)) {

            argument += buffer[bufferIterator++];

            if (buffer[bufferIterator] == '\n' ||
                buffer[bufferIterator] == ' ' ||
                buffer[bufferIterator] == 0) {

                argv.push_back(argument);
                argument = "";
                ++bufferIterator;

            }

        }

    }

    // File IO function
    bool executeFileIO(char* previousBuffer = 0) {

        /*
         * This function exclusively deals with only the commands that
         * have fileIO redirection in them.
         * */

        string currentCommand = name;
        string filename;

        bool in = false;                        // Is it an input redirection?
        bool out = false;                       // Is it an output redirection?
        bool outOnly;                           // Is it both?

        {

            // Getting command
            int i = 0;
            while (argv[i] != ">" && argv[i] != "<") {
                currentCommand += " ";
                currentCommand += argv[i];
                i++;
            }

            // Getting filename
            filename = argv[i + 1];

            // Checking IO
            for (int j = i; j < argv.size(); j++) {
                if (argv[j] == ">") out = true;
                else if (argv[j] == "<") in = true;
            }

        }

        outOnly = out && !in;

        if (outOnly) {

            // Parsing command
            command newCommand;
            newCommand.parseCommand(currentCommand.c_str());

            // Output file
            int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC);

            // Input pipe, helpful for carrying the previous output
            int inPipe[2];
            pipe(inPipe);

            // If there is previous output from the pipe, copy it in the IN pipe
            if (previousBuffer) {

                if (fork() == 0) {

                    // remember to clean input because it might cause errors
                    close(inPipe[0]);
                    int bytesWritten = write(inPipe[1], previousBuffer, strlen(previousBuffer) + 1);
                    close(inPipe[1]);

                    exit(1);

                }

                int previousBufferWriteStatus = -1;
                wait(&previousBufferWriteStatus);

                if (previousBufferWriteStatus == -1) cout << "Something went wrong while writing previous buffer" << endl;

                close(inPipe[1]);


            }

            // Execute the command
            if (fork() == 0) {

                dup2(fd, 1);                    // Put the output in the file
                close(inPipe[1]);
                dup2(inPipe[0], 0);         // Take the input from the input pipe

                newCommand.execute();               // Execute the command normally

                close(inPipe[0]);               // Closing pipes

                close(fd);

                exit(0);

            }

            int fileWriteStatus = -1;
            wait(&fileWriteStatus);

            close(fd);
            close(inPipe[0]);
            close(inPipe[1]);

            if (fileWriteStatus == -1) cout << "Something went wrong while writing to file!" << endl;



        }
        else if (in) {

            // Parse command
            command newCommand;
            newCommand.parseCommand(currentCommand.c_str());

            // Opening file to take input
            int fd = open(filename.c_str(), O_RDONLY);

            // Pipe to take output
            int outPipe[2];
            pipe(outPipe);

            // If for some reason file doesn't exist
            if (fd < 0) {
                cout << "Error: File does not exist!" << endl;
                return false;
            }

            // Execute the command
            if (fork() == 0) {

                dup2(fd, 0);
                close(outPipe[0]);
                dup2(outPipe[1], 1);            // Put the output in the out pipe

                newCommand.execute();

                close(fd);
                close(outPipe[1]);
                exit(0);

            }

            int fileReadStatus = -1;
            wait(&fileReadStatus);

            close(outPipe[1]);
            close(fd);

            char consoleBuffer[1000] = {0};
            read(outPipe[0], consoleBuffer, 1000);

            close(outPipe[0]);

            if (fileReadStatus == -1) cout << "Something went wrong while reading from file!" << endl;


            // Handling the case where it is command < file1.txt > file2.txt
            if (out) {

                {
                    int i = 0;
                    while (argv[i] != ">")
                        i++;

                    string outFile = argv[i + 1];

                    ofstream fout;
                    fout.open(outFile);

                    int c = 0;
                    while (consoleBuffer[c]) {
                        fout << consoleBuffer[c++];
                    }

                    fout.close();

                }

            }
            else {

                cout << consoleBuffer;

            }


        }




        return true;

    }

    // execute command
    bool execute() {

        if (isRedirected()) {
            return executeFileIO();
        }

        // special case -- cd
        if (strcmp(name.c_str(), "cd") == 0) {

            chdir(argv[0].c_str());
            cout << "Path changed to " << argv[0].c_str() << endl;

            return true;

        }

        if (fork() == 0) {

            // argv.size() for arguments. +1 for name +1 for NULL
            char* argv_list[argv.size() + 1 + 1];


            // copying name
            argv_list[0] = new char[name.length()];
            strcpy(argv_list[0], name.c_str());


            // copying args
            for (int i = 1; i < argv.size() + 1; i++) {

                argv_list[i] = new char[argv[i - 1].length()];
                strcpy(argv_list[i], argv[i - 1].c_str());

            }

            // marking end
            argv_list[argv.size() + 1] = NULL;


            // fetching /bin/ path name
//            string pname = getBinPath();
//            char* pathname = new char[pname.length()];
//            strcpy(pathname, pname.c_str());

            // sending execv() call
            // if the call is invalid
            if (execvp(argv_list[0], argv_list) < 0) {
                cout << "Something went wrong" << endl;
                exit(1);
            }

            // exiting
            exit(0);

        }
        else {

            wait(NULL);
            return true;

        }

    }


    // pathname function
//    string getBinPath() const {
//
//        string pname = "/bin/" + name;
//        return pname;
//
//    }

    // Checking redirection
    bool isRedirected() const {

        string out = ">";
        string in = "<";

        for (auto x : argv)
            if (x == out || x == in) return true;

        return false;

    }

    // Getting original command
//    string getOriginalCommand() const {
//
//        string c = name;
//
//        for (auto x : argv) {
//            c += " ";
//            c += x;
//        }
//
//        return c;
//
//    }


    // Getters and Setters
    string& getName() {
        return name;
    }

    vector <string>& getArgv() {
        return argv;
    }


    // destructor
    ~command() = default;



};


// Extension of command class that handles filtering and file redirection

class command_ext {

private:
    vector <command> c;     // keeps all commands

public:

    // Constructor
    command_ext() {

    }

    // Input
    friend istream& operator >> (istream& in, command_ext& c) {

        cout << "shell> ";
        char buffer[1000];

        in.getline(buffer, 1000);

        int bufferIterator = 0;

        // extracting individual commands

        while (bufferIterator < strlen(buffer)) {

            string cmd;

            while (buffer[bufferIterator] != '|' && buffer[bufferIterator] != 0) {

                cmd.push_back(buffer[bufferIterator]);
                bufferIterator++;

            }

            while (buffer[bufferIterator] == ' ' || buffer[bufferIterator] == '|')
                bufferIterator++;

            if (cmd[cmd.size() - 1] == ' ')
                cmd.pop_back();


            command currentCommand;
            currentCommand.parseCommand(cmd.c_str());

            c.c.push_back(currentCommand);

        }


        // returning istream
        return in;

    }


    // Core execution function
    bool execute() {

        // Ideal for single commands.


        if (c.size() == 1) {
            return c[0].execute();
        }

        unsigned int commandNumber = 0;

        int in[2];
        int out[2];

        pipe(in);
        pipe(out);

        char consoleBuffer[1000] = {0};

        if (fork() == 0) {

            // IO
            dup2(in[0], 0);
            dup2(out[1], 1);

            // Execution
            c[commandNumber].execute();

            exit(0);

        }

        int processOneStatus = -1;
        wait(&processOneStatus);

        if (processOneStatus != 0) cout << "Something went wrong!" << endl;


        close(in[0]);
        close(in[1]);
        close(out[1]);
        read(out[0], consoleBuffer, 1000);
        close(out[0]);

        // If there are more commands, this function will handle them.
        return executeWithInput(consoleBuffer, commandNumber + 1);

    }

    // execute function with input
    bool executeWithInput(char* consoleBuffer, unsigned int commandNumber) {

        // Remove nulls from between
        cleanInput(consoleBuffer);

        // If we have reached the end
        if (commandNumber >= c.size()) {

            cout << consoleBuffer;
            return true;

        }


        int in[2];
        int out[2];

        pipe(in);
        pipe(out);

        if (fork() == 0) {

            close(in[0]);
            int bytesWritten = write(in[1], consoleBuffer, strlen(consoleBuffer) + 1);
            close(in[1]);

            exit(0);

        }

        int inPipeWriteStatus = -1;
        wait(&inPipeWriteStatus);

        close(in[1]);


        if (fork() == 0) {

            dup2(out[1], 1);
            dup2(in[0], 0);

            close(out[0]);
            close(in[1]);

            if (c[commandNumber].isRedirected())
                c[commandNumber].executeFileIO(consoleBuffer);
            else
                c[commandNumber].execute();

            close(out[1]);
            close(in[0]);

            exit(0);

        }

        int processExecutionStatus = -1;
        wait(&processExecutionStatus);


        char temporaryBuffer[1000] = {0};

        close(out[1]);
        read(out[0], temporaryBuffer, 1000);
        close(out[0]);


        return executeWithInput(temporaryBuffer, commandNumber + 1);

    }

    void cleanInput(char* consoleBuffer) {

        /*
         * There were some issues when reading the input from the pipe
         * into a char array. This is the clean input function that takes that
         * char array and removes null characters from between the text.
         *
         *
         * Saqi0b\000 => Saqib\000
         *
         * */

        if (!consoleBuffer && strlen(consoleBuffer) == 0) return;

        bool startCleaning = false;
        const int bSize = 1000;

        for (int i = bSize; i >= 0; i--) {

            // We have to start cleaning from this point onwards
            if (consoleBuffer[i])
                startCleaning = true;

            // If we are in between the text
            if (startCleaning) {

                if (!consoleBuffer[i]) {

                    // Shift the array one character left
                    int j = i;
                    while (j < bSize - 1) {
                        consoleBuffer[j] = consoleBuffer[j + 1];
                        j++;
                    }

                }

            }

        }

    }

    // Destructor
    ~command_ext() {
        // Not needed
    }

};


int main() {


    while (true) {
        command_ext cmd;
        cin >> cmd;
        cmd.execute();
    }








    return 0;

}
