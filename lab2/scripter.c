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
#define READ  0  // Pipe read
#define WRITE 1  // Pipe write 


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
    //initialization for every command
    filev[0] = NULL;
    filev[1] = NULL;
    filev[2] = NULL;
    //Store the pointer to the filename if needed.
    //args[i] set to NULL once redirection is processed
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

    background = 0;  // Reset background flag for each line (added by us). RESET background flag for each line

    //Check if background is indicated
    if (num_comandos > 0 && strchr(comandos[num_comandos - 1], '&')) {
        background = 1;
        char *pos = strchr(comandos[num_comandos - 1], '&'); 
        //remove character 
        *pos = '\0';
    }

    int totalPipes[num_comandos-1][2]; // 2 porque puede ser READ o WRITE
    pid_t pid;

    // Create all pipes that will be used
    for (int i = 0; i < num_comandos - 1; i++){
        if (pipe(totalPipes[i]) < 0){
            perror("Error creating the pipes");
            exit(EXIT_FAILURE); // Used when a pipe creation process has failed
        }
    }

    //Finish processing
    for (int i = 0; i < num_comandos; i++) {
        int args_count = tokenizar_linea(comandos[i], " \t\n", argvv, max_args); // Tokenize (divide) each command and store in argvv
        procesar_redirecciones(argvv);

        // Check if the command is "echo"
        if (argvv[0] != NULL && strcmp(argvv[0], "echo") == 0) {
            // Loop through all arguments after the command name
            for (int j = 1; j < args_count; j++) {
                if (argvv[j] != NULL) {
                    // Remove quotes if present
                    char *argument = argvv[j];
                    if ((argument[0] == '"' && argument[strlen(argument) - 1] == '"') || (argument[0] == '\'' && argument[strlen(argument) - 1] == '\'')) {
                        argument[strlen(argument) - 1] = '\0'; // Remove trailing quote
                        argument++; // Skip leading quote
                        argvv[j] = argument; // Update the argument in argvv
                    }
                }
            }
        }

        pid = fork(); // Creation of child process for each command
        
        switch (pid){
        case -1:
            perror("Fork failed");
            exit(EXIT_FAILURE);
            break;

        case 0:  // Child process
            if (num_comandos > 1){
                if (i == 0) { // First command
                    dup2(totalPipes[i][WRITE], STDOUT_FILENO); // The first command only writes to the first pipe
                    close(totalPipes[i][WRITE]); // Close the WRITE after writing
                } else if (i == num_comandos - 1) { // Last command
                    dup2(totalPipes[i - 1][READ], STDIN_FILENO); // The last command only reads from the last pipe
                    close(totalPipes[i - 1][READ]); // Close the READ end after reading
                } else { // Middle commands
                    // IF ITS NOT THE FIRST COMMAND, set STDIN (standard input) to read from the previous pipe
                    // IF IT'S NOT THE LAST COMMAND, set STDOUT to write to the NEXT Pipe
                    dup2(totalPipes[i - 1][READ], STDIN_FILENO); // dup2 makes STDIN to read from totalPipes[i-1] (previous pipe's READ end)
                    dup2(totalPipes[i][WRITE], STDOUT_FILENO); //dup2 makes STDOUT to WRITE to totalPipes[i] (CURRENT pipe's WRITE end)
                    close(totalPipes[i][WRITE]); // Close the WRITE after writing
                    close(totalPipes[i - 1][READ]); // Close the READ end after reading
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
                dup2(finput, STDIN_FILENO); // The funcion 'dup2' in this case redirects the standard input (STDIN) to come from the file 'fd' instead of keyboard
                close(finput);
            }

            // filev[1] --> stores OUTPUT redirection files (> filename)
            if (filev[1]) { // If there exists an OUTPUT redirection file...
                int foutput = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0644); // OPEN THAT FILE (flags of writing, because it's an output)
                if (foutput < 0) {
                    perror("Error opening output file"); 
                    exit(EXIT_FAILURE); 
                }
                dup2(foutput, STDOUT_FILENO); // The funcion 'dup2' in this case redirects the standard output (STDOUT) to come from the file 'fd' instead of keyboard
                close(foutput);
            }

            // filev[2] --> stores ERROR redirection files (!> filename)
            if (filev[2]) { // If there exists an ERROR redirection file...
                int ferror = open(filev[2], O_WRONLY | O_CREAT | O_TRUNC, 0644); // OPEN THAT FILE (flags of writing, in that file the errors will be written)
                if (ferror < 0) {
                    perror("Error opening error file");
                    exit(EXIT_FAILURE); 
                }
                dup2(ferror, STDERR_FILENO); // The funcion 'dup2' in this case redirects the standard error (STDERR) to come from a file instead of keyboard
                close(ferror);
            }

            for (int j = 0; j < num_comandos - 1; j++) { // Close all pipes inside the CHILD process to ensure that the child only interacts with its own required pipe
                close(totalPipes[j][READ]);              // NOTE that each child only uses one end of one pipe:
                close(totalPipes[j][WRITE]);             // A command reading from STDIN:  should use a READ pipe.
                                                         // A command writing to STDOUT: should use a WRITE pipe.
            }

            execvp(argvv[0], argvv);
            perror("Exec failed");
            exit(EXIT_FAILURE);

            //execvp(argvv[0], argvv); // argvv[i] is the command to execute (in our case, the i-th command in the PIPELINE!!). argvv is the array that contains the arguments for argvv[i].
            //perror("Exec failed"); // In case the execution of the command fails
            //exit(EXIT_FAILURE);
        break;

        default: // Parent process (REMEMBER TO TALK ABOUT THE PIPELINE IN THE REPORT!!!!!)

        // Reason of i == num_comandos -1:
        // When a sequence of commands is elxecuted in the background, the pid that is printed is
        // that of the process executing the last command in the sequence
            if (background == 1 && i == num_comandos - 1) { // If background has been indicated in the line, we have to wait for the child to end. 
                printf("%d", pid); // The parent process must print via standard output the pid of the child process.
            }
            break;
        }
    }

    for (int i = 0; i < num_comandos - 1; i++) { // The PARENT process also needs to close all pipes!! The parenthe parent is the one that creates the pipes, but it will not be directly using them! 
        close(totalPipes[i][READ]);
        close(totalPipes[i][WRITE]);
    }

    // After executing a command in the foreground, the interpreter cannot have zombie
    //processes from previous commands executed in background

    if (background == 0) { // 0 -> a process runs in foreground (primer plano).  1 -> a process runs in background
        for (int i = 0; i < num_comandos; i++) { // Wait for all child processes
            waitpid(pid, NULL, 0); // The parent process waits for the child process to finish (if child is foreground)
        }
    }

    // Clean up child processes (wait for any child that has finished)
    //if (background == 0) {
    //    // Parent process waits for all children to terminate if no background processes
    //    for (int i = 0; i < num_comandos; i++) { // Wait for all child processes
    //        waitpid(pid, NULL, 0);  // Wait for any child process to terminate. PID '-1' is ANY child. The parent waits for ALL childs to terminate
    //    }
    //}

    return num_comandos;
}

int main(int argc, char *argv[]) {

    /* STUDENTS CODE MUST BE HERE */
    //char example_line[] = "ls -l | grep scripter | wc -l > redir_out.txt &";
    //int n_commands = procesar_linea(example_line);
    
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

    char line[max_line]; // Buffer
    char buffer; // pointer to get each character
    size_t i = 0; // unsinged integer. It's better for buffer indexing since it avoids negative values
    ssize_t nread = -1;
    char ssoo[] = "## Script de SSOO";
    int checkSSOO = 1;

    
    while ((nread = read(fd, &buffer, 1)) > 0) {

        if (buffer == '\n' || i == max_line - 1) {
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
                procesar_linea(line); // Only passed until the first \0
            }

            i = 0; // Reset the index for the next line

        } else {
            line[i] = buffer;
            i++; // points to the next available position in the buffer
        }
    }
    
    if (i > 0) {    //The last line may not finish in "\n"
        line[i] = '\0';
        procesar_linea(line);
    }

    // In case there has been an error reading
    if (nread < 0){
        perror("An error has occurred reading the file!");
        close(fd);
        exit(-1);
    }

    close(fd);

    return 0;
}