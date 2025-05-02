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

// Structure to hold process_manager parameters
typedef struct {
    int id_belt;
    int belt_size;
    int elements_to_generate;
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
		printf("[ERROR][factory_manager] Process_manager with id 0 has finished with errors.\n");
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
                printf("[ERROR][factory_manager] Process_manager with id 0 has finished with errors.\n");
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

	// Close the file descriptor
	if (close(fd) < 0) {	
		perror("[ERROR][factory_manager] Invalid file.");
		return -1;
	}

	// Ensure the line is null-terminated
	if (bytesRead == 0 || line_pos == 0 || line[line_pos - 1] != '\0') {
		line[line_pos] = '\0';
	}

	// Process the line
	int max_processes = 0;
	int process_count = 0;
	process_data_t *processes = NULL;  // Pointer to store processes

	// Get the max number of processes. If the line is empty or the number of processes is not bigger than 0, return an error
	if (sscanf(line, "%d", &max_processes) != 1 || max_processes <= 0) {
        printf("[ERROR][factory_manager] Invalid file.\n");
        free(line);
        close(fd);
        return -1;
    }

	// Dynamically allocate memory for processes based on max_processes
    processes = malloc(max_processes * sizeof(process_data_t));
    if (processes == NULL) {
        perror("[ERROR][factory_manager] Process_manager with id 0 has finished with errors.\n");
        free(line);
        close(fd);
        return -1;
    }

	// Read belt_id, belt_size_ and number of items 3 by 3
	int number_of_arguments;
	char *ptr = line;
	while((number_of_arguments = sscanf(ptr, "%d %d %d", &processes[process_count].id_belt, &processes[process_count].belt_size, &processes[process_count].elements_to_generate)) == 3) {

		// Validate the values of the process_manager
		if (processes[process_count].id_belt < 0 || processes[process_count].belt_size <= 0 || processes[process_count].elements_to_generate <= 0) {
			printf("[ERROR][factory_manager] Invalid file.\n");
			free(processes);
			free(line);
			close(fd);
			return -1;
		}

		process_count++;
		if (process_count > max_processes) { // Error if we reach the maximum number of processes
			printf("[ERROR][factory_manager] Invalid file.\n");
			free(processes);
			free(line);
			close(fd);
			return -1;
		}

		while (*ptr == ' '){ // Skip any spaces
			ptr++;
		} 
	}

	if (number_of_arguments != 0) { // Error if when finished reading the file, we have not read 3 by 3 arguments
		printf("[ERROR][factory_manager] Invalid file.\n");
		free(processes);
		free(line);
		close(fd);
		return -1;
	}

	free(line); // Free the line buffer

	// Dynamically allocate the threads and semaphores arrays
    pthread_t *threads = malloc(max_processes * sizeof(pthread_t));
	if (threads == NULL) {
		perror("[ERROR][factory_manager] Process_manager with id 0 has finished with errors.\n");
		free(processes);
		close(fd);
		return -1;
	}

    sem_t *sem_processes = malloc(max_processes * sizeof(sem_t));
	if (sem_processes == NULL) {
		perror("[ERROR][factory_manager] Process_manager with id 0 has finished with errors.\n");
		free(threads);
		free(processes);
		close(fd);
		return -1;
	}

	// Initialize the semaphores
	for (int i = 0; i < process_count; i++) {
		if (sem_init(&sem_processes[i], 0, 0) != 0) { // Initially blocked
			perror("[ERROR][factory_manager] Process_manager with id 0 has finished with errors.\n");
			free(threads);
			free(processes);
			free(sem_processes);
			close(fd);
			return -1;
		}
	}

	// Create the threads
	for (int i = 0; i < process_count; i++) {
		// Create the process_manager thread
		if (pthread_create(&threads[i], NULL, process_manager, &processes[i]) != 0) { // process[i] is the argument for process_manager
			printf("[ERROR][factory_manager] Process_manager with id %d has finished with error.\n", processes[i].id_belt);
			free(threads);
			free(processes);
			free(sem_processes);
			close(fd);
			return -1;
		}

		// Signal (alert) the semaphore to start the process_manager 
		if (sem_post(&sem_processes[i]) != 0) {
			printf("[ERROR][factory_manager] Process_manager with id %d has finished with error.\n", processes[i].id_belt);
			free(threads);
			free(processes);
			free(sem_processes);
			close(fd);
			return -1;
		}

		printf("[OK][factory_manager] Process_manager with id %d has been created.\n", processes[i].id_belt);
	}

	// Wait for all threads to finish
	for (int i = 0; i < process_count; i++) {
		// Wait the signal from the semaphore to execute the actual process_manager
		if (sem_wait(&sem_processes[i]) != 0) {
			printf("[ERROR][factory_manager] Process_manager with id %d has finished with error.\n", processes[i].id_belt);
			free(threads);
			free(processes);
			free(sem_processes);
			close(fd);
			return -1;
		}

		// Wait for the thread to finish
		if (pthread_join(threads[i], NULL) != 0) {
			printf("[ERROR][factory_manager] Process_manager with id %d has finished with error.\n", processes[i].id_belt);
			free(threads);
			free(processes);
			free(sem_processes);
			close(fd);
			return -1;
		}

		printf("[OK][factory_manager] Process_manager with id %d has finished.\n", processes[i].id_belt);
	}


	// Remove the semaphores
	for (int i = 0; i < process_count; i++) {
		if (sem_destroy(&sem_processes[i]) != 0) {
			perror("[ERROR][factory_manager] Process_manager with id 0 has finished with errors.\n");
			free(threads);
			free(processes);
			free(sem_processes);
			close(fd);
			return -1;
		}
	}
	
	// Free the allocated memory
	free(threads);
	free(processes);
	free(sem_processes);

	printf("[OK][factory_manager] Finishing.\n");
	return 0;
}