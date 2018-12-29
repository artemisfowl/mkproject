/*
 * @file 	project.c
 * @author 	sb
 * @brief 	source file for project header
 */


#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "../inc/project.h"
#include "../inc/version.h"

/* static utility functions */
static bool p_dir_exists(const char *filepath)
{
	/*
	 * return false of the directory needs to be created
	 * return true if the directory already exists
	 */
	DIR *dir = opendir(filepath);
	if (!dir && ENOENT == errno) {
		closedir(dir);
		return false;
	}

	closedir(dir);
	return true;
}

static void p_write_file(const char * filepath, const char * d)
{
	FILE *f = fopen(filepath, "w");

	if (d)
		fwrite(d, strlen(d), sizeof(char), f);

	fclose(f);
}

static off_t p_get_filesize(const char * restrict filepath)
{
	struct stat s;
	int r = 0;
	if ((r = stat(filepath, &s)) != 0) {
		perror("Error with stat");
		return r;
	}

	return s.st_size;
}

static void p_display_config_help(void)
{
	printf("\nConfiguration can be added in the following way : \n");
	printf("# Resource directory configuration - all lines with # "
			"are comments\n"
			"res_dir_location=<absolute_path>/res/\n\n");
}

static char *p_read_config(const char * restrict fp)
{
	FILE *f = fopen(fp, "r");

	if (!f) {
		perror("fopen : couldn't open file - check filepath");
		return NULL;
	}

	char *c = NULL;
	char *ret = NULL;
	char *tok = NULL;
	size_t n = 0;
	while(getline(&c, &n, f) != -1) {
		if (*c != '#') {
			tok = strtok(c, CONFIG_DELIM);
			/* after reaching the first proper line - break out */
			break;
		}
	}

	for (int f = 0; tok; tok = strtok(NULL, CONFIG_DELIM)) {
		if (strcmp(tok, CONFIG_VAR) == 0) {
			f = 1;
		} else if (f) {
			ret = strndup(tok, strcspn(tok, "\n"));
			f = 0;
		}
	}

	free(c);
	fclose(f);

	return ret;
}

