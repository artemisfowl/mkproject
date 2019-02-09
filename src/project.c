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
        FILE *f = fopen(fp, "r");
        if (!f) {
                perror("Template file doesn't exist\n");
                return NULL;
        }

        buf = calloc(p_get_filesize(fp) + 1, sizeof(char));

        char *t = NULL;
        ssize_t nread = 0;
        size_t n = 0;

        while ((nread = getline(&t, &n, f)) != -1)
                buf = strcat(buf, t);

        free(t);
        fclose(f);
        return buf;
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

int p_setup(struct project * restrict p)
{
        /*
         * 1 -> failure
         * 0 -> success
         */
        if (!p) {
                printf("No structure provided\n");
                return 1;
        }

        p->rdp_t = false;
        p->pt = NULL;
        p->resd = NULL;
        p->pdn = NULL;

        return 0;
}

int p_parse_flags(const char * restrict s, struct project * restrict p)
{
        /*
         * 0 -> success
         * 1 -> failure
         */
        if (strlen(s) != FLAG_LEN || !p) {
                printf("Length of arg_str is not proper or the "
                                "project structure instance not provided\n");
                return 1;
        }

        if (*s == '-')
                s++;	/* increment to point to the flag chars */
        switch(*s) {
                case 'h':
                        p_display_usage();
                        return 1;
                case 'v':
                        p_display_version();
                        return 1;
                case 'c':
                        p_display_config_help();
                        return 1;
                case 't':
                        p->rdp_t = true;
                        return 0;
                default:
                        printf("Unrecognised option\n");
                        p_display_usage();
                        return 1;
        }
}

void p_display_version(void)
{
        printf("mkproject version : %d.%d\n", VER_MAJ_NUM, VER_MIN_NUM);
}

int p_assign_ptype(const char * restrict s, struct project * restrict p)
{
        if (!s && !p) {
                printf("Type of project or "
                                "project struct instance not provided\n");
                return 1;
        }

        p->pt = strdup(s);
        return 0;
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
}

int p_check_config_dir(const char *cl)
{
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

int p_get_resd_loc(struct project * restrict p)
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

        if (access(cl, F_OK) != -1) {
                /* file exists */
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
                /* file doesn't exist - create an empty file */
                p_write_file(cl, NULL);
                free(cl);
                return 1;
        }

        free(cl);
        return 0;
}

int p_jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
        if (tok->type == JSMN_STRING &&
                        (int)strlen(s) == tok->end - tok->start &&
                        strncmp(json + tok->start, s,
                                tok->end - tok->start) == 0)
                return 0;
        return -1;
}

int p_get_tokenc(const char *s) /* still a un-used function */
{
        if (!s) {
                printf("JSON string has not been provided\n");
                return 0;
        }

        jsmn_parser jp;
        jsmn_init(&jp);
        return jsmn_parse(&jp, s, strlen(s), NULL, 0);
}

jsmntok_t p_get_token_value(const char *jsd, const char *tok_name)
{
        jsmntok_t r;
        r.end = 0;
        r.size = 0;
        r.start = 0;
        r.type = JSMN_UNDEFINED;

        if (!jsd && !tok_name) {
                printf("JSON data and/or token name has nto been provided\n");
                return r;
        }

        /* start the processing and return the value to the calling function */
        jsmn_parser jp;
        jsmn_init(&jp);
        int nt = p_get_tokenc(jsd);
        if (!nt) {
                printf("No tokens found\n");
                return r;
        }
        jsmntok_t t[nt];
        nt = jsmn_parse(&jp, jsd, strlen(jsd), t, nt);
        if (nt < 1 || t[0].type != JSMN_OBJECT) {
                printf("Structure of the JSON object is not proper\n");
                return r;
        }
        for (int i = 1; i < nt; i++) {
                if (p_jsoneq(jsd, &t[i], tok_name) == 0) {
                        r = t[i + 1];
                        break;
                }
        }

        return r;
}

