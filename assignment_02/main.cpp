#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

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
    command& operator = (const command& c) {

        this->name = c.name;
        this->argv = c.argv;

        return *this;

    }

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


    // execute command
    bool execute() {

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
            string pname = getBinPath();
            char* pathname = new char[pname.length()];
            strcpy(pathname, pname.c_str());

            // sending execv() call
            // if the call is invalid
            if (execv(pathname, argv_list) == -1) {

                // if it was not an internal command
                // we use the path provided by user

                free(pathname);
                pathname = new char[name.length()];
                strcpy(pathname, name.c_str());


                // if the external command is invalid.
                if (execv(pathname, argv_list) == -1) {
                    cout << "Something went wrong." << endl;
                }



            }

            // exiting
            exit(0);

            return true;

        }
        else {

            wait(NULL);
            return true;

        }

    }


    // pathname function
    string getBinPath() const {

        string pname = "/bin/" + name;
        return pname;

    }

    // Checking redirection
    bool isRedirected() const {

        string out = ">";
        string in = "<";

        for (auto x : argv)
            if (x == out || x == in) return true;

        return false;

    }

    // Getting original command
    string getOriginalCommand() const {

        string c = name;

        for (auto x : argv) {
            c += " ";
            c += x;
        }

        return c;

    }


    // Getters and Setters
    string& getName() {
        return name;
    }

    vector <string>& getArgv() {
        return argv;
    }


    // destructor
    ~command() {

    }



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


        // Handling single commands
        if (c.size() == 1) {

            if (c[0].isRedirected()) {

                // handling single redirected command
                return executeSingleDirected(c[0]);

            }
            else {

                // dealing with simple commands with built-in execution
                return c[0].execute();

            }

        }

        // handling multiple commands

        unsigned int commandNumber = 0;

        char consoleBuffer[1000] = {0};

        for (commandNumber = 0; commandNumber < c.size(); commandNumber++) {

            if (commandNumber == 0) {

                string currentCommand = c[commandNumber].getOriginalCommand();
                currentCommand += " > pipe-1-out.txt";

                command cc;
                cc.parseCommand(currentCommand.c_str());

                executeSingleDirected(cc);

            }
            else if (commandNumber > 0 && commandNumber < c.size() - 1) {


                {
                    string currentCommand = c[commandNumber].getOriginalCommand();
                    currentCommand += " < pipe-out.txt";

                    command cc;
                    cc.parseCommand(currentCommand.c_str());

                    executeSingleDirected(cc);
                }

                {

                }

            }

        }


        return true;
    }

    // executing single directed commands
    bool executeSingleDirected(const command& cmd) {

        // flags
        bool in = false;
        bool out = false;

        // determining the direction
        for (auto x : cmd.argv) {

            if (strcmp(x.c_str(), ">") == 0)
                out = true;
            else if (strcmp(x.c_str(), "<") == 0)
                in = true;

        }

        pair<command, string> dividedCommand = splitCommand(cmd);

        if (out) {

            int pipe = open(dividedCommand.second.c_str(), O_CREAT | O_WRONLY | O_TRUNC);

            if (fork() == 0) {

                dup2(pipe, 1);
                char** argv_list = parseArguments(dividedCommand.first);

                if (execvp(argv_list[0], argv_list) < 0)
                    cout << "Error: Can't execute" << endl;

            }
            else {

                wait(NULL);
                close(pipe);

            }


        }
        else if (in) {

            int pipe = open(dividedCommand.second.c_str(), O_RDONLY);
            int pipe_out = open("pipe-out.txt", O_WRONLY | O_CREAT | O_TRUNC);

            if (fork() == 0) {

                dup2(pipe, 0);
                dup2(pipe_out, 1);
                char** argv_list = parseArguments(dividedCommand.first);

                if (execvp(argv_list[0], argv_list) < 0)
                    cout << "Error: Can't execute" << endl;

            }
            else {

                wait(NULL);
                close(pipe);
                close(pipe_out);

            }


        }

        return true;

    }

    // split directed commands
    pair<command, string> splitCommand(const command& cmd) {


        pair<command, string> out;

        string firstPart = cmd.name;
        string filename;

        {
            int i = 0;
            while (cmd.argv[i] != ">" && cmd.argv[i] != "<"){
                firstPart += " ";
                firstPart += cmd.argv[i];
                ++i;
            }

            ++i;

            filename = cmd.argv[i];

        }


        command c;
        c.parseCommand(firstPart.c_str());

        out.first = c;
        out.second = filename;



        return out;

    }


    // arguments parsing function
    char** parseArguments(const command& cmd) {

        // argv.size() for arguments. +1 for name +1 for NULL
        char** argv_list = new char*[cmd.argv.size() + 2];


        // copying name
        argv_list[0] = new char[cmd.name.length()];
        strcpy(argv_list[0], cmd.name.c_str());


        // copying args
        for (int i = 1; i < cmd.argv.size() + 1; i++) {

            argv_list[i] = new char[cmd.argv[i - 1].length()];
            strcpy(argv_list[i], cmd.argv[i - 1].c_str());

        }

        // marking end
        argv_list[cmd.argv.size() + 1] = NULL;


        // returning args
        return argv_list;

    }

    // Destructor
    ~command_ext() {

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
