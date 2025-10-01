/**
 * Handles all neccessary file operations for the 
 * 'mmake' program.
 *
 * @file file_handler.c
 * @author c24nen
 * @date 25.10.01
 */

#include "file_handler.h"

int file_exists(const char *filename)
{
	FILE *fptr = fopen(filename, "r");

	if (fptr == NULL)
		return 0;

	fclose(fptr);

	return 1;
}

struct timespec get_last_mod_time(const char *filename)
{
	if (!file_exists(filename))
		return (struct timespec){0, 0};

	struct stat fileinfo;
	if (stat(filename, &fileinfo) == -1)
	{
		perror("Stat failed");
		return (struct timespec){-1, -1};
	}
	return fileinfo.st_mtim;
}

