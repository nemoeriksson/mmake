/**
 * Contains the implementation for the mmake algorithm. This program can 
 * be used as a simple version of the 'make' command. 
 *
 * The program supports the use of the optional flags
 *  -s			: Runs the program but does not print the commands ran to stdout,
 *  -B			: Force rebuiling all targets and their prerequisites,
 *  -f FILENAME	: Parses and builds using [FILENAME], defaults to "mmakefile"
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
#include "parser.h"

/**
 * Frees all dynamically allocated memory from relevant instances
 * before exiting the program with the specified exit code.
 *
 * @param options	A pointer to information about the program's flags
 * @param mfile		A pointer to the type makefile
 * @param code		The exit code
 */
static void free_and_exit(optioninfo **options_ptr, makefile *mfile, int code)
{
	free_option_info(options_ptr);
	makefile_del(mfile);
	exit(code);
}

int main(int argc, char **argv)
{
	// Get info about flags
	optioninfo *options = get_option_info(argc, argv);
	if (options == NULL)
		exit(EXIT_FAILURE);

	// Parse the make file
	makefile *mfile = get_makefile(options);

	if (mfile == NULL)
	{
		free_option_info(&options);
		exit(EXIT_FAILURE);
	}

	// Check if specific targest were specified
	if (uses_flag(options, CUSTOM_TARGETS))
	{
		char **custom_targets = &argv[optind];
		
		// Validate that all targets exist in the make file
		if (validate_targets(mfile, custom_targets) == 1)
				free_and_exit(&options, mfile, EXIT_FAILURE);

		// Check the build of all specified targets
		int rule_index = -1;
		while(custom_targets[++rule_index])
		{
			if (check_target_build(options, mfile, custom_targets[rule_index]) == 1)
				free_and_exit(&options, mfile, EXIT_FAILURE);
		}
	}
	else // Build default target only
	{
		if (check_target_build(options, mfile, makefile_default_target(mfile)) == 1)
			free_and_exit(&options, mfile, EXIT_FAILURE);
	}

	// Clean up dynamically allocated memory
	free_and_exit(&options, mfile, EXIT_SUCCESS);
}

