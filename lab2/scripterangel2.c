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
int num_back = 0; 
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

void execute_command(int i, int num_comandos) {
    // Set up pipes for the command
    if (num_comandos != 1){
        if (i > 0) { // IF ITS NOT THE FIRST COMMAND, set STDIN (standard input) to read from the previous pipe
            if(dup2(totalPipes[i-1][0], STDIN_FILENO) == -1){
            perror("Error duplicating pipe");

            for (int i = 0; i < num_comandos - 1; i++) { // The PARENT process also needs to close all pipes!! The parenthe parent is the one that creates the pipes, but it will not be directly using them! 
                close(totalPipes[i][0]);
                close(totalPipes[i][1]);
            }
            exit(EXIT_FAILURE);
            }
        }
    
        if (i < num_comandos - 1) { // IF IT'S NOT THE LAST COMMAND, set STDOUT to write to the NEXT Pipe
            if (dup2(totalPipes[i][1], STDOUT_FILENO )== -1){
                perror("Error duplicating pipe");

                for (int i = 0; i < num_comandos - 1; i++) { // The PARENT process also needs to close all pipes!! The parenthe parent is the one that creates the pipes, but it will not be directly using them! 
                    close(totalPipes[i][READ]);
                    close(totalPipes[i][WRITE]);
                }
                exit(EXIT_FAILURE);
            }   
        }

        for (int i = 0; i < num_comandos - 1; i++) { // The PARENT process also needs to close all pipes!! The parenthe parent is the one that creates the pipes, but it will not be directly using them! 
            close(totalPipes[i][READ]);
            close(totalPipes[i][WRITE]);
        }
    }
    
    // Handle redirections (input, output, error)

    // filev[0] --> stores INPUT redirection file (< filename)
    if (filev[0] && i == 0) { // If there exists an INPUT redirection file...
        int fin = open(filev[0], O_RDONLY); // OPEN THAT FILE, only for reading, because it's an input
        if (fin < 0) {
            perror("Error opening input file");
            exit(EXIT_FAILURE);
        } else {
            if (dup2(fin, STDIN_FILENO)==-1){
            perror("Error duplicating pipe");
            close(fin);
            exit(EXIT_FAILURE);
            }; // The funcion 'dup2' in this case redirects the standard input (STDIN) to come from the file 'fd' instead of keyboard
            close(fin);
        }
    }

    // filev[1] --> stores OUTPUT redirection files (> filename)
    if (filev[1] && i == num_comandos - 1) { // If there exists an OUTPUT redirection file...
        int fout = creat(filev[1], 0644); // OPEN THAT FILE (flags of writing, because it's an output)
        if (fout < 0) {
            perror("Error opening output file"); 
            exit(EXIT_FAILURE); 
        } else {
            if (dup2(fout, STDOUT_FILENO)==-1){
                perror("Error duplicating pipe");
                close(fout);
                exit(EXIT_FAILURE);
            }; // The funcion 'dup2' in this case redirects the standard input (STDIN) to come from the file 'fd' instead of keyboard
            
            close(fout);
        }
    }

    // filev[2] --> stores ERROR redirection files (!> filename)
    if (filev[2]) { // If there exists an ERROR redirection file...
        int ferr = creat(filev[2], 0644); // OPEN THAT FILE (flags of writing, in that file the errors will be written)
        if (ferr < 0) {
            perror("Error opening error file");
            exit(EXIT_FAILURE); 
        } else {
            if (dup2(ferr, STDERR_FILENO)==-1){
                perror("Error duplicating pipe");
                close(ferr);
                exit(EXIT_FAILURE);
            }; // The funcion 'dup2' in this case redirects the standard input (STDIN) to come from the file 'fd' instead of keyboard
            
            close(ferr);
        }
    }

    char mygrep[] = "mygrep";
    char mygrep2[] = "./mygrep";
    if(strcmp(argvv[0], mygrep) == 0){
        execvp(mygrep2, argvv);
        perror("Exec failed");
        exit(EXIT_FAILURE);
    }

    execvp(argvv[0], argvv);
    perror("Exec failed");
    exit(EXIT_FAILURE);
    //execvp(argvv[0], argvv); // argvv[i] is the command to execute (in our case, the i-th command in the PIPELINE!!). argvv is the array that contains the arguments for argvv[i].
    //perror("Exec failed"); // In case the execution of the command fails
    //exit(EXIT_FAILURE);
}


