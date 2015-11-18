#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "ltsys.h"

#define ENABLE_RUN_TEST	1
#if ENABLE_RUN_TEST
void RunAllTests(void);
#endif

#define REDIRECT_NOT_DONE	(0x1)
#define REDIRECT_TO_PIPE	(0x2)
#define REDIRECT_TO_STDPIPE	(0x4)
#define REDIRECT_TO_FILE	(0x8)
#define REDIRECT_SHELL_COMMAND	(0x10)
#define REDIRECT_FAILURE	(0X20)

FILE	*fp;
static char *less_argv[] = {
  "/usr/bin/less",
  "-r",
  "-F",
  "-E",
  "-X",
  "-Ps -- MORE --  forward\\: <SPACE>, <ENTER> or j  backward\\: b or k  quit\\: q",
  NULL
};

void opt_error(char *str)
{
  printf("Leitner Sys for Vocabulary 0.3 \n");
  printf("ERROR: %s\n", str);
  printf("Availiable Options are: \n");
  printf("	-b	file base name\n");
  printf("	-i	make index\n"); 
}

void start_info( )
{
  printf("Leitner Sys for Vocabulary 0.3 \n");
  printf("Type \"help\" to get cmd list, and type \"help cmd\" for details on the cmd\n");
  printf("Type \"exit\" to leave if you want. \n");
}

static void restore_sanity(struct ltsys *pls)
{
  int fd, waitstatus;

  fp = stdout;
  if (pls->stdpipe) {
    close(fileno(pls->stdpipe));
    pls->stdpipe = NULL;
    if (pls->stdpipe_pid && PID_ALIVE(pls->stdpipe_pid)) {
      while (!waitpid(pls->stdpipe_pid, &waitstatus, WNOHANG))
	stall(1000);
    }
    pls->stdpipe_pid = 0;
  }

  if (pls->pipe) {
    close(fileno(pls->pipe));
    pls->pipe = NULL;
    /*
    console("wait for redirect %d->%d to finish...\n",
	pc->pipe_shell_pid, pc->pipe_pid); */
    if (pls->pipe_pid)
      while (PID_ALIVE(pls->pipe_pid)) {
	waitpid(pls->pipe_pid, &waitstatus, WNOHANG);
	stall(1000);
      }
    if (pls->pipe_shell_pid)
      while (PID_ALIVE(pls->pipe_shell_pid)) {
	waitpid(pls->pipe_shell_pid,
	    &waitstatus, WNOHANG);
	stall(1000);
      }
    pls->pipe_pid = pls->pipe_shell_pid = 0;
  }

  if (pls->ofile) {
    fclose(pls->ofile);
    pls->ofile = NULL;
  }
}


static int setup_stdpipe(struct ltsys *pls)
{
  char *path;

  if (pipe(pls->pipefd) < 0) {
    return false;
  }

  if ((pls->stdpipe_pid = fork()) < 0) {
    return false;
  }

  path = NULL;
  if (pls->stdpipe_pid > 0) {
    close(pls->pipefd[0]);    /* parent closes read end */

    if ((pls->stdpipe = fdopen(pls->pipefd[1], "w")) == NULL) {
      perror("parent fdopen failed");
      return false;
    }
    setbuf(pls->stdpipe, NULL);

    strcpy(pls->pipe_cmd, less_argv[0]);
    return true;;
  } else {
    close(pls->pipefd[1]);    /* child closes write end */

    if (dup2(pls->pipefd[0], 0) != 0) {
      perror("child dup2 failed");
      exit(1);
    }

    path = less_argv[0];
    execv(path, less_argv);

    perror(path);
    fprintf(stderr, "execv of scroll command failed\n");
    exit(1);
  }
}

/*
 *  Run an escaped shell command, redirecting the output to
 *  the current output file.
 */
int shell_command(char *cmd)
{
  FILE *pipe;
  char buf[BUFSIZE];

  sprintf(buf, "%s 2>&1", cmd);
  if ((pipe = popen(buf, "r")) == NULL) {
    perror("cannot open pipe");
    return REDIRECT_FAILURE;
  }

  fprintf(fp, "\n");
  memset(buf, ' ', 2);
  while (fgets(buf+2, BUFSIZE-2, pipe))
    fputs(buf, fp);
  pclose(pipe);
  fprintf(fp, "\n");

  return REDIRECT_SHELL_COMMAND;
}

