#pragma once

/**
 * Handles all neccessary file operations for the 
 * 'mmake' program.
 *
 * @file file_handler.h
 * @author c24nen
 * @date 25.10.01
 */

#include <stdio.h>
#include <sys/stat.h>

/**
 * Gest the time of last modification for a given file.
 * 
 * @param filename	The name of the file
 *
 * @returns		The time of last modification, is
 *				zeroed if file isn't found. Will
 *				set to -1 if error occurs.
 */
struct timespec get_last_mod_time(const char *filename);

/**
 * Checks if a file exists of not.
 *
 * @param filename The name of the file
 *
 * @return		1 if the file exists, else 0.
 */
int file_exists(const char *filename);

