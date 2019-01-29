/**
 * @file 	main.c
 * @author 	sb
 * @brief 	mkproject program main function
 */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include "../inc/project.h"

/*
 * TODO : Write a code for checking whether the format of the JSON file is
 * correct or not
 */

int main(int argc, char *argv[])
{
	if (argc < MIN_ARGS || argc > MAX_ARGS) {
		printf("Error in number of arguments\n");
		p_display_usage();
		exit(EXIT_FAILURE);
	} else if ((argc == MIN_ARGS && strcmp(argv[MIN_ARGS - 1], "-t") == 0)
			|| (argc == MIN_ARGS + 1)) {
		printf("Expected project type and project name\n");
		p_display_usage();
		exit(EXIT_FAILURE);
	}

	/* decrement the arg count so that we do not handle the name of the
	 * program and increment the arg value pointer to point to the
	 * first argument instead of the name of the program in argv[0] */
	argc--;
	argv++;

	struct project p;
	p_setup(&p);


	/* count till argc drops */
	while (argc--) {
		/* put a check if -t is the last argument or not */
		p_parse_flags(*argv, &p);
		argv++;
		if (p.rdp_t) {
			p_assign_ptype(*argv, &p);
			p.rdp_t = false;

			/* point to the project name and then exit the loop */
			argv++;
			break;
		}
	}

	/* read the configuration file and store the resource directory
	 * location */
	p_get_resd_loc(&p);

	/* now based on the project type, read the specific json file and then
	 * copy the necessary files - this is where the code for parsing the
	 * JSON files need to be polished more - working on this today */

	/* save the name of the project in the project structure */
	p.pdn = strdup(*argv);
	printf("Name of the project : %s\n", p.pdn);

	/* making the call to the final function */
	p_mkproject(&p);

	/* free the resources before exiting */
	p_free_res(&p);

	return 0;
}