static int p_create_dir(const char *dp)
{
	if (!dp) {
		printf("Dir path has not been provided\n");
		return -1;
	}
	return mkdir(dp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

static char *p_read_file(const char * restrict fp, char *buf)
{
	buf = calloc(p_get_filesize(fp) + 1, sizeof(char));

	FILE *f = fopen(fp, "r");
	char *t = NULL;
	ssize_t nread = 0;
	size_t n = 0;

	while ((nread = getline(&t, &n, f)) != -1)
		buf = strcat(buf, t);

	free(t);
	fclose(f);
	return buf;
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
	if (tok->type == JSMN_STRING &&
				(int)strlen(s) == tok->end - tok->start &&
				strncmp(json + tok->start, s,
					tok->end - tok->start) == 0)
			return 0;
	return -1;
}

static void p_parse_json(const char * restrict jsd)
{
	if (!jsd) {
		printf("JSON data has nopt been provided");
		return;
	}

	/*
	 * As of now - not implementing complicated JSON handling - basic
	 * specific structure of JSON to be handled as of now
	 */

	/* process the JSON data - start working from here */
	int num_tok = 0;	/* number of tokens */
	jsmn_parser jp;

	/* initialize the parser */
	jsmn_init(&jp);

	/* get the number of the tokens */
	num_tok = jsmn_parse(&jp, jsd, strlen(jsd), NULL, 0);
	printf("Number of tokens found : %d\n", num_tok);

	/* re-init the parser - also write the json_eq function
	 * There seems to be a way to see if there are child nodes or not */
	jsmntok_t t[num_tok];
	jsmn_init(&jp);

	num_tok = jsmn_parse(&jp, jsd, strlen(jsd), t, num_tok);

	/* check if the first token type is of OBJECT or not */
	if (num_tok < 1 || t[0].type != JSMN_OBJECT) {
		printf("Object expected\n");
		return;
	}

	/* check the number of child elements that exist for the token type */
	for (int i = 1; i < num_tok; i++) {
		/*
		 * This is what needs to be followed:
		 * First check if the token matches with the string provided,
		 * if it matches, get the number of children. Now loop with
		 * this num_child and get the values out - finally process them
		 * in the same loop
		 */
#if 0
		/* get the length of the buffer */
		int len = t[i].end - t[i].start;

		/* splice the string here */
		char spliced[len];	/* reducing the dyn-mem-allocs */
		for (int m = t[i].start, l = 0; m <= t[i].end; m++, l++)
			spliced[l] = jsd[m];
		spliced[len] = '\0';

		printf("For string : %s --> Number of children : %d and"
				" index(i) value: %d\n",
				spliced, t[i].size, i);
#endif

		bool f_dir_id = false;
		bool f_bfiles_id = false;

		if (jsoneq(jsd, &t[i], TEMPL_DIR_ID) == 0) {
			printf("Found the %s node\n", TEMPL_DIR_ID);
			i++;
			f_dir_id = true;
		} else if (jsoneq(jsd, &t[i], TEMPL_BUILD_ID) == 0) {
			printf("Found the %s node\n", TEMPL_BUILD_ID);
			i++;
			f_bfiles_id = true;
		}

		/* based on the bool value check for the next values */
		if (f_dir_id) {
			/* call a function for splicing the string and then
			 * create the directories inside the project directory
			 * */
			printf("Inside check for dir_id\n");

			int pos = i + 1;
			for (int cn = 0; cn < t[i].size; cn++, pos++) {
				printf("%.*s\n", t[pos].end - t[pos].start,
						jsd + t[pos].start);
			}
			f_dir_id = false;
		} else if (f_bfiles_id) {
			/*
			 * call a function for splicing the string and then
			 * copy the respective files from the location
			 * mentioned to the project directory
			 */
			printf("Inside check for bfiles_id\n");

			int pos = i + 1;
			for (int cn = 0; cn < t[i].size; cn++, pos++) {
				printf("%.*s\n", t[pos].end - t[pos].start,
						jsd + t[pos].start);
			}
			f_dir_id = false;
		}

	}
}

/* header functions */
void p_display_usage(void)
{
	printf("Usage of the program:\n"
			"<program> -t <type> <name_of_project>\n"
			"-t		type of the project\n"
			"-v		display version information\n"
			"-h		display help information\n"
			"-c		display config file help information\n"
			"For example, in order to create a C project\n"
			"mkproject -t c c_project_name\n");
}

void p_setup(struct project * restrict p)
{
	if (!p) {
		printf("No structure provided\n");
		return;
	}

	/* too many strings - will be reducing them one by one */
	p->ptype = C;
	p->rdp_t = false;
	p->pt = NULL;
	p->cwd = NULL;
	p->prwd = NULL;
	p->src = NULL;
	p->inc = NULL;
	p->nfiles = 0;

	/* new fields */
	p->resd = NULL;
	p->pdn = NULL;
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
		case 'c':
			p_display_config_help();
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

void p_assign_ptype(const char * restrict s, struct project * restrict p)
{
	if (!s && !p) {
		printf("Type of project or "
				"project struct instance not provided\n");
		return;
	}

	/* just save the project type for later use */
	p->pt = strdup(s);
}

void p_free_res(struct project * restrict p)
{
	if (!p) {
		printf("Project struct instance not provided\n");
		exit(EXIT_FAILURE);
	}

	/* new fields */
	free(p->resd);
	free(p->pt);
	free(p->pdn);

	free(p->cwd);
	free(p->inc);
	free(p->prwd);
	free(p->src);
}

int p_check_config_dir(const char *cl)
{
	/* return value */
	int r = 0;

	/* check if the mkproject directory exists or not */
	if (!p_dir_exists(cl)) {
		if (p_create_dir(cl) != 0) {
			r = 0;
		}
	} else {
		r = 1;
	}

	return r;
}

void p_get_resd_loc(struct project * restrict p)
{
	char *h = getenv(USER_HOME);
	char *cl = calloc(strlen(h) + strlen(CONFIG_LOC) + 1, sizeof(char));
	cl = strcat(cl, h);
	cl = strcat(cl, CONFIG_LOC);

	if (p_check_config_dir(cl) == 1) {
		printf("Could not create the config directory\n");
		printf("Config directory may already be present at "
				"~/.config/mkproject\n");
	}

	cl = realloc(cl, (strlen(h) + strlen(CONFIG_LOC)
				+ strlen(CONFIG_FILE) + 1) * sizeof(char));
	cl = strcat(cl, CONFIG_FILE);
	printf("Config file final location : %s\n", cl);

	if (access(cl, F_OK) != -1) {
		if ((p_get_filesize(cl) == 0) ||
				!(p->resd = p_read_config(cl))) {
			printf("No configuration present in the file\n"
					"Nothing to create/copy\n");

			p_display_config_help();
			printf("\nProgram will now quit\n");
			free(cl);
			p_free_res(p);
			exit(EXIT_SUCCESS);
		}
	} else {
		p_write_file(cl, NULL);
	}

	/* free the resource */
	free(cl);
}


void p_read_template(struct project * restrict p)
{
	/* code will be added shortly */
	printf("Resource directory location from main func : %s\n", p->resd);

	/* project type has been saved already */
	printf("\nProject type as specified : %s\n", p->pt);

	/* read the specific template file - so if cpp is the type of
	 * project(pt field of the struct), read cpp.json file
	 * The JSON file should contain the complete path to the build files to
	 * be copied */

	char fp[strlen(p->resd) + strlen(p->pt) + strlen(RES_EXTENSION) + 1];
	memset(fp, 0, strlen(p->resd) + strlen(p->pt) +
			strlen(RES_EXTENSION) + 1);

	/* concatenate the values */
	strcat(fp, p->resd);
	strcat(fp, p->pt);
	strcat(fp, RES_EXTENSION);

	/* read the file and parse the json */
	char *jsnd = NULL;
	jsnd = p_read_file(fp, jsnd);

	/* print the data received */
	printf("JSON data : %s\n", jsnd);

	/* start by parsing the json file now and creating the directories as
	 * well as copying the necessary files - start from here */
	p_parse_json(jsnd);

	/* free this resource once this function is getting its call stack
	 * cleaned */
	free(jsnd);
}

void mkproject(struct project * restrict p)
{
	if (p_dir_exists(p->pdn)) {
		printf("Dir exists\n");
	} else {
		printf("Creating the directory\n");
		/* create the directory in the current working directory */
		p_create_dir(p->pdn);
	}
	p_read_template(p);	/* read template later */
}

