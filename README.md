
Custom Command Shell
This is a custom command shell developed as a solution to the problem statement provided. The shell is implemented using OS system calls to execute built-in Linux commands. It supports various features including basic command execution, changing directories, handling incorrect commands, signal handling, executing multiple commands (sequential and parallel), and output redirection.

Features
1. Basic Command Execution
The shell runs in an infinite loop, which can be exited with the exit command.
It interactively processes user commands.
The shell displays a prompt that indicates the current working directory followed by the '$' character.
Input is read using the getline() function.
Multiple words in a command (e.g., multiple arguments) are separated using the strsep() function.
2. Changing Directory
The shell supports the cd command.
cd <directoryPath> changes the working directory to directoryPath.
cd .. changes the working directory to the parent directory.
chdir() system call is used for implementing this feature.
3. Handling Incorrect Commands
When an incorrect command format is entered, the shell prints the error message: "Shell: Incorrect command."
If a command is executed but results in error messages, those error messages are displayed.
4. Signal Handling
The shell can handle signals generated from the keyboard using 'Ctrl + C' and 'Ctrl + Z'.
It continues to work even when these signals are generated.
Commands being executed by the shell respond to these signals.
The shell only stops with the 'exit' command.
5. Executing Multiple Commands
The shell supports multiple command execution:
Commands separated by '&&' are executed in parallel.
Commands separated by '##' are executed sequentially.
The shell waits for all commands to terminate before accepting further inputs.
6. Output Redirection
The shell can redirect STDOUT for commands using the '>' symbol.
For example, ls > info.out writes the output of the 'ls' command to the 'info.out' file instead of displaying it on the screen.
