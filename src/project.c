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

#if 0
static char *p_strsplice(const char *s, int start, int end)
{
        char *spliced = calloc(end - start + 1, sizeof(char));
        for (int m = start, l = 0; m < end; m++, l++)
                spliced[l] = s[m];
        spliced[end - start] = '\0';
        return spliced;
}

static void p_copy_file(const char * restrict src,
                const char * restrict dest)
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

static void p_process_bfiles(const char * restrict fname,
                struct project * restrict p)
{
        if (!fname && !p) {
                printf("Either filename or the project structure instance "
                                "have not been provided\n");
                return;
        }

        /* create the source filepath  */
        printf("Filename provided : %s\n", fname);

        /*
         * all the files for each project type has to be placed in the same
         * resource directory under the name of the project type. So, in order
         * for the resources of C to be copied, the files have to be placed
         * inside the <res_dir_path>/c/<files_here_specific_to_C_template>
         */

        /* create the source filepath */
        char sfpath[strlen(p->resd) + strlen(p->pt) + strlen(fname) + 1];
        memset(sfpath, 0, sizeof(char));

        strcat(sfpath, p->resd);
        strcat(sfpath, p->pt);
        strcat(sfpath, "/");
        strcat(sfpath, fname);

        /* check for the final name of the file */
        printf("Source filepath finally formed : %s\n", sfpath);

        /* start copying the files now */
        char dfpath[strlen(p->pdn) + strlen(fname) + 2];
        memset(dfpath, 0, sizeof(char));

        strcat(dfpath, p->pdn);
        strcat(dfpath, "/");
        strcat(dfpath, fname);

        printf("Destination filepath finally formed : %s\n", dfpath);

        p_copy_file(sfpath, dfpath);
}

static void p_parse_json(const char * restrict jsd,
                struct project * restrict p)
{
        if (!jsd) {
                printf("JSON data has not been provided");
                return;
        }

        if (!p) {
                printf("Project structure instance was not provided\n");
                return;
        }

        int num_tok = 0;
        jsmn_parser jp;

        jsmn_init(&jp);

        num_tok = jsmn_parse(&jp, jsd, strlen(jsd), NULL, 0);

        jsmntok_t t[num_tok];
        jsmn_init(&jp);

        num_tok = jsmn_parse(&jp, jsd, strlen(jsd), t, num_tok);

        if (num_tok < 1 || t[0].type != JSMN_OBJECT) {
                printf("Object expected\n");
                return;
        }

        for (int i = 1; i < num_tok; i++) {
                bool f_dir_id = false;
                bool f_bfiles_id = false;

                if (p_jsoneq(jsd, &t[i], TEMPL_DIR_ID) == 0) {
                        i++;
                        f_dir_id = true;
                } else if (p_jsoneq(jsd, &t[i], TEMPL_BUILD_ID) == 0) {
                        i++;
                        f_bfiles_id = true;
                }

                if (f_dir_id) {
                        int pos = i + 1;
                        for (int cn = 0; cn < t[i].size; cn++, pos++) {
                                char *splstr = p_strsplice(jsd,
                                                t[pos].start, t[pos].end);
                                p_process_bdirs(splstr, p);
                                free(splstr);
                        }
                        f_dir_id = false;
                } else if (f_bfiles_id) {
                        printf("Inside check for bfiles_id\n");

                        int pos = i + 1;
                        for (int cn = 0; cn < t[i].size; cn++, pos++) {
                                char *splstr = p_strsplice(jsd,
                                                t[pos].start, t[pos].end);
                                printf("After splicing of build files : %s\n",
                                                splstr);
                                p_process_bfiles(splstr, p);
                                free(splstr);
                        }
                        f_dir_id = false;
                }

        }
}
#endif


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

        free(cl);
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
        memset(bdir_str, 0, sizeof(char));
        //char *bdir_str = p_strsplice(jsd, tok_bdirs.start, tok_bdirs.end);
        p_strsplice(jsd, bdir_str, tok_bdirs.start, tok_bdirs.end);
        p_process_bdirs(bdir_str, p);

        /* now get the build files to be processed */
        jsmntok_t tok_bfiles = p_get_token_value(jsd, TEMPL_BUILD_ID);
        char bfiles_str[tok_bfiles.end - tok_bfiles.start + 1];
        memset(bfiles_str, 0, sizeof(char));
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
        printf("Number of tokens found : %d\n", nt);
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
        for (int i = 1; i < nt; i += 2) {
                /* key == k, value == v */
                char k[t[i].end - t[i].start + 1];
                char v[t[i + 1].end - t[i + 1].start + 1];

                /* splice the string */
                p_strsplice(s, k, t[i].start, t[i].end);
                p_strsplice(s, v, t[i + 1].start, t[i + 1].end);

                /* print the key, value pair */
                printf("Key : %s, Value : %s\n", k, v);

                /* copy the files now to the proper directories */
        }

        return 1;
}

void p_copy_file(const char * restrict src, const char * restrict dest)
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

        /* parse this input string and create the directories - start working
         * on this - addition of code will be here onward */

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
                memset(dname, 0, sizeof(char));
                p_strsplice(s, dname, t[i].start, t[i].end);
                char dpath[strlen(p->pdn) + strlen(dname) + 2];
                memset(dpath, 0, sizeof(char));
                strcat(dpath, p->pdn);
                strcat(dpath, "/");
                strcat(dpath, dname);
                p_create_dir(dpath);
        }

        return 1;
}

void p_read_template(struct project * restrict p)
{
        printf("Resource directory location from main func : %s\n", p->resd);

        printf("\nProject type as specified : %s\n", p->pt);

        char fp[strlen(p->resd) + strlen(p->pt) + strlen(RES_EXTENSION) + 1];
        memset(fp, 0, strlen(p->resd) + strlen(p->pt) +
                        strlen(RES_EXTENSION) + 1);

        strcat(fp, p->resd);
        strcat(fp, p->pt);
        strcat(fp, RES_EXTENSION);

        char *jsnd = NULL;
        jsnd = p_read_file(fp, jsnd);

        //printf("JSON data : %s\n", jsnd);

        /* first create the directories itself */
        p_parse_jsdata(jsnd, p);

        /* this is where the JSON data is being read into a file */
        //p_parse_json(jsnd, p);

        /*
         * Most probably have to add in the code in here
         * Also save the name of the file
         */

        free(jsnd);
}

void p_mkproject(struct project * restrict p)
{
        if (p_dir_exists(p->pdn)) {
                printf("Dir exists\n");
        } else {
                printf("Creating the directory\n");
                p_create_dir(p->pdn);
        }
        p_read_template(p);
}

