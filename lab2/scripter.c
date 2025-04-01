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
int totalPipes[9][2]; // max 10 commands (9 pipes) y 2 porque puede ser READ o WRITE

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
        } else if (strcmp(args[i], ">") == 0) {
            filev[1] = args[i+1];
            args[i] = NULL;
            args[i + 1] = NULL;
        } else if (strcmp(args[i], "!>") == 0) {
            filev[2] = args[i+1];
            args[i] = NULL; 
            args[i + 1] = NULL;
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

    //Check if background is indicated
    if (strchr(comandos[num_comandos - 1], '&')) {
        background = 1;
        char *pos = strchr(comandos[num_comandos - 1], '&'); 
        //remove character 
        *pos = '\0';
    }

    // Create all pipes that will be used
    for (int i = 0; i < num_comandos - 1; i++){
        if (pipe(totalPipes[i] < 0)){
            perror("Error creating the pipes");
            exit(EXIT_FAILURE); // Used when a pipe creation process has failed
        }
    }

    //Finish processing
    for (int i = 0; i < num_comandos; i++) {
        int args_count = tokenizar_linea(comandos[i], " \t\n", argvv, max_args); // Tokenize (divide) each command and store in argvv
        procesar_redirecciones(argvv);

        // QUITAR ESTO LUEGO

        /********* This piece of code prints the command, args, redirections and background. **********/
        /*********************** REMOVE BEFORE THE SUBMISSION *****************************************/
        /*********************** IMPLEMENT YOUR CODE FOR PROCESSES MANAGEMENT HERE ********************/
        printf("Comando = %s\n", argvv[0]);
        for(int arg = 1; arg < max_args; arg++)
            if(argvv[arg] != NULL)
                printf("Args = %s\n", argvv[arg]); 
                
        printf("Background = %d\n", background);
        if(filev[0] != NULL)
            printf("Redir [IN] = %s\n", filev[0]);
        if(filev[1] != NULL)    
            printf("Redir [OUT] = %s\n", filev[1]);
        if(filev[2] != NULL)
            printf("Redir [ERR] = %s\n", filev[2]);
        /**********************************************************************************************/

        // QUITAR ESTO LUEGO
    }

    for (int i = 0; i < num_comandos; i++) { // EACH COMMAND RUNS IN A SEPARATE CHILD PROCESS

        // Check if the last argument is '&' (if it is, it will be a background process)
        if (strcmp(argvv[i][num_comandos - 1], "&") == 0) { // Check if the last command is '&'
            background = 1; // IT WILL BE A BACKGROUND PROCESS.
            argvv[i][num_comandos - 1] = NULL;  // Remove the '&' from arguments, preventing it to be processed by the following command
        }


        pid_t pid = fork(); // Creation of child process for each command
        if (pid == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    
        if (pid == 0) { // Child process
            if (i > 0) { // IF ITS NOT THE FIRST COMMAND, set STDIN (standard input) to read from the previous pipe
                dup2(totalPipes[i - 1][READ], STDIN_FILENO); // dup2 makes STDIN to read from totalPipes[i-1] (previous pipe's READ end)
                close(totalPipes[i - 1][READ]); // Close the READ end after reading
            }
            if (i < num_comandos - 1) { // IF IT'S NOT THE LAST COMMAND, set STDOUT to write to the NEXT Pipe
                dup2(totalPipes[i][WRITE], STDOUT_FILENO); //dup2 makes STDOUT to WRITE to totalPipes[i] (CURRENT pipe's WRITE end)
                close(totalPipes[i][WRITE]); // Close the WRITE after writing
            }
    
            // filev[0] --> stores INPUT redirection file (< filename)
            if (filev[0]) { // If there exists an INPUT redirection file...
                int fd = open(filev[0], O_RDONLY); // OPEN THAT FILE, only for reading, because it's an input
                if (fd < 0) {
                    perror("Error opening input file");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO); // The funcion 'dup2' in this case redirects the standard input (STDIN) to come from the file 'fd' instead of keyboard
                close(fd);
            }

            // filev[1] --> stores OUTPUT redirection files (> filename)
            if (filev[1]) { // If there exists an OUTPUT redirection file...
                int fd = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0644); // OPEN THAT FILE (flags of writing, because it's an output)
                if (fd < 0) {
                    perror("Error opening output file"); 
                    exit(EXIT_FAILURE); 
                }
                dup2(fd, STDOUT_FILENO); // The funcion 'dup2' in this case redirects the standard output (STDOUT) to come from the file 'fd' instead of keyboard
                close(fd);
            }

            // filev[2] --> stores ERROR redirection files (!> filename)
            if (filev[2]) { // If there exists an ERROR redirection file...
                int fd = open(filev[2], O_WRONLY | O_CREAT | O_TRUNC, 0644); // OPEN THAT FILE (flags of writing, in that file the errors will be written)
                if (fd < 0) {
                    perror("Error opening error file");
                    exit(EXIT_FAILURE); 
                }
                dup2(fd, STDERR_FILENO); // The funcion 'dup2' in this case redirects the standard error (STDERR) to come from a file instead of keyboard
                close(fd);
            }

            for (int i = 0; i < num_comandos - 1; i++) { // Close all pipes inside the CHILD process to ensure that the child only interacts with its own required pipe
                close(totalPipes[i][READ]);              // NOTE that each child only uses one end of one pipe:
                close(totalPipes[i][WRITE]);             // A command reading from STDIN:  should use a READ pipe.
                                                         // A command writing to STDOUT: should use a WRITE pipe.
            }

            execvp(argvv[i], argvv); // argvv[i] is the command to execute (in our case, the i-th command in the PIPELINE!!). argvv is the array that contains the arguments for argvv[i].
            perror("Exec failed"); // In case the execution of the command fails
            exit(EXIT_FAILURE);
        }

        if (background == 0) { // 0 -> a process runs in foreground (primer plano).  1 -> a process runs in background
            waitpid(pid, NULL, 0); // The parent process waits for the child process to finish.
        } else {
            printf("Background process started with PID: %d\n", pid);
        }
    }

    for (int i = 0; i < num_comandos - 1; i++) { // The PARENT process also needs to close all pipes!! The parenthe parent is the one that creates the pipes, but it will not be directly using them! 
        close(totalPipes[i][READ]);
        close(totalPipes[i][WRITE]);
    }

    return num_comandos;
}

