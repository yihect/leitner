#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include "vs.h"
#include "ltsys.h"

#define MIN_VIS_IN_LBOX0_TO_RELOAD	VS_CNT_BASE
#define VOC_PLACE_IN_SENT_1	"\033[4m"
#define VOC_PLACE_IN_SENT_2	"\033[0m"

#if TEST_FULL_MOVE
int box_leve_max_vs[QUADRUPLE_BOX_LEVEL_CNT]=
{
  1*VS_CNT_BASE,					0, 0, 2,  /* no limit */
  3*VS_CNT_BASE,					6, 0, 4,
  3*VS_CNT_BASE,					8, 0, 8,
  5*VS_CNT_BASE,					10, 0, 10,
  8*VS_CNT_BASE,					16, 0, 16,
  0*VS_CNT_BASE, /* maybe 12, 0 for no limit */		0, 0, 0
};
#else
int box_leve_max_vs[QUADRUPLE_BOX_LEVEL_CNT]=
{
  1*VS_CNT_BASE,					0, 0, 6,  /* no limit */
  2*VS_CNT_BASE,					4, 0, 4,
  3*VS_CNT_BASE,					6, 0, 6,
  5*VS_CNT_BASE,					10, 0, 10,
  8*VS_CNT_BASE,					16, 0, 16,
  0*VS_CNT_BASE, /* maybe 12, 0 for no limit */		0, 0, 0
};
#endif

int lbox_is_full(struct ltsys *lts, int lblevel)
{
  // we consider over lbox won't be full forever
  if (lblevel == 5)
    return false;

  struct lt_box *plb = &(lts->lt_boxes[lblevel]);
  return !(plb->vs_cnt < plb->vs_limit);
}

inline int lbox_is_empty(struct ltsys *lts, int lblevel)
{
  struct lt_box *plb = &(lts->lt_boxes[lblevel]);
  return plb->vs_cnt == 0;
}

int move_to_lbox(struct ltsys *lts, int newlevel, struct vs_item *pvi, int *pug_cnt, int ugc_original, int *pnewlbox_fulled)
{
  int ret = 0;

  // remove from old level lbox
  int oldlevel = pvi->cur_lb_level;
  struct vs_item *tmppvi = container_of(pvi->inode.next, struct vs_item, inode);
  lts->lt_boxes[oldlevel].fi_linernr = lts->lt_boxes[oldlevel].ltbm->first_inode_linenr = tmppvi->vs_linenr;
  list_del(&pvi->inode);
  lts->lt_boxes[oldlevel].vs_cnt = --(lts->lt_boxes[oldlevel].ltbm->vs_cnt);
#if 1
  if (lts->lt_boxes[oldlevel].vs_busy_cnt != 0)
  {
    lts->lt_boxes[oldlevel].vs_cnt = ++(lts->lt_boxes[oldlevel].ltbm->vs_cnt);
    lts->lt_boxes[oldlevel].vs_busy_cnt = --(lts->lt_boxes[oldlevel].ltbm->vs_busy_cnt);
  }
#endif

  // and then add_tail into newlevel lbox
  if (lts->lt_boxes[newlevel].vs_cnt == 0)
  {
    lts->lt_boxes[newlevel].fi_linernr = lts->lt_boxes[newlevel].ltbm->first_inode_linenr = pvi->vs_linenr;
  } else {
    tmppvi = container_of(lts->lt_boxes[newlevel].i_list.prev, struct vs_item, inode);
    lts->vsm_item[tmppvi->vs_linenr].next_inode_linenr = pvi->vs_linenr;
  }
  lts->vsm_item[pvi->vs_linenr].next_inode_linenr = NO_MORE_VIS;
  list_add_tail(&pvi->inode, &(lts->lt_boxes[newlevel].i_list));
#if 1
  if (lbox_is_full(lts, newlevel)) {
    lts->lt_boxes[newlevel].vs_busy_cnt = ++(lts->lt_boxes[newlevel].ltbm->vs_busy_cnt);
  } else 
#endif
    lts->lt_boxes[newlevel].vs_cnt = ++(lts->lt_boxes[newlevel].ltbm->vs_cnt);
  pvi->cur_lb_level = newlevel;

  msync(lts->meta_map, DEF_META_FILE_SIZE+1, MS_SYNC);

#if 1
  // first, we account for this VS's count
  if (lbox_is_full(lts, newlevel))
  {
    ret += 1;
    *pug_cnt -= 1;
    *pnewlbox_fulled = 1;
  }
  // we must make new way in the new lbox if it's full
  while (lbox_is_full(lts, newlevel))
  {
    ret += go_lbox(lts, newlevel, GO_LBTYPE_MAKEWAY, pug_cnt, ugc_original);
    // if lbox is full, then we break after addint it to the tail
    if (newlevel == 0) break;
  }
#endif

  return ret;
}


