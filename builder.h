#pragma once

/**
 * The builder has the main responsibility for checking
 * and building the required makefile rules using data
 * from both the file handler and program handler.
 *
 * @file builder.h
 * @author c24nen
 * @date 2025.10.01
 */

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "parser.h"
#include "program_handler.h"
#include "file_handler.h"

/**
 * Handles logic related to checking if a target should be
 * built or not, and if so builds it. A rule will be rebuilt if:
 *  1. The target doesn't exit
 *  2. Any prerequisite has been updated sooner than the target
 *  3. The force rebuild flag has been specified
 *
 * This will be checked recursively for all prerequisites.
 *
 * @param options	Information about the program's flags
 * @param target	The target to build if necessary
 *
 * @return	0 on success, else 1.
 */
int check_target_build(optioninfo *options, makefile *mfile, const char *target);

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

