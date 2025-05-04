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
#include "process_manager.h"
#include "factory_manager.h"

// Sempahore to synchronize the factory
sem_t factory_semaphore; 


int main (int argc, const char * argv[] ){

	if (argc != 2) {
		printf("[ERROR][factory_manager] Invalid file.\n");
		return -1;
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		printf("[ERROR][factory_manager] Invalid file.\n");
		return -1;
	}

	size_t buffer_size = 1024; // Initial buffer size
	char *line = malloc(buffer_size); // Allocate memory for the unique line in the file

	if (line == NULL) {
		printf("[ERROR][factory_manager] Process_manager with id 0 has finished with errors.\n");
		close(fd);
		return -1;
	}
	
	// Read the unique line from the file
	size_t line_pos = 0;

	// Variable to store the character read
	char ch;
	
	// Variable to store the number of bytes read
	ssize_t bytesRead;

	// Read the file character by character
	while ((bytesRead = read(fd, &ch, 1)) > 0) {
        // Expand the buffer if it is full
        if (line_pos >= buffer_size - 1) {  
            buffer_size *= 2;  // We double the buffer size if it is full
            line = realloc(line, buffer_size); // Reallocate memory for the line
            if (line == NULL) {
                printf("[ERROR][factory_manager] Process_manager with id 0 has finished with errors.\n");
                close(fd);
                return -1;
            }
        }

		// If we reach the end of the line, break
		if (ch == '\n') {
			line[line_pos] = '\0'; // Replace '\n' with '\0'
			break;
		}

		// Save the character in the buffer
		line[line_pos++] = ch;
	}
	
	// Check for read errors
	if (bytesRead < 0) {
		printf("[ERROR][factory_manager] Invalid file.\n");
		free(line);
		close(fd);
		return -1;
	}

	// Close the file descriptor
	if (close(fd) < 0) {	
		perror("[ERROR][factory_manager] Invalid file.");
		return -1;
	}

	// Process the line
	int max_belts = 0;
	int belts_count = 0;
	process_data_t *belts = NULL;  // Pointer to store belts

	// Variable to store the number of bytes consumed by sscanf
	int bytes_consumed = 0;

	// Get the max number of belts. If the line is empty or the number of belts is not bigger than 0, return an error
	if (sscanf(line, "%d%n", &max_belts, &bytes_consumed) != 1 || max_belts <= 0) {
        printf("[ERROR][factory_manager] Invalid file.\n");
        free(line);
        return -1;
    }

	// Dynamically allocate memory for belts based on max_belts
    belts = malloc((max_belts+1) * sizeof(process_data_t));
    if (belts == NULL) {
        perror("[ERROR][factory_manager] Process_manager with id 0 has finished with errors.\n");
        free(line);
        return -1;
    }

	// Read belt_id, belt_size_ and number of items 3 by 3
	int number_of_arguments;

	char *ptr = line + bytes_consumed; // Pointer to the next argument
	while (*ptr == ' '){ // Skip any spaces
		ptr++;
	} 

	while((number_of_arguments = sscanf(ptr, "%d %d %d%n", &belts[belts_count].id_belt, &belts[belts_count].belt_size, &belts[belts_count].elements_to_generate, &bytes_consumed)) == 3) {

		// Validate the values of the process_manager
		if (belts[belts_count].id_belt < 0 || belts[belts_count].belt_size <= 0 || belts[belts_count].elements_to_generate <= 0) {
			printf("[ERROR][factory_manager] Invalid file.\n");
			free(belts);
			free(line);
			return -1;
		}

		if (sem_init(&belts[belts_count].semaphore_b, 0, 0) != 0) { // Initialize the semaphore for the process_manager
			perror("[ERROR][factory_manager] Process_manager with id 0 has finished with errors.\n");
			free(belts);
			free(line);
			return -1;
		}

		belts_count++;

		// Error if we have more belts than the maximum number of belts
		if (belts_count > max_belts) {
			printf("[ERROR][factory_manager] Invalid file.\n");
			free(belts);
			free(line);
			return -1;
		}

		ptr += bytes_consumed;
		while (*ptr == ' '){ // Skip any spaces
			ptr++;
		} 
	}

	if (number_of_arguments > 0) { // Error if when finished reading the file, we have not read 3 by 3 arguments
		printf("[ERROR][factory_manager] Invalid file.\n");
		free(belts);
		free(line);
		return -1;
	}

	free(line); // Free the line buffer

	if(sem_init(&factory_semaphore, 0, 0) != 0) { // Initialize the semaphore for the factory
		perror("[ERROR][factory_manager] Process_manager with id 0 has finished with errors.\n");
		free(belts);
		return -1;
	};

	// Create the threads
	for (int i = 0; i < belts_count; i++) {
		// Create the process_manager thread
		if (pthread_create(&belts[i].thread_b, NULL, process_manager, &belts[i]) != 0) { // belts[i] is the argument for process_manager
			printf("[ERROR][factory_manager] Process_manager with id %d has finished with error.\n", belts[i].id_belt);
			free(belts);
			return -1;
		}
		printf("[OK][factory_manager] Process_manager with id %d has been created.\n", belts[i].id_belt);
	}

	for (int i = 0; i < belts_count; i++) {
		// Signal (alert) the semaphore to start the process_manager 
		if (sem_post(&belts[i].semaphore_b) != 0) {
			printf("[ERROR][factory_manager] Process_manager with id %d has finished with error.\n", belts[i].id_belt);
			free(belts);
			return -1;
		}

		if (sem_wait(&factory_semaphore) == -1) {
			printf("[ERROR][process_manager] There was an error executing process_manager with id %d.\n", belts[i].id_belt);
			free(belts);
			return -1;
		}
	}
	
	// Wait for all threads to finish
	for (int i = 0; i < belts_count; i++) {
		
		// Wait for the thread to finish
		if (sem_post(&belts[i].semaphore_b) != 0) {
			printf("[ERROR][factory_manager] Process_manager with id %d has finished with error.\n", belts[i].id_belt);
			free(belts);
			return -1;
		}

		if (pthread_join(belts[i].thread_b, NULL) != 0) {
			printf("[ERROR][factory_manager] Process_manager with id %d has finished with error.\n", belts[i].id_belt);
			free(belts);
			return -1;
		}
		
		printf("[OK][factory_manager] Process_manager with id %d has finished.\n", belts[i].id_belt);
	}


	// Remove the semaphores
	for (int i = 0; i < belts_count; i++) {
		if (sem_destroy(&belts[i].semaphore_b) != 0) {
			perror("[ERROR][factory_manager] Process_manager with id 0 has finished with errors.\n");
			free(belts);
			return -1;
		}
	}

	if (sem_destroy(&factory_semaphore) != 0) {
		perror("[ERROR][factory_manager] Process_manager with id 0 has finished with errors.\n");
		free(belts);
		return -1;
	}

	
	// Free the allocated memory
	free(belts);

	printf("[OK][factory_manager] Finishing.\n");
	return 0;
}