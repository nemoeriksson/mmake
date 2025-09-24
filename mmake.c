#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "program_handler.h"
#include "file_handler.h"
#include "parser.h"

// Helper functions
int check_rule_build(programinfo *pinfo, const char *target);
void print_command(char **cmd);
int edited_sooner(struct timespec time1, struct timespec time2);

int main(int argc, char **argv)
{
	programinfo *pinfo = get_program_info(argc, argv);
	if (pinfo == NULL)
		exit(EXIT_FAILURE);

	targetruleinfo *trinfo = get_argument_target_rules(argc, argv);
	if (trinfo == NULL)
		trinfo = get_default_target_rule(pinfo);
	
	if (validate_rules(pinfo, trinfo) == 1)
	{
		free_target_rules(&trinfo);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < get_targetted_rule_count(trinfo); i++)
	{
		if (check_rule_build(pinfo, get_targetted_rule(trinfo, i)) == 1)
		{
			free_target_rules(&trinfo);
			free_program_info(&pinfo);
			exit(EXIT_FAILURE);
		}
	}

	free_target_rules(&trinfo);
	free_program_info(&pinfo);

	exit(EXIT_SUCCESS);
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
	{
		printf("%s", cmd[i]);
		if (cmd[i+1])
			printf(" ");

		i++;
	}
	
	printf("\n");
}

int check_rule_build(programinfo *pinfo, const char *target)
{
	if (target == NULL) return 0;
	int target_exists = file_exists(target);

	rule *ruleptr = makefile_rule(get_makefile(pinfo), target);
	if (ruleptr == NULL) 
	{
		if (!target_exists)
		{
			fprintf(stderr, "A rule for '%s' does not exist\n", target);
			return 1;
		}
		return 0;
	}
	int should_rebuild = uses_flag(pinfo, FORCE_REBUILD);
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
		if (!uses_flag(pinfo, SILENCE_COMMANDS))
			print_command(cmd);
		
		pid_t pid = fork();

		if (pid < 0)
		{
			perror("Fork failed");
			return 1;
		}

		// Child process logic
		if (pid == 0)
		{
			execvp(cmd[0], cmd);
			perror("execvp failed");
			return 1;
		}
		
		// Parent process logic
		int child_status = -1;
		waitpid(pid, &child_status, 0);

		if (WEXITSTATUS(child_status) != EXIT_SUCCESS)
			return 1;
	}

	return 0;
}

