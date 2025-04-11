#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

/* CONST VARS */
const int max_line = 1024;
const int max_commands = 10;
#define max_redirections 3 //stdin, stdout, stderr
#define max_args 15
#define READ  0  // Flag for pipe read
#define WRITE 1  // Flag for pipe write


/* VARS TO BE USED FOR THE STUDENTS */
char * argvv[max_args];
char * filev[max_redirections];
int background = 0; // 0 -> a process runs in foreground (primer plano).  1 -> a process runs in background

/**
 * This function splits a char* line into different tokens based on a given character
 * @return Number of tokens 
 */
int tokenizar_linea(char *linea, char *delim, char *tokens[], int max_tokens) {
    int i = 0;
    char *token = strtok(linea, delim);
    while (token != NULL && i < max_tokens - 1) {
        tokens[i++] = token;
        token = strtok(NULL, delim);
    }
    tokens[i] = NULL;
    return i;
}

/**
 * This function processes the command line to evaluate if there are redirections. 
 * If any redirection is detected, the destination file is indicated in filev[i] array.
 * filev[0] for STDIN
 * filev[1] for STDOUT
 * filev[2] for STDERR
 */
void procesar_redirecciones(char *args[]) {
    // initialization for every command
    filev[0] = NULL;
    filev[1] = NULL;
    filev[2] = NULL;
    // Store the pointer to the filename if needed.
    // args[i] set to NULL once redirection is processed
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            filev[0] = args[i+1];
            args[i] = NULL;
            args[i + 1] = NULL;
            i++;
        } else if (strcmp(args[i], ">") == 0) {
            filev[1] = args[i+1];
            args[i] = NULL;
            args[i + 1] = NULL;
            i++;
        } else if (strcmp(args[i], "!>") == 0) {
            filev[2] = args[i+1];
            args[i] = NULL; 
            args[i + 1] = NULL;
            i++;
        }
    }
}

/**
 * This function processes the input command line and returns in global variables: 
 * argvv -- command an args as argv 
 * filev -- files for redirections. NULL value means no redirection. 
 * background -- 0 means foreground; 1 background.
 */
