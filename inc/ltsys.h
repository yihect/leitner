#ifndef _LTSYS_H_
#define _LTSYS_H_

#include <stdio.h>
#include "vs.h"
#include "util.h"

#define DEFAULT_NUM_PER_LOAD	30

#define ERROR(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define INFO(msg) \
  do { perror(msg); } while (0)

#define PINFO(msg) \
  do { printf("%s\n", msg); } while (0)

/////////////////////////////////////////////////////////////
//  ltsys 
struct ltsys
{
  char fb_name[20];			// file base name
  int meta_fd;
  char *meta_map;
  struct vs_meta_head *vsm_head;	// mmaped meta header pt
  struct vs_meta_item *vsm_item; 	// mmaped meta item block pt
  struct lt_box lt_boxes[BOX_LEVEL_CNT];	// leitner sys level boxes
  unsigned int num_per_load;	// load num item every time
  //struct list_head vsi_list;					// vs item list(maybe hashed, for random usage)
  struct vs_item **itblk;	// item ptr array
  struct rfvg_all_struct rfvg;	// rfvg (to fix: move vs_item etc into this)

  // redirect about
  int pipefd[2];		/* output pipe file descriptors */
  FILE *stdpipe;                  /* standard pipe for output */
  pid_t stdpipe_pid;		/* per-cmd standard output pipe's pid */
  FILE *ofile;
  FILE *pipe;
  int pipe_pid, pipe_shell_pid;
  char pipe_cmd[BUFSIZE];     /* pipe command line */
  char my_tty[10];
};

extern int sigtermed;
extern FILE	*fp;

void command_init();
char *cmd_modify_n_generator(const char *text, int state);

int cmd_clear(struct ltsys *lts, int iargc, char **iargv, void *priv);
int cmd_init(struct ltsys *lts, int iargc, char **iargv, void *priv);
int cmd_load(struct ltsys *lts, int iargc, char **iargv, void *priv);
int cmd_go(struct ltsys *lts, int iargc, char **iargv, void *priv);
int cmd_lsbox(struct ltsys *lts, int iargc, char **iargv, void *priv);
int cmd_mkidx(struct ltsys *lts, int iargc, char **iargv, void *priv);
int cmd_modify(struct ltsys *lts, int iargc, char **iargv, void *priv);
int cmd_help(struct ltsys *lts, int iargc, char **iargv, void *priv);
int cmd_exit(struct ltsys *lts, int iargc, char **iargv, void *priv);

void cmd_disp();
void cmd_clear_help();
void cmd_init_help();
void cmd_load_help();
void cmd_go_help();
void cmd_lsbox_help();
void cmd_mkidx_help();
void cmd_modify_help();
void cmd_help_help();
void cmd_exit_help();

#define DEF_VS_NUM	1280
#define DEF_IDX_FILE_SIZE 	(DEF_VS_NUM*sizeof(struct vs_index_info)+sizeof(struct vs_index_head))
#define DEF_META_FILE_SIZE 	(DEF_VS_NUM*sizeof(struct vs_meta_item) + sizeof(struct vs_meta_head))	 //offset

////////////////////////////////////////////////
// command about.
enum
{
  CMD_CLR = 0,
  CMD_INIT,
  CMD_LOAD,
  CMD_GO,
  CMD_LSBOX,
  CMD_MKIDX,
  CMD_MODIFY,
  CMD_HELP,
  CMD_EXIT,
  CMD_NULL,
};

enum
{
  GO_LBTYPE_PUSH = 0,
  GO_LBTYPE_CLEAN,
  GO_LBTYPE_MAKEWAY,
};

enum
{
  STDPIPE_NONE,
  STDPIPE_PARTIAL,
  STDPIPE_ALL,
};

#define CmdMAXNum CMD_NULL

typedef int (*CMD_PROCESSOR_FN)(struct ltsys *lts, int argc, char **argv, void *priv);
typedef void (*CMD_USAGE_HELP_FN)();
typedef struct tagTCOMMAND
{
  char *CmdStr;	// command string
  char *CmdApr;
  int stdpipe_type;
  CMD_PROCESSOR_FN cmdproc;
  CMD_USAGE_HELP_FN cmdusage;
} TCOMMAND;

static TCOMMAND Cmds[CmdMAXNum+1] =
{
  { "clear", "c", STDPIPE_NONE, cmd_clear, cmd_clear_help},
  { "init", "i", STDPIPE_NONE, cmd_init, cmd_init_help},
  { "load", "ld", STDPIPE_NONE, cmd_load, cmd_load_help},
  { "go", "g", STDPIPE_NONE, cmd_go, cmd_go_help},
  { "lsbox", "ls", STDPIPE_PARTIAL, cmd_lsbox, cmd_lsbox_help},
  { "mkidx", "m", STDPIPE_NONE, cmd_mkidx, cmd_mkidx_help},
  { "modify", "mo", STDPIPE_NONE, cmd_modify, cmd_modify_help},
  { "help", "h", STDPIPE_PARTIAL, cmd_help, cmd_help_help},
  { "exit",  "e", STDPIPE_NONE, cmd_exit, cmd_exit_help},
  { (char*)NULL, (char *)NULL, 0, NULL, NULL  }
};	



void ltsys_init(struct ltsys *lts, char *bfname);
void ltsys_deinit(struct ltsys *lts);


#endif /* _LTSYS_H_ */
