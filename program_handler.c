/**
 * The program handler handles all functionality related
 * to the setup step of the 'mmake' program. This includes
 * reading program arguments, parsing flags, and getting
 * the targetted make rules (if any).
 *
 * @file program_handler.c
 * @author c24nen
 * @date 2025.09.24
 */

#include "program_handler.h"

#include "parser.h"
#define MAX_FILENAME_LEN 256

typedef struct optioninfo {
	int silence_commands;
	int force_rebuild;
	int custom_targets;
	char *makefile_name;
} optioninfo;

optioninfo *get_option_info(int argc, char **argv)
{
	optioninfo *options = malloc(sizeof(*options));

	if (options == NULL)
	{
		perror("Allocation failed");
		return NULL;
	}

	options->silence_commands = 0;
	options->force_rebuild = 0;
	options->custom_targets = 0;
	options->makefile_name = NULL;

	int uses_custom_makefile = 0;
	int opt = 0;

	// Check flags
	while ((opt = getopt(argc, argv, "sBf:")) != -1)
	{
		switch(opt)
		{
			case 's':
				options->silence_commands = 1;
				break;
			case 'B':
				options->force_rebuild = 1;
				break;
			case 'f':
				uses_custom_makefile = 1;
				options->makefile_name = strdup(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s [-f FILENAME] -s -B\n", argv[0]);
				free_option_info(&options);
				return NULL;
		}
	}

	// Use default filename if custom makefile name wasn't specified
	if (!uses_custom_makefile)
		options->makefile_name = strdup("mmakefile");

	// Check if targets have been given
	if (argc - 1 > optind)
		options->custom_targets = 1;

	return options;
}

int uses_flag(optioninfo *options, flagtype flag)
{
	switch (flag)
	{
		case SILENCE_COMMANDS:
			return options->silence_commands;
		case FORCE_REBUILD:
			return options->force_rebuild;
		case CUSTOM_TARGETS:
			return options->custom_targets;
	}

	return 0;
}

makefile *get_makefile(optioninfo *options)
{
	FILE *fptr = fopen(options->makefile_name, "r");

	if (fptr == NULL)
	{
		fprintf(stderr, "Coudln't open '%s'", options->makefile_name);
		return NULL;
	}

	makefile *mfile = parse_makefile(fptr);

	fclose(fptr);

	if (mfile == NULL)
	{
		fprintf(stderr, "Coudln't parse '%s'\n", options->makefile_name);
		return NULL;
	}

	return mfile;
}

void free_option_info(optioninfo **options_ptr)
{
	free((*options_ptr)->makefile_name);
	free(*options_ptr);
	*options_ptr = NULL;
}

int validate_targets(makefile *mfile, char **targets)
{
	int i = -1;
	while (targets[++i])
	{
		if (makefile_rule(mfile, targets[i]) == NULL)
			return 1;
	}

	return 0;
}

