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
			"res_dir_location=<absolute_path>/res/\n");
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

/* header functions */
void p_display_usage(void)
{
	printf("Usage of the program:\n"
			"<program> -t <type> <name_of_project>\n"
			"-t		type of the project\n"
			"-v		display version information\n"
			"-h		display help information\n"
			"-c		display config file help information\n"
			"-l		display the list of project types\n"
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
		/* else read the file and get the resource directory details
		 * need to write a function for reading and getting the details
		 * of the file
		 * For now add the location of the resource directory
		 * process/parse the configuration location */

		printf("Resource location : %s\n", p->resd);
	} else {
		p_write_file(cl, NULL);
	}

	/* free the resource */
	free(cl);
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

	/* new fields */
	free(p->resd);

	free(p->cwd);
	free(p->inc);
	free(p->prwd);
	free(p->pt);
	free(p->src);
}

int p_check_config_dir(const char *cl)
{
	/* return value */
	int r = 0;

	/* check if the mkproject directory exists or not */
	if (!p_dir_exists(cl)) {
		if (mkdir(cl, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
			r = 0;
		}
	} else {
		r = 1;
	}

	return r;
}

