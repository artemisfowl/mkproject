/**
 * @file 	project.h
 * @author 	sb
 * @brief 	project file holding all the atomic functions
 */

#ifndef PROJECT_H
#define PROJECT_H

#include <stdbool.h>
#include <stdlib.h>

/* macros */
#ifndef MAX_ARGS
#define MAX_ARGS 4
#endif

#ifndef MIN_ARGS
#define MIN_ARGS 2
#endif

#ifndef FLAG_LEN
#define FLAG_LEN 2
#endif

/* enum */
enum project_t {	/* this might be deprecated later */
	C,
	CPP,
	nentries = 2
};

/* structure */
struct project {
	enum project_t ptype;
	int rdp_t;

	/* directory holding fields */
	char *cwd;
	char *prwd;
	char *src;
	char *inc;
	int nfiles;	/* number of files to be copied */
};

/**
 * @function p_display_usage
 * @brief function to display the usage information of the program
 */
void p_display_usage(void);

/**
 * @function p_setup_struct
 * @brief function to initialize the fields for the struct project
 * @params [in] p is a pointer to a struct project instance
 */
void p_setup_struct(struct project * restrict p);

/**
 * @function p_parse_flags
 * @brief function to parse the flags provided as CLI argument
 * @params [in] s is a pointer to the string containing the value of flags
 * @params [in] p is a pointer to a struct project instance
 */
void p_parse_flags(const char * restrict s, struct project * restrict p);

/**
 * @function p_display_version
 * @brief function to display the version information of the program
 */
void p_display_version(void);

/**
 * @function p_list_ptypes
 * @brief function to display the list of projects supported by the program
 */
void p_list_ptypes(void);

#endif
