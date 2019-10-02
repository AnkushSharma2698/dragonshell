#include <iostream>
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
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

// Combine fork and execve if u want to run an external program
void cd(std::string str_path) {
    if (str_path == "") {
        std::cout << "dragonshell: expected argument to \"cd\"" << "\n";
        return;
    }
    // Convert my string path to a const char *
    const char *path = str_path.c_str();
    if (chdir(path) != 0) {
        std::cout << "dragonshell: No such file or directory" << "\n";
    }
};

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
        for (unsigned int i = 1; i< tokenized_a2path.size();i++) {
            PATH.push_back(tokenized_a2path[i]);
        }
    } else {
        PATH = {};
        for (unsigned int i = 0; i< tokenized_a2path.size();i++) {
            PATH.push_back(tokenized_a2path[i]);
        }
    }
}

void exitDragonShell() { // TODO ask about how exit will close the child processes and such before terminating itself.
    std::cout << "Exiting" << "\n";
    // Kill any running processes
    for (unsigned int i = 0; i< processes.size();i++) {
        kill(processes[i], SIGTERM);
    }
    // Get the pid of the main process, and exit that at the end
    pid_t pid = getpid();
    _exit(pid);
}

void killBackgroundProcesses() {
    // Kill any running processes
    for (unsigned int i = 0; i< processes.size();i++) {
        kill(processes[i], SIGTERM);
    }
}

// Check if the given path is an absolute path
bool absPath(std::vector<std::string> &instructions) {
    if (instructions[0][0] == '/') {
        return true;
    }
    return false;
}

// Check if the given path is a relative path
bool relPath(std::vector<std::string> &instructions) {
    if (instructions[0][0] == '/') {
        return false;
    }
    return true;
}

// Check if the given instructions contain a pipe symbol
int needsPipe(std::vector<std::string> &instructions) {
    int needs_pipe = -1;
    for (unsigned int i=0; i < instructions.size(); i++) {
        if (instructions[i] == "|") {
            needs_pipe = i;
        }
    }
    return needs_pipe;
}

int isBackgroundProcess(std::vector<std::string> &instructions) {
    int run_in_background = 0;
    for (unsigned int i = 0; i < instructions.size(); i++) {
        if (instructions[i] == "&") {
            run_in_background = 1;
        }
    }
    return run_in_background;
}

std::tuple<int, std::vector<std::string>> needsOutputRedirect(std::vector<std::string> &instructions, std::string delim) {
    int redirect = -1;
    for (unsigned int i=0; i < instructions.size(); i++) {
        if (instructions[i] == delim) {
            instructions.erase(instructions.begin() + i);
            redirect = i;
        }
    }
    // If after an initial screen, redirect is not -1, return instructions in current state
    if (redirect != -1) {
        return std::make_tuple(redirect, instructions);
    }

    // Check for a substring
    std::vector<std::string> delim_handler_vector;
    for (unsigned int i=0; i< instructions.size(); i++) {
        if (instructions[i].find(delim) != std::string::npos) {
            delim_handler_vector = tokenizer(instructions[i], delim.c_str());
            instructions.erase(instructions.begin() + i);
            instructions.insert(instructions.end(), delim_handler_vector.begin(), delim_handler_vector.end());
            redirect = instructions.size() - 1;
            break;

        }
    }
    return std::make_tuple(redirect, instructions);
}

// Pointer to first position of the cmd args
void set_args(std::vector<std::string> &args, char ** cmd, const char * delim) {
    std::vector<std::string> first_arg = tokenizer(args[0], delim);
    cmd[0] = (char *)first_arg[first_arg.size() - 1].c_str();

    // Add in any other required arguments
    for (unsigned int i = 1; i < args.size() + 1; i++) {
        cmd[i] = (char * )args[i].c_str();
    }
    // Add NULL at the end
    cmd[args.size()] = (char *) 0;

}

// Run the external commands made by the user
void general_cmd(std::vector<std::string> &instructions, char **cmd, int run_in_background) {
    int status;
    pid_t cid = fork();
    if (cid == -1) {
        perror("fork error");
        return;
    } else if (cid == 0){
        char *env[] = {NULL};
        int ret;
        if (run_in_background) {
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
        }
        ret = execve (instructions[0].c_str(), cmd, env);
        if (ret == -1) {
            perror("Command Failed");
        }
        _exit(0);
    }
    else {
        processes.push_back(cid);
        if (run_in_background) {
            std::cout << "PID " << cid << " is running in the background" << "\n";
            waitpid(cid, &status, 0);
        } else {
            wait(NULL);
        }
    }
}