int procesar_linea(char *linea) { 
    char *comandos[max_commands];
    int num_comandos = tokenizar_linea(linea, "|", comandos, max_commands);

    background = 0;  // Reset background flag for each line

    // Check if background is indicated
    if (num_comandos > 0 && strchr(comandos[num_comandos - 1], '&')) {
        background = 1;
        char *pos = strchr(comandos[num_comandos - 1], '&'); 
        //remove character 
        *pos = '\0';
    }

    // totalPipes is a 2D array to store the file descriptors for the pipes
    int totalPipes[num_comandos-1][2]; // 2 porque puede ser READ (0) o WRITE (1)
    pid_t pid;

    // Create all pipes that will be used
    for (int i = 0; i < num_comandos - 1; i++){
        if (pipe(totalPipes[i]) < 0){
            perror("Error creating the pipes");
            exit(EXIT_FAILURE); // Used when a pipe creation process has failed
        }
    }

    // Finish processing
    for (int i = 0; i < num_comandos; i++) {
        int args_count = tokenizar_linea(comandos[i], " \t\n", argvv, max_args); // Tokenize (divide) each command and store in argvv
        procesar_redirecciones(argvv); // We search for redirection operators. Redirections are used to change the input or output of a command
                                       // We need to 'process' the redirections so later we can redirect the input/output of the command

        // Check if the command is "echo"
        if (argvv[0] != NULL && strcmp(argvv[0], "echo") == 0) {
            // Loop through all arguments after the command name
            for (int j = 1; j < args_count; j++) {
                if (argvv[j] != NULL) {
                    // Remove quotes if present
                    char *argument = argvv[j]; // Pointer to the argument
                    if ((argument[0] == '"' && argument[strlen(argument) - 1] == '"') || (argument[0] == '\'' && argument[strlen(argument) - 1] == '\'')) {
                        argument[strlen(argument) - 1] = '\0'; // Remove trailing quote
                        argument++; // Skip leading quote
                        argvv[j] = argument; // Update xthe argument in argvv
                    }
                }
            }
        }

        pid = fork(); // Creation of child process for each command
        
        switch (pid){
            case -1: // Error in fork
                perror("Fork failed");
                exit(EXIT_FAILURE);
            break;

            case 0:  // Child process
                if (num_comandos > 1){ // We need to check if there is more than one command so we can set the pipes between them
                    if (i == 0) { // First command
                        if (dup2(totalPipes[i][WRITE], STDOUT_FILENO) == -1) { // Redirect STDOUT to the first pipe
                            perror("Error duplicating pipe for STDOUT (first command)");
                            exit(EXIT_FAILURE);
                        }
                        close(totalPipes[i][WRITE]) == -1; // Close the WRITE end after writing
                    } else if (i == num_comandos - 1) { // Last command
                        if (dup2(totalPipes[i - 1][READ], STDIN_FILENO) == -1) { // Redirect STDIN to the last pipe
                            perror("Error duplicating pipe for STDIN (last command)");
                            exit(EXIT_FAILURE);
                        }
                        close(totalPipes[i - 1][READ]) == -1; // Close the READ end after reading
                    } else { // Middle commands
                        // IF ITS NOT THE FIRST COMMAND, set STDIN (standard input) to read from the previous pipe
                        // IF IT'S NOT THE LAST COMMAND, set STDOUT to write to the NEXT Pipe
                        if (dup2(totalPipes[i - 1][READ], STDIN_FILENO) == -1) { // dup2 makes STDIN to read from totalPipes[i-1] (previous pipe's READ end)
                            perror("Error duplicating pipe for STDIN (middle command)");
                            exit(EXIT_FAILURE);
                        }
                        if (dup2(totalPipes[i][WRITE], STDOUT_FILENO) == -1) { // //dup2 makes STDOUT to WRITE to totalPipes[i] (CURRENT pipe's WRITE end)
                            perror("Error duplicating pipe for STDOUT (middle command)");
                            exit(EXIT_FAILURE);
                        }
                        close(totalPipes[i][WRITE]) == -1; // Close the WRITE end after writing
                        close(totalPipes[i - 1][READ]) == -1; // Close the READ end after reading
                    }

                    // In each child process, close the pipe ends that it doesn't need
                    for (int j = 0; j < num_comandos - 1; j++) { 
                        close(totalPipes[j][READ]);  // Close the read end of pipes the child does not use
                        close(totalPipes[j][WRITE]); // Close the write end of pipes the child does not use
                    }
                }
                
                // Handle redirections (input, output, error)

                // filev[0] --> stores INPUT redirection file (< filename)
                if (filev[0]) { // If there exists an INPUT redirection file...
                    int finput = open(filev[0], O_RDONLY); // OPEN THAT FILE, only for reading, because it's an input
                    if (finput < 0) {
                        perror("Error opening input file");
                        exit(EXIT_FAILURE);
                    }
                    if (dup2(finput, STDIN_FILENO) == -1) { // The funcion 'dup2' in this case redirects the standard input (STDIN) to come from the file 'finput' instead of keyboard
                        perror("Error duplicating input redirection");
                        exit(EXIT_FAILURE);
                    }
                    close(finput); // Close the file descriptor
                }

                // filev[1] --> stores OUTPUT redirection files (> filename)
                if (filev[1]) { // If there exists an OUTPUT redirection file...
                    int foutput = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0644); // OPEN THAT FILE (flags of writing, because it's an output)
                    if (foutput < 0) {
                        perror("Error opening output file"); 
                        exit(EXIT_FAILURE); 
                    }
                    if (dup2(foutput, STDOUT_FILENO) == -1) { // The funcion 'dup2' in this case redirects the standard output (STDOUT) to come from the file 'foutput' instead of keyboard
                        perror("Error duplicating output redirection");
                        exit(EXIT_FAILURE);
                    }
                    close(foutput); // Close the file descriptor
                }   

                // filev[2] --> stores ERROR redirection files (!> filename)
                if (filev[2]) { // If there exists an ERROR redirection file...
                    int ferror = open(filev[2], O_WRONLY | O_CREAT | O_TRUNC, 0644); // OPEN THAT FILE (flags of writing, in that file the errors will be written)
                    if (ferror < 0) {
                        perror("Error opening error file");
                        exit(EXIT_FAILURE); 
                    }
                    if (dup2(ferror, STDERR_FILENO) == -1) { // The funcion 'dup2' in this case redirects the standard error (STDERR) to come from a file 'ferror' instead of keyboard
                        perror("Error duplicating error redirection");
                        exit(EXIT_FAILURE);
                    }
                    close(ferror); // Close the file descriptor
                }
            
                // In argvv[0] we have the command to execute. argvv is the array that contains the arguments for argvv[i].
                if (execvp(argvv[0], argvv) == -1) { // Execute the command
                    perror("Exec failed");
                    exit(EXIT_FAILURE);
                }
                
                break;

            default: // Parent process

            // Reason of i == num_comandos -1:
            // As the statement of the lab says, "when a sequence of commands is elxecuted in the background, the pid that is printed is
            // that of the process executing the last command in the sequence"
                if (background == 1 && i == num_comandos - 1) { // If background has been indicated in the line, we have to wait for the child to end. 
                    printf("%d", pid); // The parent process must print via standard output the pid of the child process.
                }
        }
    }

    // The PARENT process also needs to close all pipes!! The  parent is the one that creates the pipes, but it will not be directly using them! 
    for (int i = 0; i < num_comandos - 1; i++) {
        close(totalPipes[i][READ]);
        close(totalPipes[i][WRITE]);
    }


    // After executing a command in the foreground, the interpreter cannot have zombie
    //processes from previous commands executed in background

    // Clean up child processes. Wait for any child process to terminate. The parent waits for ALL childs to terminate
    if (background == 0) { // 0 -> a process runs in foreground (primer plano).  1 -> a process runs in background
        for (int i = 0; i < num_comandos; i++) { // Wait for all child processes
            waitpid(pid, NULL, 0); // The parent process waits for the child process to finish (if child is foreground)
        }
    }

    return num_comandos;
}

