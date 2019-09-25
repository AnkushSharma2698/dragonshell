#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <unistd.h>
#include <signal.h>
#include "execution_handler.h"

std::vector<std::string> $PATH = {"/bin/",  "/usr/bin/"};

// Combine fork and execve if u want to run an external program
void cd(std::string str_path) {
    pid_t cpid = fork();
    if (cpid == 0) {
        if (str_path == "") {
            std::cout << "expected argument to \"cd\"" << "\n";
            return;
        }
        // Convert my string path to a const char *
        const char *path = str_path.c_str();
        if (chdir(path) != 0) {
            std::cout << "No such file or directory" << "\n";
        }
    } else if(cpid < 0){
        std::cout << "Fork Failed";
        _exit(0);
    } else {
        wait(NULL);
    }
};

void pwd() {
    char s[100];
    std::cout << getcwd(s,100) << "\n";
};

void show$PATH() {
    std::string s = "Current Path: ";
    for (int i = 0; i < $PATH.size();i++) {
        if (i + 1 == $PATH.size()) {
            s = s + $PATH[i];
        } else {
            s = s + $PATH[i] + ":";
        }
    }
    std::cout << s << "\n";
}

void appendToPath(std::string &append) {
    std::cout << "APPEND TO PATH" << "\n"
}



// REMEMBER getcwd()
int executeInstructions(std::vector<std::string> &instructions) {
    // Handle enter case with nothing???
    if (instructions[0] == "cd") {
        cd(instructions[1]);
    } else if (instructions[0] == "pwd"){
        pwd();
    } else if (instructions[0] == "$PATH") {
        show$PATH();
    }
    else {
        std::cout << "Command not Found" << "\n";
    }
    return 1;
}