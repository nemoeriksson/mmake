#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "parser.h"

#define MAX_FILENAME_LEN 256
#define MAX_RULE_LEN 256

typedef struct {
	int silence_commands;
	int force_rebuild;
	makefile *makef;
} programinfo;

programinfo *get_program_info(int argc, char **argv);
void free_program_info(programinfo **pinfoptr);
void free_target_rules(char ***target_rules_ptr, int *target_rules_count_ptr);
int check_rule_build(programinfo *pinfo, const char *target);
int edited_sooner(struct timespec time1, struct timespec time2);
struct timespec get_last_mod_time(const char *filename);
int file_exists(const char *filename);
void print_command(char **cmd);

int main(int argc, char **argv)
{
	programinfo *pinfo = get_program_info(argc, argv);
	if (pinfo == NULL)
		exit(EXIT_FAILURE);

		
	// * Read goals
	int target_rule_count = argc - optind;
	char **target_rules = NULL;

	// Use default goal
	if (target_rule_count == 0)
	{
		target_rule_count = 1;
		target_rules = malloc(sizeof(*target_rules));
		target_rules[0] = strndup(makefile_default_target(pinfo->makef), MAX_RULE_LEN);
	}

	// Use goals given as program arguments
	else
	{
		target_rules = malloc(sizeof(*target_rules)*target_rule_count);
		for (int i = 0; i < target_rule_count; i++)
		{
			target_rules[i] = strndup(argv[optind + i], MAX_RULE_LEN);
			if (makefile_rule(pinfo->makef, target_rules[i]) == NULL)
			{
				fprintf(stderr, "Rule '%s' not found\n", target_rules[i]);
				return 1;
			}
		}
	}

	for (int i = 0; i < target_rule_count; i++)
	{
		if (check_rule_build(pinfo, target_rules[i]) != 0)
		{
			free_target_rules(&target_rules, &target_rule_count);
			free_program_info(&pinfo);
			exit(EXIT_FAILURE);
		}
	}

	free_target_rules(&target_rules, &target_rule_count);
	free_program_info(&pinfo);

	exit(EXIT_SUCCESS);
}

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
		fprintf(stderr, "Failed to parse makefile '%s'\n", make_filename);
		return NULL;
	}

	return pinfo;
}

void free_program_info(programinfo **pinfoptr)
{
	if ((*pinfoptr)->makef != NULL)
		makefile_del((*pinfoptr)->makef);

	free(*pinfoptr);
	*pinfoptr = NULL;
}

void free_target_rules(char ***target_rules_ptr, int *target_rules_count_ptr)
{
	for (int i = 0; i < *target_rules_count_ptr; i++)
		free((*target_rules_ptr)[i]);

	free(*target_rules_ptr);
	*target_rules_ptr = NULL;
	
	*target_rules_count_ptr = 0;
}

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
	struct stat fileinfo;
	if (stat(filename, &fileinfo) != 0)
		return (struct timespec){0, 0};
		
	return fileinfo.st_mtim;
}

int edited_sooner(struct timespec time1, struct timespec time2)
{
	if (time1.tv_sec > time2.tv_sec) 
		return 1;

	if (time1.tv_sec  == time2.tv_sec)
	{
		if (time1.tv_nsec > time2.tv_nsec)
			return 1;
	}

	return 0;
}

void print_command(char **cmd)
{
	int i = 0;
	while (cmd[i])
		fprintf(stdout, "%s ", cmd[i++]);
	
	fprintf(stdout, "\n");
}

int check_rule_build(programinfo *pinfo, const char *target)
{
	if (target == NULL) return 0;
	int target_exists = file_exists(target);

	rule *ruleptr = makefile_rule(pinfo->makef, target);
	if (ruleptr == NULL) 
	{
		if (!target_exists)
		{
			fprintf(stderr, "A rule for '%s' does not exist\n", target);
			return 1;
		}
		return 0;
	}
	int should_rebuild = pinfo->force_rebuild;
	const char **prereqs = rule_prereq(ruleptr);

	// Build prerequisites recursively
	int i = 0;
	while (prereqs[i] != NULL)
	{
		if (check_rule_build(pinfo, prereqs[i]) != 0)
			return 1;
		
		i++;
	}

	struct timespec target_mod_time = get_last_mod_time(target);

	i = 0;
	while (prereqs[i] != NULL)
	{
		if (!file_exists(prereqs[i]))
		{
			should_rebuild = 1;
			break;
		}

		struct timespec prereq_mod_time = get_last_mod_time(prereqs[i]);
		if (!target_exists || edited_sooner(prereq_mod_time, target_mod_time))
		{
			should_rebuild = 1;
			break;
		}

		i++;
	}

	// Run rebuild logic
	if (should_rebuild)
	{
		char **cmd = rule_cmd(ruleptr);
		pid_t pid = fork();
		
		if (!pinfo->silence_commands)
			print_command(cmd);

		// Child process logic
		if (pid == 0)
		{
			execvp(cmd[0], cmd);
			perror("execvp failed");
			return 1;
		}
		
		// Parent process logic
		int status = -1;
		waitpid(pid, &status, 0);

		//TODO: Validate status of child process
	}

	return 0;
}

