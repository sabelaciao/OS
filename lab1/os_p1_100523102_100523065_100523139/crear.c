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

    if (sscanf(argv[2], "%o", &mode) != 1) {
        perror("The input could not be converted to octal");
        return -1;
    }

    // transform argument to octal
    sscanf(argv[2], "%o", &mode); 

    // mode_t has the permissions of a file
    mode_t varmask = umask(0);

    // create the file
    int fd = -1;
    fd = open(argv[1], O_CREAT | O_WRONLY, mode);

    umask(varmask);

    if (fd == -1){
        printf("There has been an error creating the file: %d\n", errno);
        return -1;
    }
    chmod(argv[1], mode);

    return 0;
}