int main(int argc, char *argv[]) {
    // Check the number of arguments
    if (argc != 2) {
        perror("Error: incorrect number of arguments. Usage: ./scripter <script_file_with_commands>");
        exit(-1);
    }

    // Open the file
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Error opening the file");
        exit(-1);
    }

    // Read the file

    char line[max_line]; // Array to store each line
    char buffer; // Buffer to get each character
    size_t i = 0; // Unsinged integer. It's better for buffer indexing since it avoids negative values
    ssize_t nread = -1;
    char ssoo[] = "## Script de SSOO";
    int checkSSOO = 1;

    // Read lines until a jump of line is found, that is when we add '\0' instead of '\n'
    while ((nread = read(fd, &buffer, 1)) > 0) {
        // If an end of line is found or we reach the max_line...
        if (buffer == '\n' || i == max_line - 1) {
            // We add '\0' to indicate the end of the line
            line[i] = '\0';

            if (line[0] == '\0'){
                perror("There is an empty line!");
                close(fd);
                exit(-1);
            }

            if (checkSSOO == 1) {
                if (strcmp(ssoo, line) != 0){ // Check if the first line is "## Script de SSOO"
                    perror("The first line is not \"## Script de SSOO\"");
                    close(fd);
                    exit(-1);
                }
                checkSSOO = 0;
            } else {
                // Process the line
                procesar_linea(line);
            }

            i = 0; // Reset the index for the next line

        } else {
            // Store the character in the line buffer
            line[i] = buffer;
            i++; // Points to the next available position
        }
    }
    
    // In case there has been an error reading
    if (nread < 0){
        perror("An error has occurred reading the file!");
        close(fd);
        exit(-1);
    }

    // Process the last line if the file does not end with '\n'
    if (i > 0) {
        line[i] = '\0';
        procesar_linea(line);
    }

    // Close the file descriptor
    close(fd);

    return 0;
}