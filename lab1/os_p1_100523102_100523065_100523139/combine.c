#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


#define PERM 0644 // permissions

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

	ssize_t infile1, infile2, outfile, nread;

	// Open the first file and check if it was opened correctly

	// O_RDONLY -> Open the file in binary mode
	infile1 = open(argv[1], O_RDONLY);

	if(infile1 == -1){
		perror("The first input file could not be opened");
		return -1;
	}

	// Open the second file and check if it was opened correctly
	infile2 = open(argv[2], O_RDONLY);

	if(infile2 == -1){
		close (infile1);
		perror("The second input file could not be opened");
		return -1;
	}

	outfile = creat(argv[3], PERM);

	if (outfile == -1) {
		close (infile1);
		close (infile2);
		perror("The output file could not be created!");
		return -1;
	}

	int sum = 0;
	int bufferSize = sizeof(struct alumno);
	char buffer[bufferSize];
	struct alumno myAlumn[100];
	int count = 0;


	while ((nread = read(infile1, buffer, bufferSize)) > 0){
		if (count >= 100) {
			perror("Error: there can't be more than 100 students!!");
			return -1;
		}
		// Copy buffer content to myAlumn[count]
		memcpy(&myAlumn[count], buffer, bufferSize);
		count++;
	}

	if(nread == -1){
		printf("An error has ocurred reading the first file: %d\n", errno);
		return -1;
	}

	close(infile1);

	while ((nread = read(infile2, buffer, bufferSize)) > 0){
		if (count >= 100) {
			perror("Error: there can't be more than 100 students!!");
			return -1;
		}
		// Copy buffer content to myAlumn[count]
		memcpy(&myAlumn[count], buffer, bufferSize);
		count++;
	}

	if (nread == -1){
		printf("An error has ocurred reading the second file: %d\n", errno);
		return -1;
	}

	close(infile2);

	

	close(outfile);

	return 0;
}