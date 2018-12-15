/**
 * @file main.c
 * @author sb
 * @brief mkproject program main function
 */

#include <stdio.h>
#include <stdlib.h>
#include "../inc/project.h"

int main(int argc, char *argv[])
{
	if (argc < MIN_ARGS || argc > MAX_ARGS) {
		printf("Error in number of arguments\n");
		p_display_usage();
		exit(0);
	}

	/* decrement the arg count so that we do not handle the name of the
	 * program and increment the arg value pointer to point to the
	 * first argument instead of the name of the program in argv[0] */
	argc--;
	argv++;

	/* create an object/instance of the structure */

	/* initialize the program */

	/* count till argc drops */
	while (argc--) {
		/* call the function to parse the flags */
	}

	return 0;
}
