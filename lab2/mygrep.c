    // mygrep
    // This command will search for a text string in a text file and display on standard output those lines 
    // in which it appears. If the file does not contain the searched string, the program must display the 
    // message: “ %s not found.\n ”, indicating the searched string between quotes. Finally, if any error occurs, 
    // the program must display a message on standard error output using the perror function and return -1.

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <sys/stat.h>
    #include <sys/types.h>

    int main(int argc, char **argv) {
        // check the number of arguments
        if (argc != 3) {
            perror("Usage: ./mygrep <file> <string>");
            exit(-1);
        }

        // Open the file
        int fd = open(argv[1], O_RDONLY);
        if (fd == -1) {
            perror("Error opening the file");
            exit(-1);
        }

        // Read the file
        char c; // pointer to reach each letter
        size_t i = 0; // unsinged integer. It's better for buffer indexing since it avoids negative values
        ssize_t nread = -1;
        int found = 0; // flag to just to display the message if the string was found

        // Dynamic buffer allocation (start with size 1024, later we reallocate if necessary)
        size_t buffer_size = 1024;

        // Allocate 1024 bytes to the pointer 'buffer'
        char *buffer = (char *)malloc(buffer_size);

        // If malloc fails, it returns NULL
        if (!buffer) {
            perror("Memory allocation failed");
            close(fd);
            exit(-1);
        }

        while ((nread = read(fd, &c, 1)) > 0) {
            // Expand buffer dynamically if needed
            if (i >= buffer_size - 1) {
                size_t new_size = buffer_size * 2; // why *2? Doubling is LOGARITHMIC! It's better than summing one by one
                                                // since you do not need to realloc IN EACH ITERATION, which is slow.
                                                // Note that realloc copies the old data, in a larger block. It wastes
                                                // A LOT of time. Unused space is at most 50%, which is small compared
                                                // to the permofance LOST in reallocating in each iteration


                // if (new_size == 0) new_size = 1; // Ensure at least 1 byte

                char *new_buffer = realloc(buffer, new_size); // we create a new_buffer JUST in case there is an
                                                            // error in realloc. If it happens, original buffer
                                                            // is NOT edited, which is more safe
                if (!new_buffer) {
                    perror("Memory reallocation failed");
                    free(buffer);
                    close(fd);
                    exit(-1);
                }
                buffer = new_buffer; // now update the buffer safely
                buffer_size = new_size; // and update the buffer size also safely
            }

            // read lines until a jump of line is found, which we replace by '\0'
            if (c == '\n') {
                buffer[i] = '\0';
                if (strstr(buffer, argv[2]) != NULL) { // strstr finds the first ocurrence of a substring inside a string
                    found = 1;
                    printf("%s\n", buffer);
                }
                i = 0; // we reset the index where we write in the buffer
            } else {
                // Just write if there is space in the buffer. If not, it will be
                // rellocated later
                buffer[i] = c;
                i++; // points to the next available position in the buffer
            }
        }

        // Process the last line if the file does not end with '\n'
        buffer[i] = '\0';
        if (strstr(buffer, argv[2]) != NULL) {
            found = 1;
            printf("%s\n", buffer);
        }
        
        // In case there has been an error reading
        if (nread == -1){
            perror("An error has occurred reading the file!");
            free(buffer);
            close(fd);
            exit(-1);
        }

        if (found == 0){
            printf("%s not found.\n", argv[2]);
        }

        close(fd);
        free(buffer);

        return 0;
    }

