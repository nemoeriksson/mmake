/**
 * The builder has the main responsibility for checking
 * and building the required makefile rules using data
 * from both the file handler and program handler.
 *
 * @file builder.c
 * @author c24nen
 * @date 2025.10.01
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
 * @param prereqs	A NULL-terminated list of the target's prerequisites 
 *
 * @return	1 if target doesn't exit of a prerequisite has updated,
 *			else 0.
 */
static int check_should_rebuild(const char *target, const char**prereqs)
{
	struct timespec target_mod_time = get_last_mod_time(target);

	// Check if get_last_mod_time() failed
	if (target_mod_time.tv_sec == -1)
		return 1;

	int i = -1;
	while (prereqs[++i] != NULL)
	{
		if (!file_exists(prereqs[i]))
			return 1;

		struct timespec prereq_mod_time = get_last_mod_time(prereqs[i]);
	
		// Check if get_last_mod_time() failed
		if (prereq_mod_time.tv_sec == -1)
			return 1;

		if (!file_exists(target) || edited_sooner(prereq_mod_time, target_mod_time))
			return 1;
	}

	return 0;
}

/**
 * Runs the building process for a given rule. Returns 0 on
 * success, else 1 if any step during the process fails.
 *
 * @param options	Information about the program's flags
 * @param ruleptr	Pointer to the rule to build
 *
 * @return		0 on success, else 1.
 */
static int build(optioninfo *options, rule *ruleptr)
{
	char **cmd = rule_cmd(ruleptr);

	if (!uses_flag(options, SILENCE_COMMANDS))
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
	if (child_status != 0)
		return 1;

	return 0;
}

// * Visible functions

int check_target_build(optioninfo *options, makefile *mfile, const char *target)
{
	if (target == NULL) return 0;
	int target_exists = file_exists(target);

	rule *ruleptr = makefile_rule(mfile, target);
	
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
	int i = -1;
	while (prereqs[++i] != NULL)
	{
		if (check_target_build(options, mfile, prereqs[i]) != 0)
			return 1;
	}

	// Check if this target needs to be rebuilt
	int should_rebuild = uses_flag(options, FORCE_REBUILD);

	if (check_should_rebuild(target, prereqs) == 1)
		should_rebuild = 1;

	// Run rebuild logic
	if (should_rebuild)
	{
		if (build(options, ruleptr) == 1)
			return 1;
	}

	return 0;
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

