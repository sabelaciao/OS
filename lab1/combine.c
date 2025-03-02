#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


#define PERM 0644 // Gives permissions 'read' for everyone and 'write' only for the user (default)
#define MAX_ALUMNS 100

struct alumno{
	char nombre[50];
	int nota;
	int convocatoria;
};



int main(int argc, char *argv[]){
	// The function must be called with 4 arguments, as asked in the statement!!
    if (argc != 4){ 
        printf("The number of arguments is not exact!");
    	return -1;
    }

	// Create variables to open the first two arguments (infile1, infile2),
	// open the third argument (outfile), read all 'alumnos' from both infile1
	// and infile2 (nread), and write them in the third argument and in
	// estadisticas.csv (nwrite)

	int infile1 = -1, infile2 = -1, outfile = -1, estadisticas = -1, ret = -1; // For file descriptors -> int
	ssize_t nread = -1, nwrite  = -1; // For reading/writing bytes -> ssize_t

	// We will use the buffer. First, we need to know how many bytes
	// has each 'alumno' so we get the exact number of them in each iteration
	int bufferSize = sizeof(struct alumno);

	// Creation of the buffer with the size of each 'alumno'
	char buffer[bufferSize];

	// Create of a struct of 'alumno' type with 100 spots that by default have an empty name,
	// and name and convocatoria equal to -1. This is used to handle errors later when
	// reading 'alumnos'
	struct alumno alumns[MAX_ALUMNS] = {{"", -1, -1}};

	// Used to count the number of 'alumnos'
	int count = 0;




	// Open the first file and check if it was opened correctly

	// FLAGS:
	// O_RDONLY -> Open the file for reading only!

	mode_t mask1 = umask(0); // Save mask
	infile1 = open(argv[1], O_RDONLY);
	umask(mask1);

	if (infile1 == -1){
		perror("The first input file could not be opened");
		return -1;
	}

	// We read all 'alumnos' from the file of the first argument (infile1)

	while ((nread = read(infile1, buffer, bufferSize)) > 0){
		// If we count 100 'alumnos', we should exit the program!
		if (count >= 100) {
			printf("Error: there can't be more than 100 students!!");
			close(infile1);
			return -1;
		}

		// Copy buffer content to alumns[count]
		memcpy(&alumns[count], buffer, bufferSize);

		// Check if the grades of the alumn is between 0 and 10
		if (alumns[count].nota < 0 || alumns[count].nota > 10){
			printf("Error: alumno with name %s can't have a grade lower than 0 or bigger than 10", alumns[count].nombre);
			close(infile1);
			return -1;
		}

		// Check if the name of the alumno is not empty
		if (strlen(alumns[count].nombre) == 0){
			printf("Error: there is a student without name!");
			close(infile1);
			return -1;
		}

		// Check if the convocatoria of an alumno is bigger than 0
		if (alumns[count].convocatoria <= 0){
			printf("Error: alumno with name %s can't have a convocatoria less or equal than 0", alumns[count].nombre);
			close(infile1);
			return -1;
		}

		count++;
	}

	// In case there has been an error reading any alumno from first file
	if (nread == -1){
		printf("An error has ocurred reading the first file!");
		return -1;
	}

	// Close the first file
	ret = close(infile1);

	if (ret == -1){
        perror("Error closing the first file\n");
        return -1;
    }


	// Open the second file and check if it was opened correctly
	mode_t mask2 = umask(0); // Save mask
	infile2 = open(argv[2], O_RDONLY);
	umask(mask2);

	if (infile2 == -1){
		close (infile1);
		perror("The second input file could not be opened");
		return -1;
	}


	// We read all 'alumnos' from the file of the second argument (infile2)

	while ((nread = read(infile2, buffer, bufferSize)) > 0){
		// If we count 100 'alumnos', we should exit the program!
		if (count >= 100) {
			printf("Error: there can't be more than 100 students!!");
			close(infile2);
			return -1;
		}

		// Copy buffer content to alumns[count]
		memcpy(&alumns[count], buffer, bufferSize);

		// Check if the grades of the alumn is between 0 and 10
		if (alumns[count].nota < 0 || alumns[count].nota > 10){
			printf("Error: alumno with name %s can't have a grade lower than 0 or bigger than 10", alumns[count].nombre);
			close(infile2);
			return -1;
		}

		// Check if the name of the alumno is not empty
		if (strlen(alumns[count].nombre) == 0){
			printf("Error: here is a student without name!");
			close(infile2);
			return -1;
		}

		// Check if the convocatoria of an alumno is bigger than 0
		if (alumns[count].convocatoria <= 0){
			printf("Error: alumno with name %s can't have a convocatoria less or equal than 0", alumns[count].nombre);
			close(infile2);
			return -1;
		}

		count++;
	}

	// In case there has been an error reading any alumno from second file
	if (nread == -1){
		printf("An error has ocurred reading the second file!");
		return -1;
	}

	// Close the second file
	ret = close(infile2);

	if (ret == -1){
        perror("Error closing the second file\n");
        return -1;
    }


	// Bubble sort by grade ('nota') in ascending order
	for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (alumns[j].nota > alumns[j + 1].nota) {
                // Swap alumns[j] and alumns[j+1]
                struct alumno temp = alumns[j];
                alumns[j] = alumns[j + 1];
                alumns[j + 1] = temp;
            } 
			
			// If 'nota' is the same, sort by 'convocatoria' in ascending order
            else if (alumns[j].nota == alumns[j + 1].nota) {
                if (alumns[j].convocatoria > alumns[j + 1].convocatoria) {
                    // Swap myAlumn[j] and myAlumn[j + 1]
                    struct alumno temp = alumns[j];
                    alumns[j] = alumns[j + 1];
                    alumns[j + 1] = temp;
                }

                // If both 'nota' and 'convocatoria' are the same, sort by 'nombre' in ascending order
                else if (alumns[j].convocatoria == alumns[j + 1].convocatoria) {
                    if (strcmp(alumns[j].nombre, alumns[j + 1].nombre) < 0) {
                        // Swap alumns[j] and alumns[j + 1]
                        struct alumno temp = alumns[j];
                        alumns[j] = alumns[j + 1];
                        alumns[j + 1] = temp;
                    }
                }
			}
        }
    }


	// For 'estadisticas.csv' we need to to know the porcentage of each type of
	// alumno divided by his/her grade. Then, we create an array to store
	// how many F's (grades[0]), A's (grades[1]), N's (grades[2]), S's (grades[3])
	// and M's (grades[4]) there are
	int grades[5] = {0};

	// Save how many students have 10 (M), 9 (S), N (8 or 7), A (6 or 5), F (less than 5)
    for (int i = 0; i < count; i++){
        switch (alumns[i].nota){
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
    }


	// Write all students in the outfile

	// FLAGS:
	// O_WRONLY -> Open for writing
	// O_CREAT -> Create if it does not exist
	// O_TRUNC -> Empty the file if it exists
	// PERM -> permissions

	mode_t maskOutfile = umask(0); // Save mask
	outfile = open(argv[3], O_CREAT | O_WRONLY | O_TRUNC, PERM);
	umask(maskOutfile);

	if (outfile == -1) {
		perror("The output file could not be created!");
		return -1;
	}
	
	for (int i = 0; i < count; i++){
		// Write each saved alumno
        nwrite = write(outfile, &alumns[i], sizeof(alumns[i]));
        if (nwrite == -1){
            perror("Error writing to the output file");
			close(outfile);
            return -1;
        }
    }

	// Close output file
	ret = close(outfile);

	if (ret == -1){
        perror("Error closing the output file\n");
        return -1;
    }

	// Create estadisticas.csv
	mode_t estadisticasMask = umask(0); // Save mask
	estadisticas = open("estadisticas.csv", O_WRONLY | O_CREAT | O_TRUNC, PERM);
	umask(estadisticasMask);

	// Creation of array of chars with enough space to save the concatenation of the data asked
	// and then write it in 'estadisticas.csv' 
	char result[20];

	// As we said before, in the position [0] there are the students with 'F', in [1] with 'A',
	// in [2] with 'N', in [3] with 'S', and in [4] with 'M'
	char differentGrades[5] = {'F', 'A' , 'N', 'S', 'M'};

	// Create the string as asked. If count is 0, we should not do anything as in the grades part,
	// it will divide by 0!
	for(int i = 4; i >= 0; i--){
		if (count > 0) {
			// Concatenate all data as asked in the statement in 'result' char array
			sprintf(result, "%c;%d;%.2f%%\n", differentGrades[i], grades[i], (double)(grades[i]*100)/count);

		} else {
			// If there are not any alumnos, just write in estadisticas.csv that all differente
			// types of grades have 0% of alumnos
			sprintf(result, "%c;%d;0.00%%\n", differentGrades[i], grades[i]);
		}

		// Write in estadisticas.csv file
		// We use strlen sice sizeof returns 20, as it returns the size of all the char array
		// We are only interested in those characters that are not the empty ones ('\0')
		if (write(estadisticas, result, strlen(result)) == -1){
			perror("Error writing in estadisticas.csv\n");
			close(estadisticas);
			return -1;
		}
	}

	if (count == 0){
		printf("There are no students to write in estadisticas.csv!!\n");
	}

	// Close 'estadisticas.csv' file!
	ret = close(estadisticas);

	if (ret == -1){
        perror("Error closing estadisticas file\n");
        return -1;
    }

	printf("The program worked succesfully! :D\n");
	return 0;
}