#include <assert.h>
void set_my_tty(struct ltsys *pls)
{
  char buf[BUFSIZE];
  //char *arglist[M];
  int argc;
  FILE *pipe;

  strcpy(pls->my_tty, "?");

  if (file_exists("/usr/bin/tty", NULL)) {
    sprintf(buf, "/usr/bin/tty");
    if ((pipe = popen(buf, "r")) == NULL) 
      return;

    while (fgets(buf, BUFSIZE, pipe)) {
      if (STRNEQ(buf, "/dev/")) {
	strcpy(pls->my_tty, strip_line_end(&buf[strlen("/dev/")]));
	break;
      }
    }
    pclose(pipe);
    return;
  }
  assert(0);
}

/*
 *  Determine the pids of the current popen'd shell and output command.
 *  This is all done using /proc; the ps kludge at the bottom of this
 *  routine is legacy, and should only get executed if /proc doesn't exist.
 */
static int output_command_to_pids(struct ltsys *pls)
{
  DIR *dirp;
  struct dirent *dp;
  FILE *stp;
  char buf1[BUFSIZE];
  char buf2[BUFSIZE];
  char lookfor[BUFSIZE];
  char *pid, *name, *status, *p_pid, *pgrp;
  char *arglist1[20];
  char *arglist2[20];
  int argc;
  FILE *pipe;
  int retries, shell_has_exited;

  retries = 0;
  shell_has_exited = false;
  pls->pipe_pid = pls->pipe_shell_pid = 0;
  sprintf(lookfor, "(%s)", pls->pipe_cmd);
  stall(1000);

#if 0

retry:
  if (is_directory("/proc") && (dirp = opendir("/proc"))) {
    for (dp = readdir(dirp); dp && !pls->pipe_pid; 
	dp = readdir(dirp)) {
      if (!decimal(dp->d_name, 0))
	continue;
      sprintf(buf1, "/proc/%s/stat", dp->d_name);
      if (file_exists(buf1, NULL) && 
	  (stp = fopen(buf1, "r"))) {
	if (fgets(buf2, BUFSIZE, stp)) {
	  pid = strtok(buf2, " ");
	  name = strtok(NULL, " ");
	  status = strtok(NULL, " ");
	  p_pid = strtok(NULL, " ");
	  pgrp = strtok(NULL, " ");
	  if (STREQ(name, "(sh)") &&
	      (atoi(p_pid) == getpid())) { 
	    pls->pipe_shell_pid = atoi(pid);
	    if (STREQ(status, "Z"))
	      shell_has_exited = true;
	  }
	  if (STREQ(name, lookfor) &&
	      ((atoi(p_pid) == getpid()) ||
	       (atoi(p_pid) == pls->pipe_shell_pid)
	       || (atoi(pgrp) == getpid()))) {
	    pls->pipe_pid = atoi(pid);
	    /*
	    console(
		"FOUND[%d] (%d->%d->%d) %s %s p_pid: %s pgrp: %s\n",
		retries, getpid(), 
		pls->pipe_shell_pid, 
		pls->pipe_pid,
		name, status, p_pid, pgrp); */
	  }  
	}
	fclose(stp);
      }
    }
    closedir(dirp);
  }

  if (!pls->pipe_pid && !shell_has_exited && 
      ((retries++ < 10) || pls->pipe_shell_pid)) {
    stall(1000);
    goto retry;
  }

  fprintf(fp, "getpid: %d pipe_shell_pid: %d pipe_pid: %d\n",
      getpid(), pls->pipe_shell_pid, pls->pipe_pid);

  if (pls->pipe_pid)	
    return pls->pipe_pid;

#else
  sprintf(buf1, "ps -ft %s", pls->my_tty);
  //fprintf(fp, "%s: ", buf1);

  if ((pipe = popen(buf1, "r")) == NULL) {
    perror("cannot determine output pid \n");
    return 0;
  }

  strcpy(buf2, pls->pipe_cmd);
  parse_args(buf2, arglist2);
  while (fgets(buf1, BUFSIZE, pipe)) {
    argc = parse_args(buf1, arglist1);
    if ((argc >= 8) && 
	//STREQ(arglist1[7], pls->pipe_cmd) &&
	STRNEQ(arglist1[7], arglist2[0]) &&
	STRNEQ(pls->my_tty, arglist1[5])) {
      pls->pipe_pid = atoi(arglist1[1]);
      pls->pipe_shell_pid = atoi(arglist1[2]);
      break;
    }
  }
  pclose(pipe);
  //fprintf(fp, "%d\n", pls->pipe_pid);

  return pls->pipe_pid;
#endif
}

