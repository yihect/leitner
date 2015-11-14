#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>
#include "ltsys.h"


#define LPVS_1	"  [%d|%d]\t%s\t%s\n"
#define LPVS_2	"  [%d|%d]\t%s\t\t%s\n"

void cmd_disp( void )
{
  fprintf(fp, "\n");
  fprintf(fp, "  c[lear]\tclear the screen \n");
  fprintf(fp, "  i[nit]\tinit vocabulary file etc. \n");
  fprintf(fp, "  l[oa]d\tload more vocabs \n");
  fprintf(fp, "  g[o]  \tstart to mem \n");
  fprintf(fp, "  ls[box]\tlist statistics of leitner sys box \n");
  fprintf(fp, "  m[kidx]\tmake index file  for vs \n");
  fprintf(fp, "  h[elp]\tshow cmd info \n");
  fprintf(fp, "  e[xit]\tleave the program \n");
  fprintf(fp, "\n");
}

void cmd_clear_help()
{
  fprintf(fp, "\n");
  fprintf(fp, "   usage: c[lear]  \n");
  fprintf(fp, "\n");
  fprintf(fp, "  Clear the screen.\n");
  fprintf(fp, "\n");
}
void cmd_init_help()
{
  fprintf(fp, "\n");
  fprintf(fp, "   usage: i[nit] VS_file_basename \n");
  fprintf(fp, "\n");
  fprintf(fp, "  Initialize leitner sys using the VS file with basename.\n");
  fprintf(fp, "\n");
}
void cmd_load_help()
{
  fprintf(fp, "\n");
  fprintf(fp, "   usage: l[oa]d [num] \n");
  fprintf(fp, "\n");
  fprintf(fp, "  Load num VSes into level 0 box. Load 30 VSes defaultly.\n");
  fprintf(fp, "\n");
}
void cmd_go_help()
{
  fprintf(fp, "\n");
  fprintf(fp, "   usage: g[o] [num] \n");
  fprintf(fp, "\n");
  fprintf(fp, "  Start a session to memorize. You can use ctrl_D to terminate this session.\n");
  fprintf(fp, "   num\tthe number of VSes to memorize in this session.\n");
  fprintf(fp, "\n");
}
void cmd_lsbox_help()
{
  fprintf(fp, "\n");
  fprintf(fp, "   usage: ls[box] 0..4|o|a \n");
  fprintf(fp, "\n");
  fprintf(fp, "  List all VSes in one level-box or all.\n");
  fprintf(fp, "   0..4\ta special level\n");
  fprintf(fp, "   a\tall levels\n");
  fprintf(fp, "   o\tthe over level\n");
  fprintf(fp, "\n");
}
void cmd_mkidx_help()
{
  fprintf(fp, "\n");
  fprintf(fp, "   usage: m[kidx] VS_file_basename \n");
  fprintf(fp, "\n");
  fprintf(fp, "  Make index file for VS file with basename.\n");
  fprintf(fp, "\n");
}
void cmd_help_help()
{
  fprintf(fp, "\n");
  fprintf(fp, "   usage: h[elp] [full_cmd|cmd_abbreviation] \n");
  fprintf(fp, "\n");
  fprintf(fp, "  List help information.\n");
  fprintf(fp, "\n");
}
void cmd_exit_help()
{
  fprintf(fp, "\n");
  fprintf(fp, "   usage: e[xit]  \n");
  fprintf(fp, "\n");
  fprintf(fp, "  Exit ltsys.\n");
  fprintf(fp, "\n");
}


///////////////////////////////////////////////////////////////////
// cmd processing ..
//
int cmd_clear(struct ltsys *lts, int iargc, char **iargv, void *priv)
{
  /* clear the screen */
  system("clear");
  return 1;
}

int cmd_init(struct ltsys *lts, int iargc, char **iargv, void *priv)
{
  if (iargc != 2)
  { cmd_init_help(); return 1;} 
  
  int i=0;
  char buf[128], tmp[50]={0};
  sprintf(buf, "%s%s%s", "../conf/", iargv[1], ".txt");
  if ((i=open(buf, O_RDONLY)) == -1)
  {
    sprintf(tmp, "There isnot any vs file with %s as base filename.", iargv[1]);
    opt_error(tmp);
    return 1;
  } else close(i);

  sprintf(buf, "%s%s%s", "../conf/", iargv[1], ".idx.bin");
  if ((i=open(buf, O_RDONLY)) == -1)
    cmd_mkidx(lts, 0, (char **)iargv[1], NULL);
  else close(i);

  //init from vs&index&meta file
  ltsys_init(lts, iargv[1]);
  return 1;
}


