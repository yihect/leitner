#ifndef _VS_H_
#define _VS_H_

#include "list.h"
#include "rbtree.h"

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

struct v_item
{
  void *tw;
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
  unsigned int vs_cur_total; 	// total count of current VSs
  struct lt_box_meta ltb_meta[BOX_LEVEL_CNT];
};

/////////////////////////////////////////////////
// business

/* glue wrapper for UW/MW/TW */
struct glue_wrapper
{
  char *name;
  char *desc;			// verbose description info
  union
  {
    struct rb_node n;		// (MW) rb_node into (rf|v)_struct.(rf|v)_rb  tree
    void *pt;			// for TW, pt point to *_item obj
    				// for UW, this pt may point to *_TS_ALL ts obj
  } u;
  struct glue_node
  {
    unsigned long *pglue_bm;	// glue bitmap which be used 
    				// to explain the usage of ptrs in following
    				// block.
    struct list_head lhb[0];	// lh array block 
  } gn;
};

/* glue wrapper/node logic */
unsigned bbits(unsigned n);
void set_offset(unsigned *gn_bitmap, unsigned gn_type, unsigned offset);
unsigned get_offset(unsigned gn_bitmap, unsigned gn_type);


struct type_struct
{
  unsigned int type_item_cnt; 	// total item cnt
  union {
    void *uw_ptrblk; 	// UW ptr blk for rfvg_ts
    struct {
      void *mw_ptrblk;	// block of ptr(to MWrapper for *_ALL type)
      void *tw_ptrblk;	// block of ptr(to TWrapper for *_ALL type)
    } mt;
    struct list_head uw_list;	// UWrapper list
  } ;
};

/* In following type_ts[*].u: 
 * ALL -> u.twrapper_ptrblk (ptrblk to Terminal Wrapper) 
 * OTHER -> u.uw_list (list of UWriapper objects) */
enum { RF_TS_ALL, RF_TS_ROOT, RF_TS_PREFIX, RF_TS_SUFFIX, RF_TS_MAX };
enum { V_TS_ALL, V_TS_BASIC, V_TS_NOUN, V_TS_VERB, V_TS_ADJ, V_TS_ADV, \
  V_TS_PRON, V_TS_PREP, V_TS_CONJ, V_TS_DETE, V_TS_MAX };
enum { G_TS_ALL, G_TS_SIMPLE_SENT, G_TS_COMP_SENT, G_TS_SIMPLIFY_SENT, G_TS_MAX };

/* UW have two types: fixed part (language), and dynamic part (which comes
 * from base vocabulary). Here is UW  fixed type enum */
enum { UWT_DUMB_ROOT, UWT_DUMB_PF, UWT_NOUN_SF, UWT_VERB_SF, UWT_ADJ_SF, UWT_ADV_SF, \
   UWT_NO_RF, UWT_CNTABLE_VNOUN, UWT_UNCNTABLE_VNOUN, UWT_BOTH_VNOUN, UWT_LINK_VVERB, \
   UWT_AUX_VVERB, UWT_SENSE_VVERB, UWT_CAUSATIVE_VVERB, UWT_IRREG_VVERB, UWT_TRANS_VVERB, \
   UWT_INTRANS_VVERB, UWT_ATTRIBUTIVE_VADJ, UWT_PREDICATIVE_VADJ, UWT_MANNER_VADV, \
   UWT_INTENSIFIERS_VADV, UWT_SENTENCE_VADV, UWT_DUMB_PRON, UWT_NOUN_VPREP, UWT_ADJ_VPREP, \
   UWT_VERB_VPREP, UWT_DUMB_CONJ, UWT_PRE_VDETE, UWT_MID_VDETE, UWT_SUF_VDETE, UWT_TENSE_GSP, \
   UWT_V_GSP, UWT_TOV_GSP, UWT_TOVING_GSP, UWT_PARTI_GSP, UWT_MOODS_GSP, UWT_COMPOUND_GCOMP, \
   UWT_COMPNOUN_GCOMP, UWT_COMPADJ_GCOMP, UWT_COMPADV_GCOMP, UWT_COMPCOMM_GSPL, \
   UWT_COMPNOUN_GSPL, UWT_COMPADJ_GSPL, UWT_COMPADV_GSPL, UWT_FIXED_MAX };

struct rfvg_all_struct
{
  struct type_struct rfvg_ts; 		// total UW count && UW ptr blk
  struct type_struct rf_ts[RF_TS_MAX]; 	// r_f type struct
  struct rb_root rf_rb;			// rf tree
  struct type_struct g_ts[G_TS_MAX];
  struct rb_root g_rb;
  struct type_struct v_ts[V_TS_MAX];	// all types of vocab
  struct rb_root v_rb;			// v tree
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

