#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // the function must be called with 3 arguments!!
    if (argc != 3){ 
        perror("The number of arguments are not exact!");
        return -1;
    }

    mode_t mode;
    
    // transform argument to octal and checks if it's valid
    if (sscanf(argv[2], "%o", &mode) != 1) {
        perror("The input could not be converted to octal");
        return -1;
    }

    // mode_t has the permissions of a file

    // Save old umask and set it to 0
    mode_t varmask = umask(0);

    // create the file
    int fd = -1;
    // O_EXCL -> Ensure that a file is only created if it does not already exist. If it does, it loads an error in 'errno' variable
    // O_CREAT ->  Creates the file if it does not exist.
    // O_TRUNC -> Truncate (empty) a file if it already exists.
    fd = open(argv[1], O_CREAT | O_EXCL | O_TRUNC, mode);

    umask(varmask);

    if (fd == -1){
        printf("There has been an error creating the file: %d\n", errno);
        return -1;
    }
    chmod(argv[1], mode);

    // close the file
    close(fd);

    return 0;
}