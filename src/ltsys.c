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
#include <dirent.h>
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
  fprintf(fp, "  mo[dify]\tmodify or create objects \n");
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
void cmd_modify_help()
{
  fprintf(fp, "\n");
  fprintf(fp, "   usage: mo[dify] action [obj_type] content \n");
  fprintf(fp, "\n");
  fprintf(fp, "  Modify or create objects.\n");
  fprintf(fp, "   action:\n");
  fprintf(fp, "     n\t\tcreate the obj, using obj_tag as tag.\n");
  fprintf(fp, "     w\t\tmodify the obj with obj_tag tag.\n");
  fprintf(fp, "     c\t\tcommit the obj with obj_tag tag.\n");
  fprintf(fp, "     m\t\tmodify the existed objs.\n");
  fprintf(fp, "   obj_type, tip for content complementation or obj writing:\n");
  fprintf(fp, "     rf\t\troot/fixes\n");
  fprintf(fp, "     v\t\tvocabulary\n");
  fprintf(fp, "     vg\t\tvocabulary group\n");
  fprintf(fp, "     g\t\tgrammar item\n");
  fprintf(fp, "   content:\n");
  fprintf(fp, "     obj_tag\tusing this as a tag when creating new objects\n");
  fprintf(fp, "     obj_name\tthe object with this name\n");
  fprintf(fp, "     obj_id\tthe object with this ID\n");
  fprintf(fp, "     beg,end\tthe objects at this line range.\n");
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

  if (iargc > 1)
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

struct mod_tag_file {
  char tag_name[16];
  int why_created;	// 1->"mod n"; 0->"mod m"
  int lines, ex1, ex2;
  struct list_head tg;
};
LIST_HEAD(tg_list);

struct glt_write_tips {
  int glt_type;
  char obj_type_char; // obj_type char in cmd
  char *w_tipe_file;
  int file_linenr;
} glt_wtips[GL_T_MAX] =
{
  {GL_T_PHONTRANS, 't', "phonsym_tip_ltd.txt", 0},
  {GL_T_PHON, 'p', "phonsym_tip_ltd.txt", 0},
  {GL_T_RFIX, 'r', "rfix_tip_ltd.txt", 0},
  {GL_T_VOCA, 'v', "voca_tip_ltd.txt", 0},
  {GL_T_GRAM, 'g', "gram_tip_ltd.txt", 0},
  {GL_T_SENTENCE, 's', "sentence_tip_ltd.txt", 0},
};

#define GET_FILE_LINENR1	"sed -n '$=' ../conf/%s"
#define GET_FILE_LINENR2	"sed -n '$=' ../conf/%s/%s"
#define GET_FILE_LINENR3	"sed -n '$=' ../conf/%s/%s.ltd"
#define MOD_NEW_CMD	"cat ../conf/tips/%s|./vipe +%d|sed -e '1,%dd'>../conf/stage/%s.ltd"
//#define MOD_WRITE_CMD	"sed -e "1i\\$(cat ../conf/tips/%s)" ../conf/stage/%s.ltd |./vipe +%d|sed -e '1,%dd'>../conf/stage/%s.ltd"
//#define MOD_WRITE_CMD	"cat ../conf/tips/%s ../conf/stage/%s.ltd |./vipe +%d|sed -e '1,%dd'>../conf/stage/%s.ltd"
#define MOD_WRITE_CMD	"cat ../conf/tips/%s ../conf/stage/%s.ltd >/tmp/temp.ltd;cat /tmp/temp.ltd |./vipe +%d|sed -e '1,%dd'>../conf/stage/%s.ltd; rm -f /tmp/temp.ltd"
#define TAG_LIST_ADD_CMD1	"echo \"%s\t\t%d\t%d\t%d\t%d\" > ../conf/stage/tag_list.mod"
#define TAG_LIST_ADD_CMD2	"sed -i '$a\%s\t\t%d\t%d\t%d\t%d' ../conf/stage/tag_list.mod"
#define TAG_LIST_CHANGE_CMD	"sed -i '/%s/c\\%s\t\t%d\t%d\t%d\t%d' ../conf/stage/tag_list.mod"
#define TAG_LIST_DEL_CMD	"mv ../conf/stage/tag_list.mod /tmp/temp.mod; sed '/%s/d' /tmp/temp.mod  > ../conf/stage/tag_list.mod; rm -f /tmp/temp.mod"
//#define TAG_LIST_ADD_CMD	"./atofile.sh ../conf/stage/tag_list.mod %s\t\t%d\t%d\t%d\t%d"

/*
1, replace '\' with '\\', for preparing.
sed -i '1,$s/\\/\\\\/' ./rep.txt

2, insert a '\' chacter at the end of every line in rep.txt file:
#sed -i '1,$s/$/&\\/' ./rep.txt
sed '$!s/$/\\/' ./rep.txt


3, Rep.txt file must be processed with the 2nd cmd firstly.
replace lines between 3 and 4 in test.txt file with content from rep.txt:
sed -i "3,4c\\$(cat ./rep.txt)" ./test.txt


pre: We use followint cmd to insert obj_writting tips:
sed -e "1i\\$(cat rep1.txt)" -ne "2,3p" ./test.txt
*/

void command_init()
{
  int i=0;
  struct stat fst;
  char buf[96]={0};
  char res_buf[48]={0};

  /* calculate the length of tip-files */
  for (i=0; i<GL_T_MAX; i++)
  {
    snprintf(buf, sizeof(buf), GET_FILE_LINENR2, "tips", glt_wtips[i].w_tipe_file);
    shell_command(buf, res_buf, sizeof(res_buf));
    glt_wtips[i].file_linenr = atoi(clean_line(res_buf));
  }

  /* gather all non-committed tag files in stage directory.
   * first pasing tag list file */
  char *token_list[6];
  struct mod_tag_file *tgfile=0;
  FILE *f = fopen("../conf/stage/tag_list.mod", "w+");
  if (f==0) ERROR("open tag list file error.");
  snprintf(buf, sizeof(buf), "%s", "../conf/stage/tag_list.mod");
  if (stat(buf, &fst) == -1) ERROR("fstat");
  i = fst.st_size; //i as flag of zero-length tag list file.
  while(fgets(buf,sizeof(buf),f))
  {
    parse_args(buf, token_list);
    tgfile = malloc(sizeof(struct mod_tag_file));
    strncpy(tgfile->tag_name, token_list[0], sizeof(tgfile->tag_name));
    tgfile->why_created = atoi(token_list[1]);
    tgfile->lines = atoi(token_list[2]);
    tgfile->ex1 = atoi(token_list[3]);
    tgfile->ex2 = atoi(token_list[4]);
    list_add(&tgfile->tg, &tg_list);
  }
  fclose(f);
  /* and then gather others non booked in tag list file */
  struct dirent *entry;
  struct mod_tag_file *t;
  DIR *const dir = opendir("../conf/stage");
  if (dir == NULL) ERROR("cannot recurse into directory ../conf/stage");
  else {
    while ((entry=readdir(dir)) != NULL)
    {
      list_for_each_entry(t, &tg_list, tg) {
	if (strcmp(t->tag_name, entry->d_name) == 0)
	  goto NEXT;
      }

      if (strcmp(entry->d_name, ".") != 0  &&
	  strcmp(entry->d_name, "..") != 0 &&
	  strcmp(entry->d_name, "tag_list.mod") != 0)
      {
	tgfile = malloc(sizeof(struct mod_tag_file));
	snprintf(buf, sizeof(buf), GET_FILE_LINENR2, "stage", entry->d_name);
	shell_command(buf, res_buf, sizeof(res_buf));

	tgfile->why_created = 1; //new exported from other place
	tgfile->lines = atoi(clean_line(res_buf));
	tgfile->ex1 = tgfile->ex2 = 0;
	strcpy(buf, entry->d_name);
	char *pend = strstr(buf, ".ltd");
	*pend = '\0';
	strncpy(tgfile->tag_name, buf, sizeof(tgfile->tag_name));
	list_add(&tgfile->tg, &tg_list);

	if (i == 0)
	  snprintf(buf, sizeof(buf), TAG_LIST_ADD_CMD1,
	      tgfile->tag_name, tgfile->why_created, tgfile->lines,
	      tgfile->ex1, tgfile->ex2);
	else
	  snprintf(buf, sizeof(buf), TAG_LIST_ADD_CMD2,
	      tgfile->tag_name, tgfile->why_created, tgfile->lines,
	      tgfile->ex1, tgfile->ex2);
	shell_command(buf, NULL, 0);
      }
NEXT: i=1; // i as a flag
    }
    closedir (dir);
  }
}

char *cmd_modify_n_generator(const char *text, int state)
{
  static int len;
  struct mod_tag_file *t;
  struct list_head *tgn;
  static struct list_head *s_tgn;

  if(!state) {
    len = strlen (text);
    s_tgn = &tg_list;
  }

  list_for_each(tgn, s_tgn) {
    if(tgn == &tg_list) break;
    t = list_entry(tgn, typeof(*t), tg);
    if (strncmp(t->tag_name, text, len) == 0) {
      s_tgn = tgn;
      return (dupstr(t->tag_name));
    }
  }

  /* If no names matched, then return NULL. */
  return ((char *)NULL);
}

int cmd_modify(struct ltsys *lts, int iargc, char **iargv, void *priv)
{
  int fd;
  struct stat fst;
  char *p=NULL;
  struct mod_tag_file *tgfile;
  int type, bline, eline;
  char buf[1024]={0};
  char res_buf[48]={0};

  if ((iargc==3)&&(strcmp(iargv[1], "c")!=0))
  { cmd_modify_help(); return 1;}

  if (iargc!=4)
  { cmd_modify_help(); return 1;}
  else
  {
    for (type=0; type<GL_T_MAX; type++)
    {
      if (iargv[2][0] == glt_wtips[type].obj_type_char)
	break;
    }
    if (type == GL_T_MAX)
    { cmd_modify_help(); return 1;}
  }

  switch (iargv[1][0])
  {
    case 'n':	//new obj
NEW:  snprintf(buf, sizeof(buf), MOD_NEW_CMD, glt_wtips[type].w_tipe_file,
	  glt_wtips[type].file_linenr, glt_wtips[type].file_linenr, iargv[3]);
      shell_command(buf, NULL, 0);
      snprintf(buf, sizeof(buf), "../conf/stage/%s.ltd", iargv[3]);
      if (stat(buf, &fst) == -1) ERROR("fstat");

      if (fst.st_size == 0)
	unlink(buf);
      else {
	tgfile = malloc(sizeof(struct mod_tag_file));
	snprintf(buf, sizeof(buf), GET_FILE_LINENR3, "stage", iargv[3]);
	shell_command(buf, res_buf, sizeof(res_buf));

	tgfile->why_created = 1; //new created
	tgfile->lines = atoi(clean_line(res_buf));
	tgfile->ex1 = tgfile->ex2 = 0;
	strncpy(tgfile->tag_name, iargv[3], sizeof(tgfile->tag_name));
	list_add(&tgfile->tg, &tg_list);

	snprintf(buf, sizeof(buf), "%s", "../conf/stage/tag_list.mod");
	if (stat(buf, &fst) == -1) ERROR("fstat");
	if (fst.st_size == 0)
	  snprintf(buf, sizeof(buf), TAG_LIST_ADD_CMD1,
	      tgfile->tag_name, tgfile->why_created, tgfile->lines,
	      tgfile->ex1, tgfile->ex2);
	else
	  snprintf(buf, sizeof(buf), TAG_LIST_ADD_CMD2,
	      tgfile->tag_name, tgfile->why_created, tgfile->lines,
	      tgfile->ex1, tgfile->ex2);
	shell_command(buf, NULL, 0);
      }
      return 1;

    case 'w': //write uncommitted objs
      list_for_each_entry(tgfile, &tg_list, tg) {
	if (strcmp(tgfile->tag_name, iargv[3]) == 0)
	  break;
      }
      if (&tgfile->tg == &tg_list) goto NEW;

      snprintf(buf, sizeof(buf), MOD_WRITE_CMD, glt_wtips[type].w_tipe_file, iargv[3],
	  glt_wtips[type].file_linenr, glt_wtips[type].file_linenr, iargv[3]);
      shell_command(buf, NULL, 0);
      snprintf(buf, sizeof(buf), "../conf/stage/%s.ltd", iargv[3]);
      if (stat(buf, &fst) == -1) ERROR("fstat");

      if (fst.st_size == 0) {
	unlink(buf);
	snprintf(buf, sizeof(buf), TAG_LIST_DEL_CMD, iargv[3]);
	shell_command(buf, NULL, 0);
      }
      else {
	snprintf(buf, sizeof(buf), GET_FILE_LINENR3, "stage", iargv[3]);
	shell_command(buf, res_buf, sizeof(res_buf));
	tgfile->lines = atoi(clean_line(res_buf));
	snprintf(buf, sizeof(buf), TAG_LIST_CHANGE_CMD, tgfile->tag_name,
	    tgfile->tag_name, tgfile->why_created, tgfile->lines,
	    tgfile->ex1, tgfile->ex2);
	shell_command(buf, NULL, 0);
      }
      return 1;

    case 'c':
      //commit the new created objs
      return 1;

    case 'm':
      //evaluate the obj type
      break;

    default:
      { cmd_modify_help(); return 1;}
  }

  if ((p=strchr(iargv[3], ',')) != NULL)
  {
    *p = '\0'; p++;
    bline = atoi(iargv[1]);
    eline = atoi(p);
  }else if (decimal(iargv[1], 0))
  {
    // find obj and ...
  }else
  {
    // find obj and ...
  }

  return 1;
}






