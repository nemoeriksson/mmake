#pragma once

/**
 * The program handler handles all functionality related
 * to the setup step of the 'mmake' program. This includes
 * reading program arguments, parsing flags, and getting
 * the targetted make rules (if any).
 *
 * @file program_handler.h
 * @author c24nen
 * @date 2025.09.24
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "parser.h"

typedef struct optioninfo optioninfo;

typedef enum flagtype {
	SILENCE_COMMANDS,
	FORCE_REBUILD,
	CUSTOM_TARGETS
} flagtype;

/**
 * Parses the flags given when running the 'mmake'
 * program and returns related informatio 
 * 
 * @param argc	The total amount of program arguments given
 * @param argv	A list of all program arguments
 *
 * @return		A pointer to an object containing the related 
 *				program information 
 */
optioninfo *get_option_info(int argc, char **argv);

/**
 * Checks if the provided flag option has been specified
 * as a program arguments when running the program.
 *
 * @param options	Information about the program's flags
 * @param flag		The flag to check
 *
 * @return		1 if flag is in use, else 0.
 */
int uses_flag(optioninfo *options, flagtype flag);

/**
 * Opens and parses a makefile. Will use the filename from the
 * optioninfo instance. Will return NULL if any errors occurs.
 *
 * @param options	Information about the program's flags
 *
 * @return			A pointer to type makefile, or NULL on error.
 */
makefile *get_makefile(optioninfo *options);

/**
 * Frees all dynamically allocated memory used by the
 * options info that was recieved from the 'get_option_info'
 * function. Sets the provided pointer to NULL.
 *
 * @param options_ptr		A pointer to the program's flag info
 */
void free_option_info(optioninfo **options_ptr);

/**
 * Validates that all targets in a NULL-terminated list
 * 'targets' have valid rules inside the provided make
 * file. Returns 0 on success, 1 on failure.
 *
 * @param mfile		The makefile
 * @param targets	A NULL-terminated list of all the 
 *					targets to check.
 *
 * @return			0 on success, 1 if any targets does
 *					not have a rule.
 */
int validate_targets(makefile *mfile, char **targets);


