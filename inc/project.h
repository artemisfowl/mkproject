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
#include "../inc/jsmn.h"

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

#ifndef RES_EXTENSION
#define RES_EXTENSION ".json"
#endif

#ifndef TEMPL_DIR_ID
#define TEMPL_DIR_ID "dirs"
#endif

#ifndef TEMPL_BUILD_ID
#define TEMPL_BUILD_ID "build_files"
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
	char *pdn;	/* project directory name or the project name */

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

/**
 * @function p_get_resd_loc
 * @brief function to get the resource directory location
 * @params [in] p is a pointer to a struct project instance
 */
void p_get_resd_loc(struct project * restrict p);

/**
 * @function p_strsplice
 * @brief function to splice the given string within the given boundaries
 * @params [in] s is the string containing the substring to be spliced out
 * @params [in] a is the buffer in which the string has to be put in
 * @params [in] start is the starting index of the substring
 * @params [in] end is the ending index of the substring
 */
void p_strsplice(const char *s, char *a, int start, int end);

/**
 * @function p_parse_jsdata
 * @brief function to parse the json data and perform the operations on the
 * files and the directories specified
 * @params [in] jsd is the json data to be processed by this function
 * @params [in] p is a pointer to the project structure instance/object
 */
void p_parse_jsdata(const char *jsd, struct project * restrict p);

/**
 * @function p_jsoneq
 * @brief function to check if the tag is present in the JSON structure or not
 * @params [in] json is the JSON string
 * @params [in] tok is the token which will be matched
 * @params [in] s is the name of the tag to be matched
 *
 */
int p_jsoneq(const char *json, jsmntok_t *tok, const char *s);

/**
 * @function p_get_tokenc
 * @brief function to return the number of tokens found by the JSMN parser
 * @params [in] s is the JSON string for which the token count will be returned
 */
int p_get_tokenc(const char *s);

/**
 * @function p_process_bdirs
 * @brief function to create the directories from the configuration
 * @params [in] s is the string containing the names of the directories
 * @params [in] p is a pointer to the project structure instance/object
 */
int p_process_bdirs(const char *s, struct project * restrict p);

/**
 * @function p_process_bfiles
 * @brief function to copy the respective files from the configuration
 * @params [in] s is the string containing the names of the directories
 * @params [in] p is a pointer to the project structure instance/object
 */
int p_process_bfiles(const char *s, struct project * restrict p);

/**
 * @function p_read_template
 * @brief function to read the template file
 * @params [in] p is a pointer to a struct project instance
 */
void p_read_template(struct project * restrict p);

/**
 * @function mkproject
 * @brief function to create and copy the files - main handler
 * @params [in] p is a pointer to a struct project instance
 */
void p_mkproject(struct project * restrict p);

#endif
