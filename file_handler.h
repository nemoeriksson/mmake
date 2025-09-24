#pragma once

/**
 * Handles all neccessary file operations for the 
 * 'mmake' program.
 *
 * @file file_handler.h
 * @author c24nen
 * @date 25.09.24 
 */

#include <stdio.h>
#include <sys/stat.h>

/**
 * Gest the time of last modification for a
 * given file.
 *
 * @returns		The time of last modification, is
 *				zeroed if file isn't found.
 */
struct timespec get_last_mod_time(const char *filename);

/**
 * Checks if a file exists of not.
 *
 * @return		1 if the file exists, else 0.
 */
int file_exists(const char *filename);

