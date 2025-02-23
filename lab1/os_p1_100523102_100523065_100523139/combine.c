#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


#define PERM 0644 // permissions
#define MAX_ALUMNS 100

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
	struct alumno myAlumn[MAX_ALUMNS];
	int count = 0;
	int grades[5] = {0};


	while ((nread = read(infile1, buffer, bufferSize)) > 0){
		if (count >= 100) {
			perror("Error: there can't be more than 100 students!!");
			return -1;
		}
		// Copy buffer content to myAlumn[count]
		memcpy(&myAlumn[count], buffer, bufferSize);

	
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
	}

	if (nread == -1){
		printf("An error has ocurred reading the second file: %d\n", errno);
		return -1;
	}

	close(infile2);

	// Sort by grade
	for (int i = 0; i < count - 1; i++) {
		// Save how many students have 10 (M), 9 (S), N (8 or 7), A (6 or 5), F (less than 5)
		switch (myAlumn[count].nota){
			case 10:
				grades[4]++;
			break;

			case 9:
				grades[3]++;
			break;

			case 8: case 7:
				grades[2]++;
			break;

			case 6: case 5:
				grades[1]++;
			break;

			default:
				grades[0]++;
			break;
		}
		
        for (int j = 0; j < count - i - 1; j++) {
            if (myAlumn[j].nota > myAlumn[j + 1].nota) {
                // Swap myAlumn[j] and myAlumn[j+1]
                struct alumno temp = myAlumn[j];
                myAlumn[j] = myAlumn[j + 1];
                myAlumn[j + 1] = temp;
            } 
			
			// If nota is the same, sort by convocatoria (ascending order)
            else if (myAlumn[j].nota == myAlumn[j + 1].nota) {
                if (myAlumn[j].convocatoria > myAlumn[j + 1].convocatoria) {
                    // Swap myAlumn[j] and myAlumn[j + 1]
                    struct alumno temp = myAlumn[j];
                    myAlumn[j] = myAlumn[j + 1];
                    myAlumn[j + 1] = temp;
                }
                // If both nota and convocatoria are the same, sort by name (ascending order)
                else if (myAlumn[j].convocatoria == myAlumn[j + 1].convocatoria) {
                    if (strcmp(myAlumn[j].nombre, myAlumn[j + 1].nombre) > 0) {
                        // Swap myAlumn[j] and myAlumn[j + 1]
                        struct alumno temp = myAlumn[j];
                        myAlumn[j] = myAlumn[j + 1];
                        myAlumn[j + 1] = temp;
                    }
                }
			}
        }
    }

	
	// Write all students in the outfile

	for (int i = 0; i < count; i++) {
        nread = write(outfile, &myAlumn[i], bufferSize);
        if (nread == -1) {
            perror("Error writing to the output file");
            return -1;
        }
    }

	close(outfile);

	return 0;
}