// maxcnt_2go: max count to go for a full lbox, 0 for no limit
// pug_cnt: VSes cnt pt to go which gave by user, 0 for no limit
// ugc_original: Original value of user-gaved VSes cnt
int go_lbox_internal(struct ltsys *lts, int lblevel, int maxcnt_2go, int *pug_cnt, int ugc_original)
{
  int ret,cnt2go = 0;

  // maintain this if-block carefully
  if (maxcnt_2go == 0)		// in level 0 lbox, if the original ugc is 0, then we go forever until
    				// user canceled with ctrl+d
    cnt2go = (ugc_original == 0) ? -1 : *pug_cnt;
  else if (ugc_original == 0)	// in other level lbox,  depends on ugc_original
    cnt2go = maxcnt_2go;
  else 				// in other level lbox, ugc_original!=0, depends on *pug_cnt
    cnt2go = MIN(maxcnt_2go, *pug_cnt);

  ret = cnt2go;
  int j=0, tries=0, wrong_pos=-1;
  char input_str[30];
  while ((cnt2go != 0) && !lbox_is_empty(lts, lblevel))
  {
    printf("\n  \033[4m %d \033[0m\n", ugc_original-*pug_cnt+1);
    tries=0; wrong_pos=-1;
    struct vs_item *pvi = container_of(lts->lt_boxes[lblevel].i_list.next, struct vs_item, inode);
    while (tries++ < MAX_TRIES)
    {
      if (tries == 1)
      {
	if (pvi->comm) {
	  printf("    SENT: \"%s\"\n\t #:%s \n", pvi->sbuf, pvi->comm);
	} else {
	  printf("    SENT: \"%s\"\n", pvi->sbuf);
	}
      }
      printf("    >>>  ");

      //get answer
      memset(input_str, 0, sizeof(input_str));
      if(fgets(input_str, sizeof(input_str)-1, stdin) == NULL) ERROR("fgets");

      input_str[strlen(input_str)-1] = '\0';
      while (input_str[strlen(input_str)-1] == ' ' || input_str[strlen(input_str)-1] == '\t') {
	input_str[strlen(input_str)-1] = '\0';
      }
      while (input_str[0] == ' ' || input_str[0] == '\t') {
	for (j = 0; j < strlen(input_str); j++) {
	  input_str[j] = input_str[j+1];
	}
      }

      if (strcmp(pvi->vbuf, input_str) == 0) {
	printf("     ?   VVV\t\n");
      }else {
	if (wrong_pos == -1) wrong_pos=tries;
	if (tries == 1) {
	  printf("     ?   XXX\t\n");
	}else {
	  printf("     ?   XXX\t");
	  printf("\tC IS: \"%s\"\n", pvi->vbuf);
	}
      }
    }

    int newlbox_fulled = 0; //flag to indicate the new lbox's fullness in move_to_lbox() function
    switch (wrong_pos) 
    {
      case 1:
	printf("    ** Wrong at 1st try, put this VS into level 1 lb absolutely.\n");
	cnt2go -= move_to_lbox(lts, 0, pvi, pug_cnt, ugc_original, &newlbox_fulled);
	//move_to_lbox(lts, 0, pvi, pug_cnt, ugc_original);
	break;
      case 2:
	printf("    ** Wrong at 2nd try, maybe you need some more tries, so we put this VS into level 1 lb.\n");
	cnt2go -= move_to_lbox(lts, 0, pvi, pug_cnt, ugc_original, &newlbox_fulled);
	//move_to_lbox(lts, 0, pvi, pug_cnt, ugc_original);
	break;
      case 3:
	printf("    ** Wrong at 3rd try, you must use your mind to overcome it.\n");
	cnt2go -= move_to_lbox(lts, lblevel+1, pvi, pug_cnt, ugc_original, &newlbox_fulled);
	//move_to_lbox(lts, lblevel+1, pvi, pug_cnt, ugc_original);
	break;
      default:
	printf("    ** Wonderful done, we put this VS into next level.\n");
	cnt2go -= move_to_lbox(lts, lblevel+1, pvi, pug_cnt, ugc_original, &newlbox_fulled);
	//move_to_lbox(lts, lblevel+1, pvi, pug_cnt, ugc_original);
	break;
    }

    if (newlbox_fulled == 0) // if we have not accounted this VS in move_to_lbox()
    {
      cnt2go--;

      // if *pug_cnt == 0 initially, let it go, do sub
      // but if it is subbed into zero, we do break
      if (--(*pug_cnt) == 0)
	break;
    }
  }
  //printf("\n");

  return ret-cnt2go;
}

