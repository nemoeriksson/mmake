/**
 * The builder has the main responsibility for checking
 * and building the required makefile rules using data
 * from both the file handler and program handler.
 *
 * @file builder.c
 * @author c24nen
 * @date 2025.09.24
 */

#include "builder.h"
#include "program_handler.h"

// * Internal functions

/**
 * Prints a list of arguments as a command.
 *
 * @param cmd	A command as a list of arguments
 */
static void print_command(char **cmd)
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

/**
 * Checks if the first provided timespec is sooner than the other.
 *
 * @param time1	Timespec to compare
 * @param time2 The timespec to compare against
 *
 * @return		1 if time1 is sooner than time2, else 0.
 */
static int edited_sooner(struct timespec time1, struct timespec time2)
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

/**
 * Checks if any prerequisites have been updated sooner than the
 * target or if the target doesn't exist.
 *
 * @param target	The target to check
 * @param prereqs	A list of the target's prerequisites 
 *
 * @return	1 if target doesn't exit of a prerequisite has updated,
 *			else 0.
 */
static int check_should_rebuild(const char *target, const char**prereqs)
{
	struct timespec target_mod_time = get_last_mod_time(target);

	int i = 0;
	while (prereqs[i] != NULL)
	{
		if (!file_exists(prereqs[i]))
			return 1;

		struct timespec prereq_mod_time = get_last_mod_time(prereqs[i]);
		
		if (!file_exists(target) || edited_sooner(prereq_mod_time, target_mod_time))
			return 1;

		i++;
	}

	return 0;
}

/**
 * Runs the building process for a given rule. Returns 0 on
 * success, else 1 if any step during the process fails.
 *
 * @param pinfo		Information about the program
 * @param ruleptr	Pointer to the rule to build
 *
 * @return		0 on success, else 1.
 */
static int build(programinfo *pinfo, rule *ruleptr)
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

	// Validate child process exit status
	if (WEXITSTATUS(child_status) != EXIT_SUCCESS)
		return 1;

	return 0;
}

/**
 * Handles logic related to checking if a target should be
 * built or not, and if so builds it. A rule will be rebuilt if:
 *  1. The target doesn't exit
 *  2. Any prerequisite has been updated sooner than the target
 *  3. The force rebuild flag has been specified
 *
 * This will be checked recursively for all prerequisites.
 *
 * @param pinfo		Information about the program
 * @param target	The target to build if necessary
 *
 * @return	0 on success, else 1.
 */
static int check_rule_build(programinfo *pinfo, const char *target)
{
	if (target == NULL) return 0;
	int target_exists = file_exists(target);

	rule *ruleptr = makefile_rule(get_makefile(pinfo), target);
	
	if (ruleptr == NULL) 
	{
		// If the rule nor its file exists there is an error, else
		//	if the file exists the build process should continue.
		if (!target_exists)
		{
			fprintf(stderr, "A rule for '%s' does not exist\n", target);
			return 1;
		}
		return 0;
	}

	const char **prereqs = rule_prereq(ruleptr);

	// Build prerequisites recursively
	int i = 0;
	while (prereqs[i] != NULL)
	{
		if (check_rule_build(pinfo, prereqs[i]) != 0)
			return 1;
		
		i++;
	}

	// Check if this target needs to be rebuilt
	int should_rebuild = uses_flag(pinfo, FORCE_REBUILD);

	if (check_should_rebuild(target, prereqs) == 1)
		should_rebuild = 1;

	// Run rebuild logic
	if (should_rebuild)
	{
		if (build(pinfo, ruleptr) == 1)
			return 1;
	}

	return 0;
}

// * Visisble functions

int build_required_rules(programinfo *pinfo, targetruleinfo *trinfo)
{
	for (int i = 0; i < get_targetted_rule_count(trinfo); i++)
	{
		if (check_rule_build(pinfo, get_targetted_rule(trinfo, i)) == 1)
			return 1;
	}

	return 0;
}

