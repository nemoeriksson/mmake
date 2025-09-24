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

typedef struct programinfo programinfo;
typedef struct targetruleinfo targetruleinfo;

typedef enum flagoption {
	SILENCE_COMMANDS,
	FORCE_REBUILD
} flagoption;

/**
 * Parses the flags given when running the 'mmake'
 * program and returns related information that can
 * be used in the rest of the program to read the
 * flag's states and the makefile that should be parsed.
 * 
 * @param argc	The total amount of program arguments given
 * @param argv	A list of all program arguments
 *
 * @return		A pointer to an object containing the related 
 *				program information 
 */
programinfo *get_program_info(int argc, char **argv);

/**
 * Checks if the provided flag option has been specified
 * as a program arguments when running the program.
 *
 * @param pinfo		Information about the program
 * @param flag		The flag to check
 *
 * @return		1 if flag is in use, else 0.
 */
int uses_flag(programinfo *pinfo, flagoption flag);

/**
 * Returns the makefile from program info instance.
 *
 * @param pinfo		Information about the program
 *
 * @return		The makefile that is in use by the program
 */
makefile *get_makefile(programinfo *pinfo);

/**
 * Frees all dynamically allocated memory used by the
 * program info that was recieved from the 'get_program_info'
 * function. Sets the programinfo pointer to NULL.
 *
 * @param pinfo_ptr		A pointer to the program info
 */
void free_program_info(programinfo **pinfo_ptr);


/**
 * Gets information related to the targetted make rules that were
 * specified as program arguments when running the 'mmake' program.
 *
 * @param argc	Amount of program arguments
 * @param argv	A list of all program arguments
 *
 * @return		Information about the target rules specified as
 *				program arguments, if none were given then returns NULL
 */
targetruleinfo *get_argument_target_rules(int argc, char **argv);

/**
 * Gets information related to the default make rule. This will be
 * the first rule defined in the provided makefile.
 *
 * @param pinfo	Information about the program
 *
 * @return		Information about the default makefile target rule.
 */
targetruleinfo *get_default_target_rule(programinfo *pinfo);

/**
 * Gets the number of rules that need to be checked to build the
 * program as specified either as program arguments of the makefile's
 * default rule, in which this will return 1.
 *
 * @param trinfo	Information about the targets
 *
 * @return		The amount of rules specified as program arguments, or 1.
 */
int get_targetted_rule_count(targetruleinfo *trinfo);

/**
 * Gets the target rule's name at the specified index. Returns NULL
 * if the index is invalid.
 *
 * @param trinfo	Information about the targets to build
 * @param index		Index of the target to get rule from
 *
 * @return		The target rule name, or NULL.
 */
char *get_targetted_rule(targetruleinfo *trinfo, int index);

/**
 * Frees all dynamically allocated memory that is used by an
 * instance of targetruleinfo. Also sets the targetruleinfo
 * pointer to NULL.
 */
void free_target_rules(targetruleinfo **trinfo_ptr);

/**
 * Validates that all rules recieved from either 'get_argument_target_rules'
 * or 'get_default_target_rule' are valid rules inside the provided
 * make file.
 *
 * @param pinfo		Information about the program
 * @param trinfo	Information about the targets to validate
 *
 * @returns 0 on success, 1 if any rule is invalid.
 */
int validate_rules(programinfo *pinfo, targetruleinfo *trinfo);