static int setup_redirect(char *cmdline, struct ltsys *pls, int cmd_idx)
{
  char *p = cmdline;
  int append = false;
  FILE *ofile;

  if (FIRSTCHAR(p) == '!')
  {
    p++;
    p = strip_beginning_whitespace(p);
    if (!strlen(p))
      return REDIRECT_FAILURE;
    else
      return shell_command(p);
  }

  while (*p) {
    if (*p == '|') {
      p++;
      p = strip_beginning_whitespace(p);
      if (!strlen(p))
	return REDIRECT_FAILURE;

      if ((pls->pipe=popen(p, "w")) == NULL) {
	perror("unable to popen \n");
	return REDIRECT_FAILURE;
      }
      setbuf(pls->pipe, NULL);

      fp = pls->pipe;
      strcpy(pls->pipe_cmd, p);
      pls->pipe_pid = output_command_to_pids(pls);
      return REDIRECT_TO_PIPE;
    }
    if (*p == '>') {
      if (*(p+1) == '>') {
	append = true;
	p++;
      }
      p++;
      p = strip_beginning_whitespace(p);
      if (!strlen(p))
	return REDIRECT_FAILURE;

      if ((ofile = fopen(p, append ? "a+" : "w+")) == NULL) {
	perror("unable to open \n");
	return REDIRECT_FAILURE;
      }
      setbuf(ofile, NULL);

      fp = pls->ofile = ofile;
      return REDIRECT_TO_FILE;
    }
    p++;
  }

  /* TODO: here default to scroll */
  if (Cmds[cmd_idx].stdpipe_type != STDPIPE_NONE)
  {
    if (!setup_stdpipe(pls)) {
      return REDIRECT_FAILURE;
    }
    fp = pls->stdpipe;

    return REDIRECT_TO_STDPIPE;
  }

  return REDIRECT_NOT_DONE;
}

/* Generator function for command completion.  STATE lets us know whether
   to start from scratch; without any state (i.e. STATE == 0), then we
   start at the top of the list. */
char *command_generator (const char *text, int state)
{
  static int list_index, len;
  char *name;

  /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
  if (!state)
    {
      list_index = 0;
      len = strlen (text);
    }

  /* Return the next name which partially matches from the command list. */
  while (name = Cmds[list_index].CmdStr)
    {
      list_index++;

      if (strncmp (name, text, len) == 0)
        return (dupstr(name));
    }

  /* If no names matched, then return NULL. */
  return ((char *)NULL);
}

/* Attempt to complete on the contents of TEXT.  START and END bound the
   region of rl_line_buffer that contains the word to complete.  TEXT is
   the word to complete.  We can use the entire contents of rl_line_buffer
   in case we want to do some simple parsing.  Return the array of matches,
   or NULL if there aren't any. */
char **ltsys_completion(const char *text, int start, int end)
{
  char **matches;

  matches = (char **)NULL;

  /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
  if (start == 0)
    matches = rl_completion_matches(text, command_generator);

  return (matches);
}

