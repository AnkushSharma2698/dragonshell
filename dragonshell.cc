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

// print method for a vector
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
        std::vector<std::string> instruction = tokenize(command[i], "| ");
        executeInstructions(instruction);
    }
}

// Driver of the code
int main(int argc, char **argv) {
  std::cout << "Welcome to Dragon Shell! \n";

  while (true) {
      std::string command;
      std::cout << "dragonshell > ";

      // Get user input
      getline(std::cin, command);

      // Parse the input and send it to check if it is a valid command
      std::vector<std::string> tokenized_command = tokenize(command, ";");
      execute(tokenized_command);
  }
}