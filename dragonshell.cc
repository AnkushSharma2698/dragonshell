#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <csetjmp>
#include "execution_handler.h"

/**
 * @brief Tokenize a string 
 * 
 * @param str - The string to tokenize
 * @param delim - The string containing delimiter character(s)
 * @return std::vector<std::string> - The list of tokenized strings. Can be empty
 */

void sighandler(int signum);
pid_t dragonshell_pid = getpid();
sigjmp_buf looper;

std::vector<std::string> tokenize(const std::string &str, const char *delim) {
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

void execute(std::vector<std::string> &command) {
    // Since there can be multiple commands, we will loop through them and tokenize them individually
    for (int i = 0; i < static_cast<int>(command.size());i++) {
        std::vector<std::string> instruction = tokenize(command[i], " ");
        executeInstructions(instruction);
    }
}

void sighandler(int signum) {
//    killBackgroundProcesses();
    if (getpid() != dragonshell_pid) {
        kill(getpid(), signum);
    }
    siglongjmp(looper, -1);
}

void mainLoop() {
    std::string command;
    while (getline(std::cin, command)) {
        std::vector<std::string> tokenized_command = tokenize(command, ";");
        execute(tokenized_command);
        std::cout << "dragonshell > ";
    }
    std::cout << "\n";
    exitDragonShell();
}

// Driver of the code
int main(int argc, char **argv) {
    struct sigaction s;
    s.sa_handler = sighandler;
    sigemptyset(&s.sa_mask);
    s.sa_flags=0;
    sigaction(SIGINT, &s, 0);
    sigaction(SIGTSTP, &s, 0);

    std::cout << "Welcome to Dragon Shell! \n";
    std::cout << "dragonshell > ";
    if (sigsetjmp(looper, 1) == -1) {
        std::cout << "\ndragonshell > ";
        mainLoop();
        _exit(1);
    }
    mainLoop();
}
