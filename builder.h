#pragma once

/**
 * The builder has the main responsibility for checking
 * and building the required makefile rules using data
 * from both the file handler and program handler.
 *
 * @file builder.h
 * @author c24nen
 * @date 2025.09.24
 */

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "program_handler.h"
#include "file_handler.h"

/**
 * Iterates through all rules and checks if they need to be
 * built or not (i.e. have been updated). Will return 0 on
 * success even if no building has taken place (no changes
 * to prerequisites).
 *
 * @return		0 on success, 1 if any step in the building
 *				process fails.
 */
int build_required_rules(programinfo *pinfo, targetruleinfo *trinfo);