void p_strsplice(const char *s, char *a, int start, int end)
{
        for (int m = start, l = 0; m < end; m++, l++)
                a[l] = s[m];
        a[end - start] = '\0';
}

void p_parse_jsdata(const char *jsd, struct project * restrict p)
{
        if (!jsd && !p) {
                printf("JSON data and/or project structure instance"
                                " has not been provided\n");
                return;
        }

        printf("\nJSON data received : %s\n", jsd);

        /* directories */
        jsmntok_t tok_bdirs = p_get_token_value(jsd, TEMPL_DIR_ID);
        char bdir_str[tok_bdirs.end - tok_bdirs.start + 1];
        memset(bdir_str, 0, tok_bdirs.end - tok_bdirs.start + 1);
        p_strsplice(jsd, bdir_str, tok_bdirs.start, tok_bdirs.end);
        p_process_bdirs(bdir_str, p);

        /* now get the build files to be processed */
        jsmntok_t tok_bfiles = p_get_token_value(jsd, TEMPL_BUILD_ID);
        char bfiles_str[tok_bfiles.end - tok_bfiles.start + 1];
        memset(bfiles_str, 0, tok_bfiles.end - tok_bfiles.start + 1);
        p_strsplice(jsd, bfiles_str, tok_bfiles.start, tok_bfiles.end);
        p_process_bfiles(bfiles_str, p);
}

int p_process_bfiles(const char *s, struct project * restrict p)
{
        /*
         * 1 -> success
         * 0 -> failure
         */
        if (!s && !p) {
                printf("JSON data and/or project structure instance"
                                " has not been provided\n");
                return 0;
        }
        printf("Build files JSON : %s\n", s);

        int nt = p_get_tokenc(s);
        if (!nt) {
                printf("No tokens found\n");
                return 0;
        }
        jsmn_parser jp;
        jsmn_init(&jp);
        jsmntok_t t[nt];
        nt = jsmn_parse(&jp, s, strlen(s), t, nt);
        if (nt < 1 || t[0].type != JSMN_OBJECT) {
                printf("Structure of the JSON object is not proper\n");
                return 0;
        }

        /*
         * all the files for each project type has to be placed in the same
         * resource directory under the name of the project type. So, in order
         * for the resources of C to be copied, the files have to be placed
         * inside the <res_dir_path>/c/<files_here_specific_to_C_json>
         */

        for (int i = 1; i < nt; i += 2) {
                /* key == k, value == v */
                char k[t[i].end - t[i].start + 1];
                char v[t[i + 1].end - t[i + 1].start + 1];

                /* splice the string */
                p_strsplice(s, k, t[i].start, t[i].end);
                p_strsplice(s, v, t[i + 1].start, t[i + 1].end);

                /* print the key, value pair */
                char src[strlen(p->resd) + strlen(p->pt) + strlen(k) + 2];
                memset(src, 0, strlen(p->resd) + strlen(p->pt) + strlen(k) + 2);
                strcat(src, p->resd);
                strcat(src, p->pt);
                strcat(src, "/");
                strcat(src, k);

                /* implement check for the directory which will be destination
                 * - this will be only required for the one which is not going
                 *   to the root directory*/

                /* first form the source filepath */
                if (!strcmp(v, ROOT_DIR)) {
                        char dest[strlen(p->pdn) + strlen(k) + 2];
                        memset(dest, 0, strlen(p->pdn) + strlen(k) + 2);
                        strcat(dest, p->pdn);
                        strcat(dest, "/");
                        strcat(dest, k);

                        p_copy_file(src, dest);
                } else {
                        printf("Source file : %s\n", src);
                        /* implement directory check */
                        char dest_dir[strlen(p->pdn) + strlen(v) + 3];
                        memset(dest_dir, 0, strlen(p->pdn) + strlen(v) + 3);
                        strcat(dest_dir, p->pdn);
                        strcat(dest_dir, "/");
                        strcat(dest_dir, v);
                        strcat(dest_dir, "/");
                        dest_dir[strlen(p->pdn) + strlen(v) + 3] = '\0';

                        if (!p_dir_exists(dest_dir)) {
                                printf("%s : directory not added in the dirs"
                                                " list\n",
                                                dest_dir);
                                return 0;
                        }

                        char dest[strlen(p->pdn) + strlen(v) + strlen(k) + 3];
                        memset(dest, 0, strlen(p->pdn) + strlen(v) +
                                        strlen(k) + 3);
                        strcat(dest, p->pdn);
                        strcat(dest, "/");
                        strcat(dest, v);
                        strcat(dest, "/");
                        strcat(dest, k);

                        p_copy_file(src, dest);
                }
        }

        return 1;
}

