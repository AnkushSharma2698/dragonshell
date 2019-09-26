#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <unistd.h>
#include <signal.h>
#include "execution_handler.h"

std::vector<std::string> PATH = {"/bin/",  "/usr/bin/"}; // Global variable storing the current PATH var of the shell
std::vector<pid_t> processes = {};

std::vector<std::string> tokenizer(const std::string &str, const char *delim) {
    char* cstr = new char[str.size() + 1];
    std::strcpy(cstr, str.c_str());

    char* tokenized_string = strtok(cstr, delim);

    std::vector<std::string> tokens;
    while (tokenized_string != NULL)
    {
        tokens.push_back(std::string(tokenized_string));
        tokenized_string = strtok(NULL, delim);
    }
    delete[] cstr;

    return tokens;
}

// print method for a vector < -- Make sure to delete this code once you are done
void printer(std::vector<std::string> const &input) {
    for (int i = 0; i < input.size(); i++) {
        std::cout << i;
        std::cout << input.at(i) << ' ';
        }
        std::cout << '\n';
}

// Combine fork and execve if u want to run an external program
void cd(std::string str_path) {
    if (str_path == "") {
        std::cout << "expected argument to \"cd\"" << "\n";
        return;
    }
    // Convert my string path to a const char *
    const char *path = str_path.c_str();
    if (chdir(path) != 0) {
        std::cout << "No such file or directory" << "\n";
    }
};

void pwd() {
    char* s = new char[100];
    std::cout << getcwd(s,100) << "\n";
    delete[] s; // cleaning mem
};

void show$PATH() {
    std::string s = "Current Path: ";
    for (int i = 0; i < PATH.size();i++) {
        if (i + 1 == PATH.size()) {
            s = s + PATH[i];
        } else {
            s = s + PATH[i] + ":";
        }
    }
    std::cout << s << "\n";
}

void appendToPath(std::vector<std::string> &append) {
    // Now that we are in the appending block we must ensure that we are getting adequate values
    // If no $PATH, ensure that we are given a path to overwrite the global $PATH
    std::vector<std::string> tokenized_a2path = tokenizer(append[1], ":");
    // Check if the first item in the tokenized a2path is a $PATH variable, because then we would append
    if (tokenized_a2path.size() == 0) {
        PATH = {""}; // Overwrite the path vector completely
    }
    else if (tokenized_a2path[0] == "$PATH") {
        // Append whatever is passed in directly to the
        for (int i = 1; i< tokenized_a2path.size();i++) {
            PATH.push_back(tokenized_a2path[i]);
        }
    } else {
        PATH = {};
        for (int i = 0; i< tokenized_a2path.size();i++) {
            PATH.push_back(tokenized_a2path[i]);
        }
    }
}

void exitDragonShell() { // TODO ask about how exit will close the child processes and such before terminating itself.
    std::cout << "Exiting" << "\n";
    // Kill any running processes
    for (int i = 0; i< processes.size();i++) {
        kill(processes[i], SIGTERM);
    }
    // Get the pid of the main process, and exit that at the end
    pid_t pid = getpid();
    _exit(pid);
}

bool absPath(std::vector<std::string> &instructions) {
    std::cout <<instructions[0][0] << "\n";
    if (instructions[0][0] == '/') {
        std::cout << "THIS IS AN ABS";
        return true;
    }
    return false;
}

bool relPath(std::vector<std::string> &instructions) {
    if (instructions[0][0] == '/') {
        return false;
    }
    return true;
}

int checkPATH(std::vector<std::string> &instructions) {
    std::cout << "CHECKING VS THE PATH" << "\n";
    // Test if this is a full path and works
    if (absPath(instructions)) {
        // Do some stuff
        // Check
        pid_t cid = fork();
        if (cid == -1) {
            perror("fork error");
            _exit(1);
        }
        else if (cid == 0){
            int ret;
            char *cmd[] = { (char *)"ls", (char *)"-l", (char *)0 };
            char *env[] = { (char *)"HOME=/usr/home", (char *)"LOGNAME=home", (char *)0 };
            ret = execve ("/bin/ls", cmd, env);
            _exit(0);
        }
        else {
            wait(NULL);
            return 1;
        }

    } else if (relPath(instructions)) {

    } else {
        // Shit dont work
    }
    // Test if it is a relative path and is in the PATH
    return 1;
}


// DRIVER FOR THE EXECUTION STEP
int executeInstructions(std::vector<std::string> &instructions) {
    // Handle enter case with nothing???
    if (instructions[0] == "cd") {
        cd(instructions[1]);
    } else if (instructions[0] == "pwd"){
        pwd();
    } else if (instructions[0] == "$PATH") {
        show$PATH();
    }else if (instructions[0] == "a2path"){
        appendToPath(instructions);
    }
    else if (instructions[0] == "exit") {
        exitDragonShell();
    } else if (instructions.size() == 0) {
        // Case where nothing is in the vector
        return 1;
    }
    else {
        // Check if the command exists in the PATH, else command will not be found
        int i = checkPATH(instructions); // TODO figure out this function
        if (i == 0) {
            std::cout << "dragonshell: Command not Found" << "\n";
        }
    }
    return 1;
}