int cmd_load(struct ltsys *lts, int iargc, char **iargv, void *priv)
{
  int num_to_load = lts->num_per_load;
  if (iargc == 2) 
    num_to_load = MIN(num_to_load, atoi(iargv[1]));

  // we must think about current space in lbox and available VSes in vs file.
  num_to_load = MIN(num_to_load, (lts->lt_boxes[0].vs_limit-lts->lt_boxes[0].vs_cnt));
  num_to_load = MIN(num_to_load, lts->vsm_head->vs_total-lts->vsm_head->vs_cur_total);
  if (priv != NULL) *((int *)priv) = num_to_load;

  int i=0, bp=lts->vsm_head->vs_cur_total;
  struct vs_item *pvi;
  while (i<num_to_load)
  {
    pvi = *(lts->itblk + bp + i);

    if (lts->lt_boxes[0].vs_cnt == 0)
    {
      lts->lt_boxes[0].fi_linernr = lts->lt_boxes[0].ltbm->first_inode_linenr = bp+i;
    }
    else
    {
      //lts->vsm_item[lts->lt_boxes[0].vs_cnt-1].next_inode_linenr = bp+i;
      struct vs_item *tmppvi = container_of(lts->lt_boxes[0].i_list.prev, struct vs_item, inode);
      lts->vsm_item[tmppvi->vs_linenr].next_inode_linenr = bp+i;
    }

    lts->vsm_item[bp+i].next_inode_linenr = NO_MORE_VIS;
    list_add_tail(&pvi->inode, &(lts->lt_boxes[0].i_list));
    pvi->cur_lb_level = 0;
    lts->lt_boxes[0].vs_cnt = ++(lts->lt_boxes[0].ltbm->vs_cnt);
    lts->vsm_head->vs_cur_total ++;
    i++;
  }

  msync(lts->meta_map, DEF_META_FILE_SIZE+1, MS_SYNC);

  return 1;
}

sigjmp_buf env_sig;
void sigh(int signum)
{
  printf("\n");
  siglongjmp(env_sig, 12);
}

int cmd_go(struct ltsys *lts, int iargc, char **iargv, void *priv)
{
  int cnt = 0; // 0 for nolimit
  int ret = 0;
  if (iargc == 2) cnt=atoi(iargv[1]);

  struct sigaction nact, oact;
  nact.sa_handler = sigh;
  sigemptyset(&nact.sa_mask);
  nact.sa_flags = SA_RESETHAND | SA_RESTART;
  sigaction(SIGINT, &nact, &oact);

  if (sigsetjmp(env_sig, 1) != 0)
  {
    fprintf(fp, "\n");
    sigaction(SIGTERM, &oact, NULL);
    return 1;
  }

  extern int firstwhile;
  firstwhile = 0;
  ret = go_lbox(lts, 0, GO_LBTYPE_PUSH, &cnt, cnt);
  fprintf(fp, "We have goed %d VS items.\n", ret);
  fprintf(fp, "\n");

  sigaction(SIGTERM, &oact, NULL);

  return 1;
}

int cmd_lsbox(struct ltsys *lts, int iargc, char **iargv, void *priv)
{
  if ((iargc != 2) || (strlen(iargv[1]) != 1)) 
  { cmd_lsbox_help(); return 1;} 

  int level = -1;
  char c=iargv[1][0], all=0;
  struct list_head *vslh=NULL;
  if ((c>='0') && (c<=('0'+BOX_LEVEL_CNT-2)))
  {
    vslh = &lts->lt_boxes[c-'0'].i_list;
    level = c-'0';
  }
  else if ((c=='a') || (c=='A'))
  {
    all=1;
  }
  else if ((c=='o') || (c=='O'))
  {
    vslh = &lts->lt_boxes[BOX_LEVEL_CNT-1].i_list;
    level = BOX_LEVEL_CNT-1;
  }
  else 
  {
    cmd_lsbox_help();	// won't go here
    return 1;
  }

  fprintf(fp, "\n");
  struct vs_item *pi=NULL, **ppi=NULL;
  if (all == 1)
  {
    ppi = lts->itblk;
    while(ppi <= (lts->itblk+lts->vsm_head->vs_total-1))
    {
      pi = *ppi;
      fprintf(fp, (strlen(pi->vbuf)>=8)?LPVS_1:LPVS_2, pi->vs_linenr, pi->pos, pi->vbuf, pi->sbuf);
      ppi++;
    }
  }
  else
  {
    struct lt_box *plb = &(lts->lt_boxes[level]);
    fprintf(fp, "  level: %d\tfi_line: %d\t[%d|%d|%d] \n", plb->level, plb->fi_linernr,
			plb->vs_cnt, plb->vs_busy_cnt, plb->vs_limit);
    int cnt=0;
    list_for_each_entry(pi, vslh, inode)
    {
      if (cnt++ == plb->vs_cnt) fprintf(fp, "  ----\n");
      fprintf(fp, (strlen(pi->vbuf)>=8)?LPVS_1:LPVS_2, pi->vs_linenr, pi->pos, pi->vbuf, pi->sbuf);
    }
  }
  fprintf(fp, "\n");

  return 1;
}