void p_copy_file(const char *src, const char *dest)
{
        /*
         * implement the checks whether the source file exists or not
         * if it doesn't - exit the program
         */
        if (!src && !dest) {
                printf("Either source location or destination location has "
                                "not been provided\n");
                return;
        }

        FILE *sfile = fopen(src, "rb");
        if (!sfile) {
                perror("fopen failed");
                return;
        }
        /* check if the dir exists - if not - return the control from that
         * check */
        FILE *dfile = fopen(dest, "ab");
        void *d = NULL;

        /* write the portion of the code for copying */
        if (sfile && dfile) {
                size_t n = p_get_filesize(src);
                d = calloc(n, sizeof(size_t));
                if (fread(d, sizeof(char), n , sfile) != n)
                        perror("File reading had errors");
                (void)fwrite(d, sizeof(char), n, dfile);
        }

        fclose(dfile);
        fclose(sfile);
        free(d);
}

int p_process_bdirs(const char *s, struct project * restrict p)
{
        /*
         * 1 -> success
         * 0 -> failure
         */
        if (!s && !p) {
                printf("JSON data and/or project structure instance"
                                " has not been provided\n");
                return 0;
        }

        /* JSON data to be parsed */
        printf("Directory string : %s\n", s);

        /* starting to reparse the input string */
        int nt = p_get_tokenc(s);
        if (!nt) {
                printf("No tokens found\n");
                return 0;
        }
        printf("Number of tokens found : %d\n", nt);

        jsmn_parser jp;
        jsmn_init(&jp);
        jsmntok_t t[nt];
        nt = jsmn_parse(&jp, s, strlen(s), t, nt);
        if (nt < 1 || t[0].type != JSMN_ARRAY) {
                printf("Structure of the JSON object is not proper\n");
                return 0;
        }

        for (int i = 1; i < nt; i++) {
                char dname[t[i].end - t[i].end + 1];
                memset(dname, 0, t[i].end - t[i].end + 1);
                p_strsplice(s, dname, t[i].start, t[i].end);
                char dpath[strlen(p->pdn) + strlen(dname) + 2];
                memset(dpath, 0, strlen(p->pdn) + strlen(dname) + 2);
                strcat(dpath, p->pdn);
                strcat(dpath, "/");
                strcat(dpath, dname);
                p_create_dir(dpath);
        }

        return 1;
}

void p_read_template(struct project * restrict p)
{
        char fp[strlen(p->resd) + strlen(p->pt) + strlen(RES_EXTENSION) + 1];
        memset(fp, 0, strlen(p->resd) + strlen(p->pt) +
                        strlen(RES_EXTENSION) + 1);

        strcat(fp, p->resd);
        strcat(fp, p->pt);
        strcat(fp, RES_EXTENSION);

        char *jsnd = NULL;
        jsnd = p_read_file(fp, jsnd);

        /* what happens when the template file is written in a wrong way? */

        if (jsnd)
                p_parse_jsdata(jsnd, p);

        free(jsnd);
}

void p_mkproject(struct project * restrict p)
{
        if (p_dir_exists(p->pdn)) {
                printf("Dir exists\n");
        } else {
                p_create_dir(p->pdn);
        }
        p_read_template(p);
}

