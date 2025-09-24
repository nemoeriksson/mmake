/**
 * Contains the implementation for the mmake algorithm. This program can 
 * be used as a simple version of the 'make' command. 
 *
 * The program supports the use of the optional flags
 *  -s			: Runs the program but does not print the commands ran to stdout,
 *  -B			: Force rebuiling all targets and their prerequisites,
 *  -f FILENAME	: Parses and builds using [FILENAME] instead of "mmakefile"
 *
 * When running the program it is also possible to specify which targets
 * to build, if none are specified the first target found in the make file 
 * will be used.
 *
 * Usage:
 *  ./mmake [-f FILENAME] [-s] [-B] [TARGETS ...]
 *
 * @file mmake.c
 * @author c24nen
 * @date 2025.09.24
 */

#include "program_handler.h"
#include "builder.h"

int main(int argc, char **argv)
{
	// Get info about flags
	programinfo *pinfo = get_program_info(argc, argv);
	if (pinfo == NULL)
		exit(EXIT_FAILURE);

	// Get info about which rules to build if necessary
	targetruleinfo *trinfo = get_argument_target_rules(argc, argv);
	if (trinfo == NULL)
		trinfo = get_default_target_rule(pinfo);
	
	// Check that the specifed rules are valid
	if (validate_rules(pinfo, trinfo) == 1)
	{
		free_target_rules(&trinfo);
		exit(EXIT_FAILURE);
	}

	// Build only necessary rules 
	if (build_required_rules(pinfo, trinfo) == 1)
	{
		free_target_rules(&trinfo);
		free_program_info(&pinfo);
		exit(EXIT_FAILURE);
	}

	// Clean up dynamically allocated memory
	free_target_rules(&trinfo);
	free_program_info(&pinfo);

	exit(EXIT_SUCCESS);
}

