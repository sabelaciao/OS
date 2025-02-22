#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

struct alumno{
	char nombre[50];
	int nota;
	int convocatoria;
};



int main(int argc, char *argv[]){
	// the function must be called with 4 arguments!!
    if (argc != 4){ 
        perror("The number of arguments are not exact!");
        return -1;
    }

	int textFileOne;
	// Open the first file and check if it was opened correctly
	textFileOne = open(argv[1], 0);

	if(textFileOne == -1){
		perror("The first input file could not be opened");
		return -1;
	}

	int textFileTwo;
	// Open the second file and check if it was opened correctly
	textFileTwo = open(argv[2], 0);

	if(textFileTwo == -1){
		perror("The second input file could not be opened");
		return -1;
	}

	return 0;
}