int cmd_mkidx(struct ltsys *lts, int iargc, char **iargv, void *priv)
{
  char vsfile[20]={0};
  int vsfd=0, vsidx_fd=0, tn;
  char *vs_pt=NULL, *vsidx_pt=NULL;
  struct stat vs_sb, vsidx_sb;

  if (!strcmp(lts->fb_name, (char *)iargv) && lts->meta_map)
  {
    fprintf(fp, "Warning... You are making index of current vs.\n");
    return 0;
  }

  sprintf(vsfile, "%s%s%s", "../conf/", (char *)iargv, ".txt");
  if ((vsfd = open(vsfile, O_RDWR)) == -1) ERROR("open");
  if (fstat(vsfd, &vs_sb) == -1) ERROR("fstat");
  vs_pt = mmap(NULL, vs_sb.st_size, PROT_WRITE, MAP_PRIVATE, vsfd, 0);
  if (vs_pt == MAP_FAILED) ERROR("mmap");

  sprintf(vsfile, "%s%s%s%s", "../conf/", (char *)iargv, ".idx", ".bin");
  if ((vsidx_fd = open(vsfile, O_RDWR|O_CREAT, 0666)) == -1) ERROR("open");
  if (fstat(vsidx_fd, &vsidx_sb) == -1) ERROR("fstat");
  if (vsidx_sb.st_size < DEF_IDX_FILE_SIZE) 
  {
    lseek(vsidx_fd, DEF_IDX_FILE_SIZE, SEEK_SET);
    write(vsidx_fd, "\0", 1);
    lseek(vsidx_fd, 0, SEEK_SET);
  }
  vsidx_pt = mmap(NULL, DEF_IDX_FILE_SIZE+1, PROT_READ|PROT_WRITE, MAP_SHARED, vsidx_fd, 0);
  if (vsidx_pt == MAP_FAILED) ERROR("mmap");

  unsigned long pos=0, bpos=0, lines=0;
  char *pb=vs_pt, aLine[128];;
  char *pt1=vs_pt, *pte1=vs_pt+vs_sb.st_size;
  char *vsidx_info_pt=vsidx_pt+sizeof(struct vs_index_head);
  while (pt1 < pte1)
  {
    if (*pt1 == '\n')
    {
      strncpy(aLine, pb, pt1-pb);
      aLine[pt1-pb]='\0';
      
      if (*aLine != '#')
      {
	char *tmp=NULL, *tmp2=NULL;;
	if ((tmp=strchr(aLine, '=')) != NULL) // we count one line if there is a '='
	{
	  tn = lines++;
	  ((struct vs_index_info *)vsidx_info_pt)[tn].vs_linenr = tn;
	  ((struct vs_index_info *)vsidx_info_pt)[tn].pos = bpos;
	}
      }
    }

    if ((*pt1=='\n') && (pt1<pte1))
    {
       bpos += pt1-pb+1; 
       pb=pt1+1;
    }
    pt1++; pos++;
  }
  
  ((struct vs_index_head *)vsidx_pt)->total_vs = lines; // total vs count
  msync(vsidx_pt, DEF_IDX_FILE_SIZE+1, MS_SYNC);
  munmap(vs_pt, vs_sb.st_size);
  munmap(vsidx_pt, DEF_IDX_FILE_SIZE+1);
  close(vsfd);
  close(vsidx_fd);
  return 1;
}


int cmd_help(struct ltsys *lts, int iargc, char **iargv, void *priv)
{
  int i;
  if(iargc > 1)
  {
    for(i=0; i<CmdMAXNum; i++)
    {
      if((strcmp(iargv[1],Cmds[i].CmdStr)==0)
	  || (strcmp(iargv[1],Cmds[i].CmdApr)==0))
	break;
    }

    if (i==CmdMAXNum)
      cmd_disp();
    else
      Cmds[i].cmdusage();
  }
  else cmd_disp();

  return 1;
}

int cmd_exit(struct ltsys *lts, int iargc, char **iargv, void *priv)
{
  fprintf(fp, " < ltsys exit.... Bye !!!!>\n");
  return 0;	
}