void run_pipe_cmd(std::vector<std::string> &pipe_in, std::vector<std::string> &pipe_out,char **in_cmd, char **out_cmd, int run_in_background) {
    pid_t pid;
    int status;
    if ((pid = fork()) < 0) perror("fork error!");
    if (pid == 0) {
        pid_t pipe_pid;
        int fd[2];
        if (pipe(fd) < 0) perror("pipe error!");
        if ((pipe_pid = fork()) < 0) perror("fork error!");
        if (pipe_pid == 0) {
            // add the stdout of this code to the pipe
            char *env[] = {NULL};
            close(fd[0]); // No reading
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            if (run_in_background) {
                close(STDOUT_FILENO);
                close(STDERR_FILENO);
            }
            if (execve(pipe_in[0].c_str(), in_cmd, env) < 0) {
                perror("stdout error!");
            }
            _exit(0);
        } else {
            // In this parent is where we want to read the stdout from the child
            char *env[] = {NULL};
            close(fd[1]); // No writing
            dup2(fd[0], STDIN_FILENO); // stdin = fd[0]
            close(fd[0]);
            if (run_in_background) {
                close(STDOUT_FILENO);
                close(STDERR_FILENO);
            }
            if (execve(pipe_out[0].c_str(), out_cmd, env) < 0) {
                perror("stdin error!");
            }

            _exit(0);
        }
    } else {
        processes.push_back(pid);
        if (run_in_background) {
            std::cout << "PID " << pid << " is running in the background" << "\n";
            waitpid(pid, &status, 0);
        } else {
            wait(NULL);
        }
    }
}