// *puser_gave_cnt: VSes cnt to go which gaved by user
// -1  		-> nolimit, terminated by CTRL-D
// integer>0	-> VSes cnt to go, auto load if necessary.
int firstwhile = 0; // only while once
int go_lbox(struct ltsys *lts, int lblevel, int go_lbox_type, int *puser_gave_cnt, int ugc_original)
{
  //if(sigtermed == 1) return 3; // user pressed ctrl^c to terminate this go-session
  int gl_ret = 0;

  if (lbox_is_full(lts, lblevel+1) == 1)
    gl_ret += go_lbox(lts, lblevel+1, go_lbox_type, puser_gave_cnt, ugc_original);

  int maxcnt_2go = box_leve_max_vs[4*lblevel+go_lbox_type+1];
  gl_ret += go_lbox_internal(lts, lblevel, maxcnt_2go, puser_gave_cnt, ugc_original);
  if ((ugc_original>0) && (*puser_gave_cnt==0)) return gl_ret;

  //we go more VIs from lbox[1] before reloading when there is no VIs in lbox[0]
  int i,j,k=0,num_loaded = 0;
  if ((lblevel == 0) && (go_lbox_type != GO_LBTYPE_MAKEWAY))
  {
    if (lts->lt_boxes[1].vs_cnt > MIN_VIS_IN_LBOX0_TO_RELOAD)
    {
      gl_ret += go_lbox_internal(lts, 1, lts->lt_boxes[1].vs_cnt-MIN_VIS_IN_LBOX0_TO_RELOAD,  puser_gave_cnt, ugc_original);
      if ((ugc_original>0) && (*puser_gave_cnt==0)) return gl_ret;
    }

    cmd_load(lts, 0, NULL, &num_loaded);
    if (num_loaded > 0)
      gl_ret += go_lbox(lts, 0, GO_LBTYPE_PUSH, puser_gave_cnt, ugc_original);
    else
    {
      // In case dead looping, we use (firstwhile==0) as condition.
      // We must prevent going all VIs under GO_LBTYPE_MAKEWAY
      while ((firstwhile==0) && (lts->lt_boxes[5].vs_cnt < lts->vsm_head->vs_cur_total))
      {
	firstwhile = 1;
	//we go all VIs in lbox1234 looply with 01->012->0123->01234 circles
	for (i=1; i<=4; i++)
	  for (j=0; j<=i; j++)
	  {
	    // ignore the 1st 0 in loop 01->012->0123->01234 circles
	    if (k++ == 0) continue;
	    gl_ret += go_lbox(lts, j, GO_LBTYPE_CLEAN, puser_gave_cnt, ugc_original);
	    if ((ugc_original>0) && (*puser_gave_cnt==0)) return gl_ret;
	  }
	// at the end of a circle, we should go another circle if need
	if (j==5) firstwhile=0;
      }
    }
  }

  return gl_ret;
}


char *replace_str2(const char *str, const char *old, const char *new)
{
  char *ret, *r;
  const char *p, *q;
  size_t oldlen = strlen(old);
  size_t count, retlen, newlen = strlen(new);
  int samesize = (oldlen == newlen);

  if (!samesize) {
    for (count = 0, p = str; (q = strstr(p, old)) != NULL; p = q + oldlen)
      count++;
    /* This is undefined if p-str > PTRDIFF_MAX */
    retlen = p - str + strlen(p) + count * (newlen - oldlen);
  } else
    retlen = strlen(str);

  if ((ret = malloc(retlen + 1)) == NULL)
    return NULL;

  r = ret, p = str;
  while (1) {
    /* if the old and new strings are different lengths - in other
     * words we have already iterated through with strstr above,
     * and thus we know how many times we need to call it - then we
     * can avoid the final (potentially lengthy call to strstr,
     * which we already know is going to return NULL, by
     * decrementing and checking count.
     */
    if (!samesize && !count--)
      break;
    /* otherwise i.e. when the old and new strings are the same
     * length, and we don't know how many times to call strstr, 
     * we must check for a NULL return here (we check it in any
     * event, to avoid further conditions, and because there's 
     * no harm done with the check even when the old and new
     * strings are different lengths).
     */
    if ((q = strstr(p, old)) == NULL)
      break;
    /* This is undefined if q-p > PTRDIFF_MAX */
    int l = q - p;
    memcpy(r, p, l);
    r += l;
    memcpy(r, new, newlen);
    r += newlen;
    p = q + oldlen;
  }
  strcpy(r, p);

  return ret;
}

