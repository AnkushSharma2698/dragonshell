#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <fstream>
#include <sys/stat.h>
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

// Basic checkers for path types
bool absPath(std::vector<std::string> &instructions) {
    if (instructions[0][0] == '/') {
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

// Pointer to first position of the cmd args
void set_args(std::vector<std::string> &args, char ** cmd, const char * delim) {
    std::vector<std::string> first_arg = tokenizer(args[0], delim);
    cmd[0] = (char *)first_arg[first_arg.size() - 1].c_str();

    // Add in any other required arguments
    for (int i = 1; i < args.size() + 1; i++) {
        cmd[i] = (char * )args[i].c_str();
    }
    // Add NULL at the end
    cmd[args.size()] = (char *) 0;

}

// Run the external commands made by the user
void external_cmd(std::vector<std::string> &instructions, const char *delim) {
    pid_t cid = fork();
    if (cid == -1) {
        perror("fork error");
        return;
    } else if (cid == 0){
        char * exe_cmd[instructions.size() + 1];
        set_args(instructions, exe_cmd, delim); // Set args for the cmd
        char *env[] = {NULL};

        int ret;
        ret = execve (instructions[0].c_str(), exe_cmd, env);
        if (ret == -1) {
            perror("Command Failed");
        }
        _exit(0);
    }
    else {
        processes.push_back(cid);
        wait(NULL);
    }
}

// Check if a file at a given path exists
bool exists(std::string path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return true;
    } else {
        return false;
    }
}

void run_abs_cmd(std::vector<std::string> &instructions) {
    int cmd_exists = 0;
    if (exists(instructions[0])) {
        external_cmd(instructions, "/");
        cmd_exists = 1;
    }
    if (cmd_exists == 0) {
        std::cout << "dragonshell: Command not Found" << "\n";
    }
}

void run_rel_cmd(std::vector<std::string> &instructions) {
    int cmd_exists = 0;
    std::string s = instructions[0];
    for (int i=0; i< PATH.size(); i++) {
        std::string temp_path = PATH[i];
        instructions[0] = temp_path.append(s);
        if (exists(instructions[0])) {
            external_cmd(instructions, "/");
            cmd_exists = 1;
        };
    }
    if (cmd_exists == 0) {
        std::cout << "dragonshell: Command not Found" << "\n";
    }
}


void checkPATH(std::vector<std::string> &instructions) {
    if (absPath(instructions)) {
        run_abs_cmd(instructions);
    } else if (relPath(instructions)) {
        run_rel_cmd(instructions);
    }
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
        checkPATH(instructions); // TODO figure out this function
    }
    return 1;
}