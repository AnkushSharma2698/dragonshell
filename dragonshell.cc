#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "execution_handler.h"

/**
 * @brief Tokenize a string 
 * 
 * @param str - The string to tokenize
 * @param delim - The string containing delimiter character(s)
 * @return std::vector<std::string> - The list of tokenized strings. Can be empty
 */

void sighandler(int signum);

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

// print method for a vector < -- Make sure to delete this code once you are done
void print(std::vector<std::string> const &input) {
    for (int i = 0; i < input.size(); i++) {
        std::cout << i;
        std::cout << input.at(i) << ' ';
    }
    std::cout << '\n';
}

void execute(std::vector<std::string> &command) {
    // Since there can be multiple commands, we will loop through them and tokenize them individually
    for (int i = 0; i < static_cast<int>(command.size());i++) {
        std::vector<std::string> instruction = tokenize(command[i], " ");
        executeInstructions(instruction);
    }
}

void sighandler(int signum) {
    std::cout << "hadnling control c";

}

// Driver of the code
int main(int argc, char **argv) {
    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&(sa.sa_mask));
    sa.sa_handler = sighandler;
    sigaction(SIGINT, &sa, NULL); // CTRL+C handling
    // TODO add ctrl + z handling

    std::cout << "Welcome to Dragon Shell! \n";
    std::string command;
    std::cout << "dragonshell > ";
    while (getline(std::cin, command)) {
        std::vector<std::string> tokenized_command = tokenize(command, ";");
        execute(tokenized_command);
        std::cout << "dragonshell > ";
    }
    std::cout << "\n"; // Formatting
    exitDragonShell();
}
