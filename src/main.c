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

	/*
	 * Before going ahead with getting the details from the CLI arguments 
	 * check if the .config directory exists or not.
	 * Add the function in the project module
	 */

	/* decrement the arg count so that we do not handle the name of the
	 * program and increment the arg value pointer to point to the
	 * first argument instead of the name of the program in argv[0] */
	argc--;
	argv++;

	struct project p;
	if (p_setup(&p)) {
                perror("p_setup failed\n");
		exit(EXIT_FAILURE);
        }

	/* count till argc drops */
	while (argc--) {
		if (p_parse_flags(*argv, &p)) {
                        perror("p_parse_flags failed\n");
                        exit(EXIT_FAILURE);
                }
		argv++;
		if (p.rdp_t) {
			if (p_assign_ptype(*argv, &p)) {
                                perror("p_assign_ptype failed\n");
                                exit(EXIT_FAILURE);
                        }
			p.rdp_t = false;

			/* point to the project name and then exit the loop */
			argv++;
			break;
		}
	}

        /* need a return type from this function */
	if (p_get_resd_loc(&p)) {
                /* failure case - dummy file has been created without any
                 * configuration data */
                printf("Configuration file has been created - empty content\n");
        } else {
                /* configuration file exists already - may have configuration
                 * data */
                p.pdn = strdup(*argv);
                p_mkproject(&p);
        }

	p_free_res(&p);
	return 0;
}
