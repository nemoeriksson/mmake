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

#define MAX_FILENAME_LEN 256

typedef struct programinfo {
	int silence_commands;
	int force_rebuild;
	makefile *makef;
} programinfo;

typedef struct targetruleinfo {
	char **rule_targets;
	int rule_count;
} targetruleinfo;

programinfo *get_program_info(int argc, char **argv)
{
	programinfo *pinfo = malloc(sizeof(*pinfo));
	pinfo->silence_commands = 0;
	pinfo->force_rebuild = 0;
	pinfo->makef = NULL;

	int opt = 0;
	char make_filename[MAX_FILENAME_LEN] = "mmakefile\0";

	// Check flags
	while ((opt = getopt(argc, argv, "sBf:")) != -1)
	{
		switch(opt)
		{
			case 's':
				pinfo->silence_commands = 1;
				break;
			case 'B':
				pinfo->force_rebuild = 1;
				break;
			case 'f':
				strncpy(make_filename, optarg, MAX_FILENAME_LEN);
				break;
			default:
				fprintf(stderr, "Usage: %s [-f FILENAME] -s -B", argv[0]);
				free_program_info(&pinfo);
				return NULL;
		}
	}

	// Parse makefile
	FILE *fptr = fopen(make_filename, "r");
	
	if (fptr == NULL)
	{
		perror("Couldn't open specified makefile");
		free_program_info(&pinfo);
		return NULL;
	}

	pinfo->makef = parse_makefile(fptr);
	fclose(fptr);

	if (pinfo->makef == NULL)
	{
		free_program_info(&pinfo);
		fprintf(stderr, "Failed to parse makefile '%s'\n", make_filename);
		return NULL;
	}

	return pinfo;
}

int uses_flag(programinfo *pinfo, flagoption flag)
{
	switch (flag)
	{
		case SILENCE_COMMANDS:
			return pinfo->silence_commands;
		case FORCE_REBUILD:
			return pinfo->force_rebuild;
	}

	return 0;
}

makefile *get_makefile(programinfo *pinfo)
{
	return pinfo->makef;
}

void free_program_info(programinfo **pinfoptr)
{
	if ((*pinfoptr)->makef != NULL)
		makefile_del((*pinfoptr)->makef);

	free(*pinfoptr);
	*pinfoptr = NULL;
}

targetruleinfo *get_argument_target_rules(int argc, char **argv)
{
	targetruleinfo *trinfo = malloc(sizeof(*trinfo));

	trinfo->rule_count  = argc - optind;

	if (trinfo->rule_count == 0)
	{
		free(trinfo);
		return NULL;
	}

	trinfo->rule_targets = malloc(sizeof(*(trinfo->rule_targets)) * trinfo->rule_count);
	for (int i = 0; i < trinfo->rule_count; i++)
		trinfo->rule_targets[i] = strdup(argv[optind + i]);

	return trinfo;

}

targetruleinfo *get_default_target_rule(programinfo *pinfo)
{
	targetruleinfo *trinfo = malloc(sizeof(*trinfo));
	trinfo->rule_count = 1;

	trinfo->rule_targets = malloc(sizeof(*(trinfo->rule_targets)));
	trinfo->rule_targets[0] = strdup(makefile_default_target(pinfo->makef));

	return trinfo;
}

int get_targetted_rule_count(targetruleinfo *trinfo)
{
	return trinfo->rule_count;
}

char *get_targetted_rule(targetruleinfo *trinfo, int index)
{
	if (index > trinfo->rule_count-1 || index < 0)
		return NULL;

	return trinfo->rule_targets[index];
}

void free_target_rules(targetruleinfo **trinfo_ptr)
{
	for (int i = 0; i < (*trinfo_ptr)->rule_count; i++)
		free((*trinfo_ptr)->rule_targets[i]);

	free((*trinfo_ptr)->rule_targets);
	free(*trinfo_ptr);
	*trinfo_ptr = NULL;
}

int validate_rules(programinfo *pinfo, targetruleinfo *trinfo)
{
	for (int i = 0; i < trinfo->rule_count; i++)
	{
		if (makefile_rule(pinfo->makef, trinfo->rule_targets[i]) == NULL)
		{
			fprintf(stderr, "Rule '%s' not found\n", trinfo->rule_targets[i]);
			return 1;
		}
	}

	return 0;
}