int main(int argc, char *argv[]) {

    /* STUDENTS CODE MUST BE HERE */
    char example_line[] = "ls -l | grep scripter | wc -l > redir_out.txt &";
    int n_commands = procesar_linea(example_line);
    
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

    char buffer[max_line]; // Buffer
    char c; // pointer to get each character
    size_t i = 0; // unsinged integer. It's better for buffer indexing since it avoids negative values
    ssize_t nread = -1;
    char ssoo[] = "## Script de SSOO";
    int checkSSOO = 1;

    
    while ((nread = read(fd, &c, 1)) > 0) {
        if (c == '\n') {
            buffer[i] = '\0';

            if (buffer[0] == '\0'){
                perror("There is an empty line!");
                close(fd);
                exit(-1);
            }

            if (checkSSOO == 1) {
                if (strcmp(ssoo, buffer) != 0){ // Check if the first line is "## Script de SSOO"
                    perror("The first line is not \"## Script de SSOO\"");
                    close(fd);
                    exit(-1);
                }
                checkSSOO = 0;
            } else {
                procesar_linea(buffer); // Only passed until the first \0
            }

            i = 0; // Reset the index for the next line

        } else {
            buffer[i] = c;
            i++; // points to the next available position in the buffer
        }
    }
        
    // In case there has been an error reading
    if (nread < 0){
        perror("An error has occurred reading the file!");
        free(buffer);
        close(fd);
        exit(-1);
    }

    return 0;
}