int procesar_linea(char* linea) {
    //background = 0; that is a global variable.
    char* comandos[max_commands]; // Array to store the commands on each line.
    int num_comandos = tokenizar_linea(linea, "|", comandos, max_commands); // Returns in the variable num_commands, the number of commands on each line. 

    //Check if background is indicated.
    if (strchr(comandos[num_comandos - 1], '&')) { // If at the end pf the command the symbol '&' is indicated, it will be run in the background.
        background = 1;
        char* pos = strchr(comandos[num_comandos - 1], '&');
        // Remove character & in order to execute it later correctly. 
        *pos = '\0'; // And add the \0 to indicate the end of a string. 
    }

    // Create all necessary pipes:
    for (int i = 0; i < num_comandos - 1; i++) { // We need one pipe less than the number of commands, as they go between them. 
        if (pipe(totalPipes[i]) < 0) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Finish processing: 
    int pids[max_commands];
    for (int i = 0; i < num_comandos; i++) {
        // Process each command into argvv and filev
        int args_count = tokenizar_linea(comandos[i], " \t\n", argvv, max_args);
        procesar_redirecciones(argvv); // We call the function procesar_redirecciones to see if some redirection has been indicated in that line.

        pids[i] = fork();
        if (pids[i] == 0) {
            execute_command(i, num_comandos);
            exit(EXIT_FAILURE); // If execvp fails, an error is raised and we continue to the next line.
        }
        else if (pids[i] < 0) {
            perror("Fork failed."); // Check that no error when doing fork has happened.
            exit(EXIT_FAILURE);
        }
        else {
            if (background == 1 && i == num_comandos - 1) {
                printf("%d", pids[i]); // The parent process must print via standard output the pid of the child process.
                num_back += num_comandos;
            }
        }
    }

    // Close all pipes in parent.
    for (int i = 0; i < num_comandos - 1; i++) { // The PARENT process also needs to close all pipes!! The parenthe parent is the one that creates the pipes, but it will not be directly using them! 
        close(totalPipes[i][READ]);
        close(totalPipes[i][WRITE]);
    }

    
    // Wait for foreground processes.
    if (background != 0) { // If background has been indicated in the line, we have to wait for the child to end. 
        for (int i = 0; i < num_comandos; i++) {
            waitpid(pids[i], NULL, 0);
        }
    }
    return num_comandos;
}


int main(int argc, char* argv[]) {

    char buffer[1]; // Buffer
    char line[max_line]; // Line to be processed
    size_t i = 0; // unsinged integer. It's better for buffer indexing since it avoids negative values
    int nread = 0; // Number of bytes read
    char ssoo[] = "## Script de SSOO";

    // Open the file
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Error opening the file");
        exit(-1);
    }

    do {
        nread = read(fd, &buffer[0], 1);
        if (nread < 0){
            perror("Error reading from the file");
            exit(-1);
        }

        if (i < max_line - 1) {
            if (buffer[0] == '\n' || nread == 0) { // If an end of line is found or we do not have more bytes to read, we evaluate what to do for the end of the line.
                if (i == 0) { // No empty line can be found in the text file(unless it is the last one), so we raise an error.
                    perror("Empty line founded.");
                    exit(-1);
                }
                else if (line[i - 1] == '\r') { // If the number of elements on the line is nonzero, it is not empty, so we can continue.
                    if (i == 1) { // If on the line there is only the '/r' character, that means it is also empty, so an error is raised.
                        perror("Empty line founded.");
                        exit(-1);
                    }
                    else {
                        line[i - 1] = '\0'; // If it is not empty, we change the '/r' for '/0', as the '/r' is for Windows styled files.
                    }
                }
                else {
                    line[i] = '\0'; // If /r is not found, we simply put at the end of the line '/0'.
                }
                i = 0; // Set the counter to 0, for the next line.
                procesar_linea(line); // Call the processr_linea function to execute it.

                background = 0;
                for (int j = 0; j < max_args; j++) { // Set all the entrances of the array for the arguments to NULL so that it is empty for the next line.
                    argv[j] = NULL;
                }
            }
            else { // If it is not the end of the line, we introduce the character read that is on the buffer onto the string line.
                line[i] = buffer[0];
                i++; // Next character.
            }
        }
        else { // An error is raised, furnishing, if the number of characters per line exceeds the maximum.
            perror("Maximum number of characters per line reached");
            exit(-1);
        }
    } while (nread != 0);

    for (int j = 0; j < num_back; j++) { 
        waitpid(-1, NULL, 0);
    }

    return 0;
}