/*
 * @file utility.c
 * @author sb
 * @brief source file for utility header
 */


#include <stdio.h>
#include "../inc/project.h"

void p_display_usage(void)
{
	printf("Usage of the program:\n"
			"<program> -t <type> <name_of_project>\n"
			"-t		type of the project\n"
			"-v		display version information\n"
			"-h		display help information\n"
			"-l		display the list of project types\n");
}
