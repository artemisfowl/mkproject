/**
 * @file 	project.h
 * @author 	sb
 * @brief 	project file holding all the atomic functions
 */

#ifndef PROJECT_H
#define PROJECT_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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

#ifndef USER_HOME
#define USER_HOME "HOME"
#endif

#ifndef CONFIG_LOC
#define CONFIG_LOC "/.config/mkproject/"
#endif

#ifndef CONFIG_FILE
#define CONFIG_FILE "mkpconfig"
#endif

#ifndef CONFIG_DELIM
#define CONFIG_DELIM "="
#endif

#ifndef CONFIG_VAR
#define CONFIG_VAR "res_dir_location"
#endif

/* enum */
enum project_t {	/* this might be deprecated later */
	C,
	CPP,
	UNRECOGNIZED,
	nentries = 3
};

/* structure */
struct project {
	enum project_t ptype;
	int rdp_t;
	char *pt;	/* project type name - dynamicity is the purpose */
	char *resd;	/* resource directory location */

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
 * @function p_setup
 * @brief function to initialize the fields for the struct project
 * @params [in] p is a pointer to a struct project instance
 */
void p_setup(struct project * restrict p);

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

/*
 * @function p_assign_ptype
 * @brief function to parse and assign the project type
 * @params [in] s pointer to the project type string
 * @params [in] p is a pointer to a struct project instance
 */
void p_assign_ptype(const char * restrict s, struct project * restrict p);

/**
 * @function p_free_resources
 * @brief function to free the resources allocated
 */
void p_free_res(struct project * restrict p);

/**
 * @function p_read_config
 * @brief function to read the configuration file
 * @notes The configuration file will be present in the $HOME/.config/mkproject
 * location
 */
int p_check_config_dir(const char *cl);

/*
 * @function p_get_resd_loc
 * @brief function to get the resource directory location
 * @params [in] p is a pointer to a struct project instance
 */
void p_get_resd_loc(struct project * restrict p);

#endif
