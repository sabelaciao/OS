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

int main (int argc, const char * argv[] ){

	if (argc != 2) {
		perror("Usage: ./factory_manager <text_file>");
		return -1;
	}


	int* status;


	return 0;
}