void vs_get_newline(char *buf, char *pstart)
{
  while ((*buf++ = *pstart++) != '\n') ;
  buf[-1] = '\0';
}

void ltsys_meta_init(struct ltsys *lts, struct vs_index_head *pidxh)
{
  int i=0, j=0, newly_created=0;
  struct stat meta_sb; char vsfile[20]={0};
  sprintf(vsfile, "%s%s%s%s", "../conf/", (char *)lts->fb_name, ".meta", ".bin");
  if ((lts->meta_fd = open(vsfile, O_RDWR|O_CREAT, 0666)) == -1) ERROR("open");
  if (fstat(lts->meta_fd, &meta_sb) == -1) ERROR("fstat");
  if (meta_sb.st_size < DEF_META_FILE_SIZE) 
  {
    lseek(lts->meta_fd, DEF_META_FILE_SIZE, SEEK_SET);
    write(lts->meta_fd, "\0", 1);
    lseek(lts->meta_fd, 0, SEEK_SET);

    newly_created = 1;
  }
  lts->meta_map = mmap(NULL, DEF_META_FILE_SIZE+1, PROT_READ|PROT_WRITE, MAP_SHARED, lts->meta_fd, 0);
  if (lts->meta_map == MAP_FAILED) ERROR("mmap");
  lts->vsm_head = (struct vs_meta_head *)lts->meta_map;	// maybe offset
  lts->vsm_item = (struct vs_meta_item *)((lts->vsm_head)+1);	
  lts->vsm_head->vs_total = pidxh->total_vs;

  for (i=0; i<BOX_LEVEL_CNT; i++)
  {
    lts->lt_boxes[i].level = i;
    lts->lt_boxes[i].vs_limit = box_leve_max_vs[4*i];
    lts->lt_boxes[i].ltbm = lts->vsm_head->ltb_meta+i;
    if (newly_created)
    {
      lts->lt_boxes[i].ltbm->first_inode_linenr = NEW_FIRST_INODE_LINENR;
      lts->lt_boxes[i].ltbm->vs_cnt = 0;
      lts->lt_boxes[i].ltbm->vs_busy_cnt = 0;
    }
    lts->lt_boxes[i].fi_linernr = lts->lt_boxes[i].ltbm->first_inode_linenr;
    lts->lt_boxes[i].vs_cnt = lts->lt_boxes[i].ltbm->vs_cnt;
    lts->lt_boxes[i].vs_busy_cnt = lts->lt_boxes[i].ltbm->vs_busy_cnt;

    j = 0;
    int previ, curi; 
    struct vs_item *pvsi;
    INIT_LIST_HEAD(&(lts->lt_boxes[i].i_list));
    while (j < (lts->lt_boxes[i].vs_cnt+lts->lt_boxes[i].vs_busy_cnt))
    {
      if (j==0) {
	curi = previ = lts->lt_boxes[i].fi_linernr;
      }else if (j==1) {
	previ = lts->lt_boxes[i].fi_linernr;
	curi = lts->vsm_item[previ].next_inode_linenr;
      }else {
	previ = curi;
	curi = lts->vsm_item[previ].next_inode_linenr;
      }
      pvsi = lts->itblk[curi];
      list_add_tail(&pvsi->inode, &(lts->lt_boxes[i].i_list));
      pvsi->cur_lb_level = i;
      j++;
    }
  }
  msync(lts->meta_map, DEF_META_FILE_SIZE+1, MS_SYNC);
}

