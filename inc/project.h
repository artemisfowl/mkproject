/**
 * @file project.h
 * @author sb
 * @brief project file holding all the atomic functions
 */

/* macros */
#ifndef MAX_ARGS
#define MAX_ARGS 4
#endif

#ifndef MIN_ARGS
#define MIN_ARGS 2
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

/* functions */
void p_display_usage(void);
void p_setup_struct(struct project * restrict p);