void cmdline_init(struct ltsys *pls)
{
  set_my_tty(pls);

  rl_initialize();
  rl_readline_name = "ltsys";
  rl_attempted_completion_function = ltsys_completion;

  /* we use vi mode in readline */
  rl_editing_mode = 0;
  rl_bind_key(CTRL('N'), rl_get_next_history);
  rl_bind_key(CTRL('P'), rl_get_previous_history);
  rl_bind_key_in_map(CTRL('P'), rl_get_previous_history,
      vi_insertion_keymap);
  rl_bind_key_in_map(CTRL('N'), rl_get_next_history,
      vi_insertion_keymap);
  rl_generic_bind(ISFUNC, "[A", (char *)rl_get_previous_history, 
      vi_movement_keymap);
  rl_generic_bind(ISFUNC, "[B", (char *)rl_get_next_history, 
      vi_movement_keymap);
}

int parse_args(char *cmdline, char **av)
{
  char *tok,i ;
  int ac = 0;
  static const char *delim = " \f\n\r\t\v";

  for (tok = strtok(cmdline, delim); tok; tok = strtok(NULL, delim))
  {
    av[ac++] = tok;

    if ((strstr(tok, "|") != NULL) || (strstr(tok, ">") != NULL))
    {
      ac--;
      break;
    }
  }
  av[ac]= NULL;

  return ac;
}


int main(int argc, char *argv[])
{

#if ENABLE_RUN_TEST
  if ((argc == 2) 
      && (strcmp(argv[1], "--test"))==0)
  {
    RunAllTests();
    return 0;
  }
#endif /* ENABLE_RUN_TEST */


  char keystring[256+1], *line, *s;
  char temp_cmdline[256+1];
  int i_argc = 0;
  char **i_argv;
  int c, i =0, ivalue=0;
  char ret = 0;
  char *bfname=NULL, *tval;
  struct ltsys *pls;

  if ((pls = (struct ltsys *)malloc(sizeof(struct ltsys))) == NULL) 
    ERROR("malloc");

  opterr = 0;
  while ((c = getopt(argc, argv, "l:b:i")) != -1)
  {
    switch (c)
    {
      case 'b':	// must have
	{
	  bfname = optarg;
	  break;
	}
      case 'l':
	tval = optarg;
	break;
      case 'i':
	{
	  ivalue=1;
	  break;
	}
      default:
	return 1;
    }
  }

  if (bfname == NULL)
  {
    opt_error("-b option must be used.");
    free(pls);
    return 1;
  }else if(ivalue==1) 
    cmd_mkidx(pls, 0, (char **)bfname, NULL);

  sprintf(keystring, "%s%s%s", "../conf/", bfname, ".idx.bin");
  if ((i=open(keystring, O_RDONLY)) == -1)
  {
    opt_error("You should use -i to make index with -b.");
    free(pls);
    return 1;
  } else close(i);

  //init from vs&index&meta file
  ltsys_init(pls, bfname);

  cmdline_init(pls);
  start_info();
  i_argv = (char**)malloc(1000);
  memset( i_argv, 0, sizeof(i_argv) );	

  fp = stdout;
  while( 1 )
  {	
    memset((char *)i_argv, 0, sizeof(i_argv));
    line = readline("ltsys> ");
    if (!line) break;

    s = clean_line(line);
    if( strlen(s) < 2 )	//	when only input a ENTER key
    {
      free(line);
      continue;		
    }

    add_history(s);
    strcpy(temp_cmdline, s);
    i_argc = parse_args(s, (char **)i_argv);
    if(i_argc)
    {
      for(i=0; i<CmdMAXNum; i++)
      {
	if((strcmp(i_argv[0],Cmds[i].CmdStr)==0)
	    || (strcmp(i_argv[0],Cmds[i].CmdApr)==0))
	  break;
      }
    }

    if ((i<CMD_NULL) 
	|| (strstr(temp_cmdline, "!")!=NULL)
	|| (strstr(temp_cmdline, ">")!=NULL))
    {
      clean_line(temp_cmdline);
      ret = setup_redirect(temp_cmdline, pls, i);
      if (ret == REDIRECT_SHELL_COMMAND)
      {
	free(line);
	continue;
      }
    }

    if (i >= CMD_NULL)
    {
      cmd_disp();
      free(line);
      continue;
    }

    if ((ret=((Cmds[i].cmdproc)(pls,i_argc,(char**)i_argv,NULL))) == 0)
      break;

    restore_sanity(pls);
    free(line);

    usleep(50);
  }

  free(i_argv);
  return 0;
}



