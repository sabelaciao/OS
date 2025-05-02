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
		printf("Usage: ./factory_manager <text_file>");
		return -1;
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		printf("[ERROR][factory_manager] Invalid file.");
		free(fd);
		return -1;
	}

	size_t buffer_size = 1024; // Iitial buffer size
	char *line = malloc(buffer_size); // Allocate memory for the line

	if (line == NULL) {
		printf("[ERROR][factory_manager] Process_manager with id 0\n");
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
                printf("[ERROR][factory_manager] Process_manager with id 0\n");
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
		printf("[ERROR][factory_manager] Invalid file.\n");
		free(line);
		close(fd);
		return -1;
	}

	// Ensure the line is null-terminated
	if (bytesRead == 0 || line_pos == 0 || line[line_pos - 1] != '\0') {
		line[line_pos] = '\0';
	}

	// Process the line
	int max_processes = 0;
	int process_count = 0;

	// Get the max number of processes. If the line is empty or the number of processes is not bigger than 0, return an error
	if (sscanf(line, "%d", &max_processes) != 1 || max_processes <= 0) {
        printf("[ERROR][factory_manager] Invalid file.\n");
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