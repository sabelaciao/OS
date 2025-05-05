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
	
	// Open the file
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		printf("[ERROR][factory_manager] Invalid file.\n");
		return -1;
	}
	
	// Initial buffer size
	size_t buffer_size = 1024; 

	// Allocate memory for the unique line in the file
	char *line = malloc(buffer_size);
 
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
			// When we read '\n', there should not be any more characters in the file, because the file would have more than one line.
			if ((bytesRead = read(fd, &ch, 1)) > 0) {
				printf("[ERROR][factory_manager] Invalid file.\n");
				free(line);
				close(fd);
				return -1;
			}
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
 
	// Pointer to the next argument
	char *ptr = line + bytes_consumed; 

	// Skip the first space
	if (*ptr != ' ' || ptr[1] < '0' || ptr[1] > '9') {
		printf("[ERROR][factory_manager] Invalid file.\n");
		free(belts);
		free(line);
		return -1;
	}
 
	while((number_of_arguments = sscanf(ptr, "%d%n", &belts[belts_count].id_belt, &bytes_consumed)) == 1) {
 
		ptr += bytes_consumed;

		// Check spacing after first number (must be exactly one space)
		if (*ptr != ' ' || ptr[1] < '0' || ptr[1] > '9') {
			printf("[ERROR][factory_manager] Invalid file.\n");
			free(belts);
			free(line);
			return -1;
		}

		// Skip the single space
		ptr++; 

		// Read the second number
		if ((number_of_arguments = sscanf(ptr, "%d%n", &belts[belts_count].belt_size, &bytes_consumed)) != 1) {
			printf("[ERROR][factory_manager] Invalid file.\n");
			free(belts);
			free(line);
			return -1;
		}
		
		ptr += bytes_consumed;
		
		// Check spacing after second number (must be exactly one space)
		if (*ptr != ' ' || ptr[1] < '0' || ptr[1] > '9') {
			printf("[ERROR][factory_manager] Invalid file.\n");
			free(belts);
			free(line);
			return -1;
		}
		ptr++;  // Skip the single space

		// Read the third number
		if ((number_of_arguments = sscanf(ptr, "%d%n", &belts[belts_count].elements_to_generate, &bytes_consumed)) != 1) {
			printf("[ERROR][factory_manager] Invalid file.\n");
			free(belts);
			free(line);
			return -1;
		}
		
		ptr += bytes_consumed;

		// After the third number, either:
    	// 1. End of string (valid)
    	// 2. Exactly one space (if more belts follow)
    	if (*ptr != '\0') {
        	if (*ptr != ' ' || ptr[1] < '0' || ptr[1] > '9') {
            	printf("[ERROR][factory_manager] Invalid file.\n");
            	free(belts);
            	free(line);
            return -1;
        	}

			// Skip the single space
        	ptr++; 
   	 	}

		// Validate the values of the process_manager
		if (belts[belts_count].id_belt < 0 || belts[belts_count].belt_size <= 0 || belts[belts_count].elements_to_generate <= 0) {
			printf("[ERROR][factory_manager] Invalid file.\n");
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
	}

	// Error if we have not read any belts. In the statement of the lab, it says: <Max number belts> [<belt ID> <belt size> <No.elements>]+
	if (belts_count == 0) {
		printf("[ERROR][factory_manager] Invalid file.\n");
		free(belts);
		free(line);
		return -1;
	}
 
	// Free the line buffer
	free(line);

	// Initlize the semaphore of the factory
	// The semaphore is used to synchronize the process_manager. It is initialized to 1, so that the first process_manager can start
	if(sem_init(&factory_semaphore, 0, 1) != 0) {
		printf("[ERROR][factory_manager] Process_manager with id 0 has finished with errors.\n");
		free(belts);
		return -1;
	}
	
 
	// Create the threads
	for (int i = 0; i < belts_count; i++) {

		// Initialize the semaphore for each process_manager. It is used to synchronize the process_manager.
		// It is initialized to 0, so that the process_manager will wait for the signal to start
		if (sem_init(&belts[i].semaphore_b, 0, 0) != 0) {
			printf("[ERROR][factory_manager] Process_manager with id %d has finished with error.\n", belts[i].id_belt);
			free(belts);
			return -1;
		}
		
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
	}

	// Wait for all threads to finish
	for (int i = 0; i < belts_count; i++) {
		if (pthread_join(belts[i].thread_b, NULL) != 0) {
			printf("[ERROR][factory_manager] Process_manager with id %d has finished with error.\n", belts[i].id_belt);
			free(belts);
			return -1;
		}
		 
		printf("[OK][factory_manager] Process_manager with id %d has finished.\n", belts[i].id_belt);
	}
 
 
	// Remove the semaphores
	for (int i = 0; i < belts_count; i++) {
		sem_destroy(&belts[i].semaphore_b);
	}
	
	 
	// Free the allocated memory
	free(belts);
 
	printf("[OK][factory_manager] Finishing.\n");
	return 0;
}