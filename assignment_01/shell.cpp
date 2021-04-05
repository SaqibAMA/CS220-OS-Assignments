#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

class command {

private:
  string name;              // keeps command
  vector <string> argv;     // keeps argv and argc

public:

  // constructor
  command() {
    name = "";              // no command
    argv.resize(0);         // no arguments
  }

  // input operator
  friend istream& operator >> (istream& in, command& c) {

    cout << ">> ";

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
  string getBinPath() {

    string pname = "/bin/" + name;
    return pname;

  }


  // destructor
  ~command() {
    
    cout << "--" << endl;
    
  }

};

int main() {

  while (true) {
    command c;
    cin >> c;
    c.execute();
  }

  return 0;

}