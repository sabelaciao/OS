/*
 *
 * factory_manager.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>

// Structure to hold process_manager parameters
typedef struct {
    int id_belt;
    int belt_size;
    int items_to_produce;
} process_data_t;

int main (int argc, const char * argv[] ){

	if (argc != 2) {
		perror("Usage: ./factory_manager <text_file>");
		return -1;
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror("[ERROR][factory_manager] Invalid file.");
		free(fd);
		return -1;
	}

	size_t buffer_size = 1024; // Iitial buffer size
	char *line = malloc(buffer_size); // Allocate memory for the line

	if (line == NULL) {
		perror("[ERROR][factory_manager] Process_manager with id 0");
		close(fd);
		return -1;
	}
	
	// Read the unique line from the file
	size_t line_pos = 0;
	char ch;
	ssize_t bytesRead;

	while ((bytesRead = read(fd, &ch, 1)) > 0) {
        // Expand the buffer if it is full
        if (line_pos >= buffer_size - 1) {  
            buffer_size *= 2;  // We double the buffer size
            line = realloc(line, buffer_size);
            if (line == NULL) {
                perror("[ERROR][factory_manager] Process_manager with id 0");
                close(fd);
                return -1;
            }
        }

		// Save the character in the buffer
		line[line_pos++] = ch;

		// If we reach the end of the line, break
		if (ch == '\n') {
			line[line_pos-1] = '\0'; // Replace '\n' with '\0'
			break;
		}
	}

	if (bytesRead < 0) {
		perror("[ERROR][factory_manager] Invalid file.");
		free(line);
		close(fd);
		return -1;
	}

	int* status;


	if (close(fd) < 0) {
		perror("[ERROR][factory_manager] Invalid file.");
		return -1;
	}

	return 0;
}