/**
 * @file 	main.c
 * @author 	sb
 * @brief 	mkproject program main function
 */

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

	/* decrement the arg count so that we do not handle the name of the
	 * program and increment the arg value pointer to point to the
	 * first argument instead of the name of the program in argv[0] */
	argc--;
	argv++;

	struct project p;
	p_setup_struct(&p);

	/* read the configuration file */
	if (p_read_config() == 1) {
		printf("Could not create the config directory\n");
		printf("Config directory is supposed to be created at "
				"~/.config/mkproject\n");
		p_free_res(&p);
		exit(EXIT_FAILURE);
	}

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
	printf("Name of the project : %s\n", *argv);

	/* free the resources before exiting */
	p_free_res(&p);

	return 0;
}
