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
        fprintf("Usage: %s <file> <string>\n", argv[0]);
        return -1;
    }
    // open the file
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Error opening the file");
        return -1;
    }

    // read the file
    char c;
    int i = 0;
    char buffer[1024];
    while (read(fd, &c, 1) == 1) {
        if (c == '\n') {
            buffer[i] = '\0';
            if (strstr(buffer, argv[2]) != NULL) {
                printf("%s\n", buffer);
            }
            // hay que resetear el buffer?
            i = 0;
        } else {
            if (i < sizeof(buffer) - 1) {
                buffer[i] = c;
                i++;
            }
        }
    }
    buffer[i] = '\0';
    if (strstr(buffer, argv[2]) != NULL) {
        printf("%s\n", buffer);
    } else {
        printf("\"%s\" not found.\n", argv[2]);
    }
    close(fd);
    return 0;
}