void ltsys_init(struct ltsys *lts, char *bfname)
{
  char vsfile[20]={0}, aLine[256];
  int vsfd=0, vsidx_fd=0;
  char *vs_pt=NULL, *vsip=NULL;
  struct stat vs_sb, vsidx_sb;

  if (lts->meta_map)
    ltsys_deinit(lts);

  strcpy(lts->fb_name, bfname);
  sprintf(vsfile, "%s%s%s", "../conf/", lts->fb_name, ".txt");
  if ((vsfd = open(vsfile, O_RDWR)) == -1) ERROR("open");
  if (fstat(vsfd, &vs_sb) == -1) ERROR("fstat");
  vs_pt = mmap(NULL, vs_sb.st_size, PROT_WRITE, MAP_PRIVATE, vsfd, 0);
  if (vs_pt == MAP_FAILED) ERROR("mmap");

  sprintf(vsfile, "%s%s%s%s", "../conf/", lts->fb_name, ".idx", ".bin");
  if ((vsidx_fd = open(vsfile, O_RDWR)) == -1) ERROR("open");
  if (fstat(vsidx_fd, &vsidx_sb) == -1) ERROR("fstat");
  vsip = mmap(NULL, vsidx_sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, vsidx_fd, 0);
  if (vsip == MAP_FAILED) ERROR("mmap");

  lts->itblk = malloc(sizeof(struct vs_item *)*(((struct vs_index_head *)vsip)->total_vs));
  if (lts->itblk == NULL) ERROR("malloc");

  struct vs_item *pitem=NULL;
  struct vs_index_info *pi=(struct vs_index_info *)(vsip+sizeof(struct vs_index_head)); 
  struct vs_index_info *pie=pi+((struct vs_index_head *)vsip)->total_vs;
  while (pi<pie)
  {
    memset(aLine, 0, sizeof(aLine));
    vs_get_newline(aLine, vs_pt+pi->pos);

    int i=0;
    char *tmp=NULL, *tmp2=NULL, undline[30]={0};
    if ((tmp=strchr(aLine, '=')) != NULL)
    {
      pitem = malloc(sizeof(struct vs_item));
      if (pitem == NULL) goto Err0;
      memset(pitem, 0, sizeof(struct vs_item));
      strncpy(pitem->vbuf, aLine, tmp-aLine);

      if ((tmp2=strchr(tmp+1, '#')) != NULL)
      {
	*tmp2 = '\0';
	pitem->comm = malloc(strlen(tmp2+1)+1);
	if (pitem->comm == NULL) goto Err1;
	memset(pitem->comm, 0, strlen(tmp2+1)+1);
	strcpy(pitem->comm, tmp2+1);
      } 

      strcpy(undline, VOC_PLACE_IN_SENT_1);
      while (i++ < strlen(pitem->vbuf))
	strcat(undline, " ");
      strcat(undline, VOC_PLACE_IN_SENT_2);
      pitem->sbuf = replace_str2(tmp+1, pitem->vbuf, undline);
      //pitem->sbuf = malloc(strlen(tmp+1)+1);
      if (pitem->sbuf == NULL) goto Err2;
      //strcpy(pitem->sbuf, tmp+1);

      pitem->vs_linenr = pi->vs_linenr;
      pitem->pos = pi->pos;

      //INIT_LIST_HEAD(&pitem->vsinode);
      INIT_LIST_HEAD(&pitem->inode);
      //list_add_tail(&pitem->vsinode, &lts->vsi_list);
      lts->itblk[pitem->vs_linenr] = pitem;
    }

    pi++;
  }

  // meta file init
  ltsys_meta_init(lts, (struct vs_index_head *)vsip);
  lts->num_per_load = DEFAULT_NUM_PER_LOAD;

  munmap(vs_pt, vs_sb.st_size);
  munmap(vsip, vsidx_sb.st_size);
  close(vsfd);
  close(vsidx_fd);
  return;

Err2:
  free(pitem->vbuf);
Err1:
  free(pitem);
Err0:
  munmap(vs_pt, vs_sb.st_size);
  munmap(vsip, vsidx_sb.st_size);
  close(vsfd);
  close(vsidx_fd);
  ERROR("malloc"); //exit(EXIT_FAILURE);
}


void ltsys_deinit(struct ltsys *lts)
{
  struct stat sb;

  if (lts->meta_map)
  {
    struct vs_item **ppvsi = lts->itblk;
    while(ppvsi <= (lts->itblk+lts->vsm_head->vs_total-1))
    {
      free((*ppvsi)->sbuf);
      if ((*ppvsi)->comm)
      {
	free((*ppvsi)->comm);
	(*ppvsi)->comm=0;
      }
      free(*ppvsi);
      ppvsi++;
    }
    free(lts->itblk);

    struct stat sb;
    if (fstat(lts->meta_fd, &sb) == -1)           /* To obtain file size */
      ERROR("fstat");
    munmap(lts->meta_map, sb.st_size);
    close(lts->meta_fd);
  }

  memset(lts, 0, sizeof(struct ltsys));
}



