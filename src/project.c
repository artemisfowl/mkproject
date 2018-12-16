/*
 * @file 	project.c
 * @author 	sb
 * @brief 	source file for project header
 */


#include <stdio.h>
#include "../inc/project.h"
#include "../inc/version.h"

void p_display_usage(void)
{
	printf("Usage of the program:\n"
			"<program> -t <type> <name_of_project>\n"
			"-t		type of the project\n"
			"-v		display version information\n"
			"-h		display help information\n"
			"-l		display the list of project types\n"
			"For example, in order to create a C project\n"
			"mkproject -t c c_project_name\n");
}

void p_setup_struct(struct project * restrict p)
{
	if (!p) {
		printf("No structure provided\n");
		return;
	}

	p->ptype = C;
	p->rdp_t = false;
	p->pt = NULL;
	p->cwd = NULL;
	p->prwd = NULL;
	p->src = NULL;
	p->inc = NULL;
	p->nfiles = 0;
}

void p_parse_flags(const char * restrict s, struct project * restrict p)
{
	if (strlen(s) != FLAG_LEN || !p) {
		printf("Length of arg_str is not proper or the"
				"project structure instance not provided\n");
		return;
	}

	if (*s == '-')
		s++;	/* increment to point to the flag chars */
	switch(*s) {
		case 'h':
			p_display_usage();
			exit(EXIT_SUCCESS);
		case 'v':
			p_display_version();
			exit(EXIT_SUCCESS);
		case 'l':
			p_list_ptypes();
			exit(EXIT_SUCCESS);
		case 't':
			p->rdp_t = true;
			break;
		default:
			printf("Unrecognised option\n");
			p_display_usage();
			exit(EXIT_SUCCESS);
	}
}

void p_display_version(void)
{
	printf("mkproject version : %d.%d\n", VER_MAJ_NUM, VER_MIN_NUM);
}

void p_list_ptypes(void)
{
	for (int i = 0; i < nentries; i++) {
		switch (i) {
			case C:
				printf("%d. C\n", i + 1);
				break;
			case CPP:
				printf("%d. CPP\n", i + 1);
				break;
			case UNRECOGNIZED:
				printf("%d. UNRECOGNIZED\n", i + 1);
				break;
		}
	}
}

void p_assign_ptype(const char * restrict s, struct project * restrict p)
{
	if (!s && !p) {
		printf("Type of project or "
				"project struct instance not provided\n");
		return;
	}

	if (!strcmp(s, "c")) {
		p->ptype = C;
	} else if (!strcmp(s, "cpp")) {
		p->ptype = CPP;
	} else {
		p->ptype = UNRECOGNIZED;
		printf("Warning : Unrecognized project type provided\n");
	}
}

void p_free_res(struct project * restrict p)
{
	if (!p) {
		printf("Project struct instance not provided\n");
		exit(EXIT_FAILURE);
	}

	free(p->cwd);
	free(p->inc);
	free(p->prwd);
	free(p->pt);
	free(p->src);
}
