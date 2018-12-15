/*
 * @file project.c
 * @author sb
 * @brief source file for project header
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

void p_setup_struct(struct project * restrict p)
{
	if (!p) {
		printf("No structure provided\n");
		return;
	}

	p->ptype = C;
	p->rdp_t = false;
	p->cwd = NULL;
	p->prwd = NULL;
	p->src = NULL;
	p->inc = NULL;
	p->nfiles = 0;
}
