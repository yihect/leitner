#ifndef _VS_H_
#define _VS_H_

#include "list.h"

#define VBUF_LEN 15
#define BOX_LEVEL_CNT 6		// leitner sys box levels
#define DOUBLE_BOX_LEVEL_CNT	BOX_LEVEL_CNT*2
#define TRIPLE_BOX_LEVEL_CNT	BOX_LEVEL_CNT*3 
#define QUADRUPLE_BOX_LEVEL_CNT	BOX_LEVEL_CNT*4

#define VS_CNT_BASE	30
#define MAX_TRIES	3

#define NO_MORE_VIS	0x4e4f4d4f	// NOMOrevi
#define NEW_FIRST_INODE_LINENR	0x4e46494c	// NewFIL
#define NEW_COUNT	0x4e434e54	// NewCNT

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

struct vs_index_head
{
  unsigned int total_vs;	// vs total count
};

struct vs_index_info
{
  unsigned int vs_linenr; 	// line number
  unsigned int pos;					// absolute position
};

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
  unsigned int vs_cur_total; 	// total count of current VSs
  struct lt_box_meta ltb_meta[BOX_LEVEL_CNT];
};

struct lt_box
{
  unsigned int level;
  unsigned int fi_linernr;	// line nr of the first vs item
  unsigned int vs_cnt;			// vs item total count
  unsigned int vs_limit;		// vs item cnt limit
  unsigned int vs_busy_cnt;
  struct list_head i_list; 	// vs item list
  struct lt_box_meta *ltbm;	
};

#endif /* _VS_H_ */