void run_redirect_cmd(std::vector<std::string> &instructions, std::vector<std::string> &output_file,char ** cmd, int run_in_background) {
    int status;
    pid_t cid = fork();
    if (cid == -1) {
        perror("fork error");
        return;
    } else if (cid == 0) { // Some special output handling will go down here
        int fd = open(output_file[0].c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
        if (fd  < 0) {
            perror("encountered open error");
        }
        dup2(fd, 1); // Send the stdout to the file

        close(fd);
        if (run_in_background) {
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
        }
        char *env[] = {NULL};
        int ret;
        ret = execve(instructions[0].c_str(), cmd, env);
        if (ret == -1) {
            perror("Command failed");
        }
        _exit(0);
    } else {
        processes.push_back(cid);
        if (run_in_background) {
            std::cout << "PID " << cid << " is running in the background" << "\n";
            waitpid(cid, &status, 0);
        } else {
            wait(NULL);
        }
    }
}

// Parse the instructions to exclude the > or |
std::vector<std::string> set_instructions(std::vector<std::string> &instructions, int position) {
    std::vector<std::string> new_instructions;
    for (int i = 0; i < position; i++) {
        new_instructions.push_back(std::string(instructions[i]));
    }
    return new_instructions;
}

// Set the output around a | or >
std::vector<std::string> set_output(std::vector<std::string> &instructions, int position) {
    std::vector<std::string> output;
    if (instructions.size() < 2) { // If there is nothing after the index of the | or >
        output.push_back("");
        return output;
    }
    for (unsigned int i = position; i < instructions.size(); i++) {
        output.push_back(instructions[i]);
    }
    return output;
}

void pwd(int redirect, std::vector<std::string> &instructions) {
    if (redirect != -1) { // Handle redirect case
        pid_t cid = fork();
        if (cid == -1) {
            perror("fork error");
            return;
        } else if (cid == 0) { // Some special output handling will go down here
            char* s = new char[100];
            std::vector<std::string> output_file = set_output(instructions, redirect);
            int fd = open(output_file[0].c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
            if (fd  < 0) {
                perror("encountered open error");
            }
            dup2(fd, 1); // Send the stdout to the file
            close(fd);
            std::cout << getcwd(s,100) << "\n";
            delete[] s; // cleaning mem
            _exit(0);
        } else {
            processes.push_back(cid);
            wait(NULL);
        }
    } else {
        char* s = new char[100];
        std::cout << getcwd(s,100) << "\n";
        delete[] s; // cleaning mem
    }

};

void show$PATH(int redirect, std::vector<std::string> &instructions) {
    std::string s = "Current Path: ";
    for (unsigned int i = 0; i < PATH.size();i++) {
        if (i + 1 == PATH.size()) {
            s = s + PATH[i];
        } else {
            s = s + PATH[i] + ":";
        }
    }
    if (redirect != -1) {
        pid_t cid = fork();
        if (cid == -1) {
            perror("fork error");
            return;
        } else if (cid == 0) { // Some special output handling will go down here
            std::vector<std::string> output_file = set_output(instructions, redirect);
            int fd = open(output_file[0].c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
            if (fd  < 0) {
                perror("encountered open error");
            }
            dup2(fd, 1); // Send the stdout to the file
            close(fd);
            std::cout << s << "\n";
            _exit(0);
        } else {
            processes.push_back(cid);
            wait(NULL);
        }
    } else {
        std::cout << s << "\n";
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

std::tuple<std::vector<std::string>, std::vector<std::string>> get_pipe_instructions(std::vector<std::string> &instructions, int pipe_index) {
    std::vector<std::string> pipe_from;
    std::vector<std::string> pipe_to;

    // Check if the second half of the pipe statement is an absolute path, if not then we have to make it one
    std::string s = instructions[pipe_index + 1];
    if (instructions[pipe_index + 1][0] != '/') {
        if (exists(instructions[pipe_index + 1]) == false) {
            for (unsigned int i=0; i< PATH.size(); i++) { // This block compares the command against the PATH
                std::string temp_path = PATH[i];
                instructions[pipe_index + 1] = temp_path.append(s);
                if (exists(instructions[pipe_index + 1])) {
                    break;
                }
            }
        }
    }

    for (int i = 0; i < pipe_index; i++) {
        pipe_from.push_back(instructions[i]);
    }
    for (unsigned int i = pipe_index + 1; i < instructions.size(); i++) {
        pipe_to.push_back(instructions[i]);
    }
    return std::make_tuple(pipe_from, pipe_to);
}

// Runs absolute path commands
void run_abs_cmd(std::vector<std::string> &instructions, int redirect, int pipe_needed, int run_in_background) {
    int cmd_exists = 0;
    char * exe_cmd[instructions.size() + 1];
    if (exists(instructions[0])) {
        if (pipe_needed != -1) {
            // We must parse the input accordingly
            std::tuple<std::vector<std::string>, std::vector<std::string>> pipe_instructions = get_pipe_instructions(instructions, pipe_needed);
            char * write_cmd[std::get<0>(pipe_instructions).size() + 1 ];
            char * read_cmd[std::get<1>(pipe_instructions).size() + 1 ];
            set_args(std::get<0>(pipe_instructions), write_cmd, "/");
            set_args(std::get<1>(pipe_instructions), read_cmd, "/");
            run_pipe_cmd(std::get<0>(pipe_instructions), std::get<1>(pipe_instructions), write_cmd, read_cmd, run_in_background);
        }else if (redirect != -1) {
            std::vector<std::string> updated_instructions = set_instructions(instructions, redirect);
            std::vector<std::string> output_file = set_output(instructions, redirect);
            set_args(updated_instructions, exe_cmd, "/");
            run_redirect_cmd(updated_instructions, output_file, exe_cmd, run_in_background);
        } else { // No special handling needed here
            set_args(instructions, exe_cmd, "/");
            general_cmd(instructions, exe_cmd, run_in_background);
        }
        cmd_exists = 1;
    }
    if (cmd_exists == 0) {
        std::cout << "dragonshell: Command not Found" << "\n";
    }
}

// Run relative path commands
void run_rel_cmd(std::vector<std::string> &instructions, int redirect, int pipe_needed, int run_in_background) {
    int cmd_exists = 0;
    char * exe_cmd[instructions.size() + 1];
    std::string s = instructions[0];
    if (exists(s)) { // This block checks if the file is in the current working dir
        if (pipe_needed != -1) {
            // We must parse the input accordingly
            std::tuple<std::vector<std::string>, std::vector<std::string>> pipe_instructions = get_pipe_instructions(instructions, pipe_needed);
            char * write_cmd[std::get<0>(pipe_instructions).size() + 1 ];
            char * read_cmd[std::get<1>(pipe_instructions).size() + 1 ];
            set_args(std::get<0>(pipe_instructions), write_cmd, "/");
            set_args(std::get<1>(pipe_instructions), read_cmd, "/");
            run_pipe_cmd(std::get<0>(pipe_instructions), std::get<1>(pipe_instructions), write_cmd, read_cmd, run_in_background);
        }
        else if (redirect != -1) {
            std::vector<std::string> updated_instructions = set_instructions(instructions, redirect);
            std::vector<std::string> output_file = set_output(instructions, redirect);
            set_args(updated_instructions, exe_cmd, "/");
            run_redirect_cmd(updated_instructions, output_file, exe_cmd, run_in_background);
        } else {
            set_args(instructions, exe_cmd, "/"); // Set args for the cmd
            general_cmd(instructions, exe_cmd, run_in_background);
        }
        cmd_exists = 1;
        return;
    }
    for (unsigned int i=0; i< PATH.size(); i++) { // This block compares the command against the PATH
        std::string temp_path = PATH[i];
        instructions[0] = temp_path.append(s);
        if (exists(instructions[0])) {
            if(pipe_needed != -1) {
                // We must parse the input accordingly
                std::tuple<std::vector<std::string>, std::vector<std::string>> pipe_instructions = get_pipe_instructions(instructions, pipe_needed);
                char * write_cmd[std::get<0>(pipe_instructions).size() + 1 ];
                char * read_cmd[std::get<1>(pipe_instructions).size() + 1 ];
                set_args(std::get<0>(pipe_instructions), write_cmd, "/");
                set_args(std::get<1>(pipe_instructions), read_cmd, "/");
                run_pipe_cmd(std::get<0>(pipe_instructions), std::get<1>(pipe_instructions), write_cmd, read_cmd, run_in_background);
            }
            else if (redirect != -1) {
                std::vector<std::string> updated_instructions = set_instructions(instructions, redirect);
                std::vector<std::string> output_file = set_output(instructions, redirect);
                set_args(updated_instructions, exe_cmd, "/");
                run_redirect_cmd(updated_instructions, output_file, exe_cmd, run_in_background);
            } else {
                set_args(instructions, exe_cmd, "/"); // Set args for the cmd
                general_cmd(instructions, exe_cmd, run_in_background);
            }
            cmd_exists = 1;
        };
    }
    if (cmd_exists == 0) {
        std::cout << "dragonshell: Command not Found" << "\n";
    }
}

// Check the path that was given by the user
void checkPATH(std::vector<std::string> &instructions) {
    int run_in_background = isBackgroundProcess(instructions);
    std::tuple<int, std::vector<std::string>> tup = needsOutputRedirect(instructions, ">"); // This checks if the command asks for a redirect
    int pipe_needed = needsPipe(instructions);
    // Next we will ask the user if a pipe is needed
    if (absPath(instructions)) { // Run a general command if no redirect needed
        run_abs_cmd(std::get<1>(tup), std::get<0>(tup), pipe_needed, run_in_background);
    } else if (relPath(instructions)) { // Run a general command if no redirect needed
        run_rel_cmd(std::get<1>(tup), std::get<0>(tup), pipe_needed, run_in_background);
    }
}


// DRIVER FOR THE EXECUTION STEP
void executeInstructions(std::vector<std::string> &instructions) {
    // Handle enter case with nothing???
    if (instructions[0] == "cd") {
        cd(instructions[1]);
    } else if (instructions[0] == "pwd"){
        std::tuple<int, std::vector<std::string>> tup = needsOutputRedirect(instructions, ">"); // Handle redirects
        // HANDLE PIPES
        pwd(std::get<0>(tup), std::get<1>(tup));
    } else if (instructions[0] == "$PATH") {
        std::tuple<int, std::vector<std::string>> tup = needsOutputRedirect(instructions, ">"); // Handle redirects
        // Handle PIPEs
        show$PATH(std::get<0>(tup), std::get<1>(tup));
    }else if (instructions[0] == "a2path"){
        appendToPath(instructions);
    }
    else if (instructions[0] == "exit") {
        exitDragonShell();
    } else if (instructions.size() == 0) {
        // Case where nothing is in the vector
        return;
    }
    else {
        // Check if the command exists in the PATH or in cwd, else command will not be found
        checkPATH(instructions);
    }
}