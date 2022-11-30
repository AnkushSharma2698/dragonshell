
Features:
- cd:
    -  In order to implement the change directory functionality I used
    the system call chdir(). I tested this implementation using a variety of passed paths that are 
    possible in the linux shell including test cases such as going back a directory, or an exact path, and relative paths
- pwd:
    - To implement this I used the getcwd() command.
- a2path:
    -  In order to implement this feature I used a global variable to store the PATH variable. Whenever a user states a
    path following this command it overwrites the PATH or is appended to the PATH.
- exit:
    - This feature checks for user input matching "exit" and when found, it kills all child processes and then used the
    system call _exit to terminate dragonshell.
- Running External Programs:
    - To implement running external commands, when user input is given, it is passed to the checkPath() function which
    then checks if the command is an absolute path or relative path. Once decided, it runs either the run_abs_cmd() or 
    run_rel_cmd() depending on the type of command that is given. Following this a child process is made using fork()
    and execve() is then used to run the given command.
- Running Multiple Commands:
    - Tokenized user input based on ";". This created a vector of commands that I then iterate over to run commands 
    consecutively.
- Support Background Execution
    - In order to handle background execution, I checked the user input for the existence of an "&" and based on that I
    would run the command in the same manner as I did for running external programs, but I closed STDOUT and STDERR, and 
    the parent no longer waits for that process to complete; simply resuming dragonshell execution.
- Support Redirecting Output of Program to File
    - Checked for the existence of a ">" in the user input. If it exists, I tokenized the user input based on ">". Before 
    beginning execution of the process, I specified the output_file and used the system call dup2() to send the STDOUT to 
    the specified file. Once this was complete I performed the execve() call.
- Support Piping Output of on program to another program
    - Checked for the existence of a "|" in the user input. If it exists, I tokenized the user input based on "|". Before 
      beginning execution of the process, I specify the read and write pipe destinations by splitting the input. To begin
      the pipe process, I use fork() to create a child of the parent process, then I fork() the child that was created.
      I wrote all STDOUT to the child of the initial process, and all STDIN to the child of the parent process. To implement the 
      pipe I used the pipe() system call, dup2() system call and close() system call, as well as execve() for execution.
- Handle Signals
    - To handle signals I utilized sigaction(). The handler that I implemented handles both SIGINT(CTRL+C) as well as 
    SIGTSTP(CTRL+Z). Implementation details: When one of the keyboard interrupts is detected, kill any process running in the foreground,
    excluding dragonshell itself; Then return to the main while loop to continue dragonshell.
    
