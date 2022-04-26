#ifndef _VS_H_
#define _VS_H_

#include <assert.h>
#include "list.h"
#include "rbtree.h"
#include "types.h"

#ifndef BUG_ON
#define BUG_ON(cond) assert(!(cond))
#endif

#define VBUF_LEN 30
#define BOX_LEVEL_CNT 6		// leitner sys box levels
#define DOUBLE_BOX_LEVEL_CNT	BOX_LEVEL_CNT*2
#define TRIPLE_BOX_LEVEL_CNT	BOX_LEVEL_CNT*3
#define QUADRUPLE_BOX_LEVEL_CNT	BOX_LEVEL_CNT*4

#define TEST_FULL_MOVE	1
#if TEST_FULL_MOVE
#define VS_CNT_BASE	4//30
#else
#define VS_CNT_BASE	30
#endif

#define MAX_TRIES	3

#define NO_MORE_VIS	0x4e4f4d4f	// NOMOrevi
#define NEW_FIRST_INODE_LINENR	0x4e46494c	// NewFIL
#define NEW_COUNT	0x4e434e54	// NewCNT

///////////////////////////////////////////////////
// language basics

struct glue_node;

struct vs_item
{
  unsigned int vs_linenr;
  unsigned int pos;				// absolute position
  char vbuf[VBUF_LEN];
  char *sbuf;
  char *comm;
  unsigned int play_cnt;
  //struct list_head vsinode;	// for all
  struct list_head inode;	// for single
  int cur_lb_level;	// current ltbox level
};

struct v_entry
{
  idt ve_id;
  char *ve_name;
  void *mw;
};

struct v_item
{
  idt vi_id;
  char *vi_name;
  char *vi_meaning;
  void *tw;
  union
  {
    char *ext1;
    char *ext2;
  } u;
};

struct g_item
{
  void *tw;
};

struct rf_item
{
  char *name;
  char *desc;
  unsigned long lan_type;	// language specific type flags
  void *tw;	// to_wrapper ptr (for fixed size, we don't put extra glue data into *_item)
};


///////////////////////////////////////////////////
// index about

struct vs_index_head
{
  unsigned int total_vs;	// vs total count
};

struct vs_index_info
{
  unsigned int vs_linenr; 	// line number
  unsigned int pos;					// absolute position
};


/////////////////////////////////////////////////
// meta about

struct vs_meta_item
{
  unsigned int next_inode_linenr;		// line nr of next vs item
  unsigned int play_cnt;						// play count stored
};

struct lt_box_meta
{
  unsigned int first_inode_linenr;	// line nr of first vs item
  unsigned int vs_cnt; 	// vs item count in this box
  unsigned int vs_busy_cnt;	// busy VSs
};

struct vs_meta_head
{
  unsigned int vs_total;	// total count of available VSs
  unsigned int vs_cur_total;	// total count of current VSs
  struct lt_box_meta ltb_meta[BOX_LEVEL_CNT];
};

/////////////////////////////////////////////////
// business

struct lt_box
{
  unsigned int level;
  unsigned int fi_linernr;	// line nr of the first vs item
  unsigned int vs_cnt;			// vs item total count
  unsigned int vs_limit;		// vs item cnt limit
  unsigned int vs_busy_cnt;
  struct list_head i_list;	// vs item list
  struct lt_box_meta *ltbm;
};

struct ltsys;
int go_lbox(struct ltsys *lts, int lblevel, int go_lbox_type, int *puser_gave_cnt, int ugc_original);

#endif /* _VS_H_ */

