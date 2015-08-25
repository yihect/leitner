#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "ltsys.h"

#define ENABLE_RUN_TEST	1
#if ENABLE_RUN_TEST
void RunAllTests(void);
#endif



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
  printf("ltsys> ");
}


int parse_args(char *cmdline, char **av)
{
  char *tok,i ;
  int ac = 0;
  static const char *delim = " \f\n\r\t\v";

  for (tok = strtok(cmdline, delim); tok; tok = strtok(NULL, delim))
  {
    av[ac++] = tok;
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


  char keystring[256+1];
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

  start_info();
  i_argv = (char**)malloc(1000);
  memset( i_argv, 0, sizeof(i_argv) );	

  while( 1 )
  {	
    memset(keystring, 0, sizeof(keystring));
    memset((char *)i_argv, 0, sizeof(i_argv));
    fgets(keystring, sizeof(keystring)-1, stdin );

    if( strlen(keystring) < 2 )	//	when only input a ENTER key
    {
      printf("ltsys> ");
      continue;		
    }

    i_argc = parse_args(keystring, (char **)i_argv);
    if(i_argc)
    {
      for(i=0; i<CmdMAXNum; i++)
      {
	if((strcmp(i_argv[0],Cmds[i].CmdStr)==0)
	    || (strcmp(i_argv[0],Cmds[i].CmdApr)==0))
	  break;
      }
    }

    if (i >= CMD_NULL)
    {
      cmd_disp();
      printf("ltsys> ");
      continue;
    }

    if (ret=((Cmds[i].cmdproc)(pls,i_argc,(char**)i_argv,NULL)) == 0)
      break;
    printf("ltsys> ");


    usleep(50);
  }

  free(i_argv);
  return 0;
}



