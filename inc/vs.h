#ifndef _VS_H_
#define _VS_H_

#include <assert.h>
#include "list.h"
#include "rbtree.h"

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

typedef unsigned short	idt;

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
    unsigned int gbm[2];	// glue bitmap which be used 
    				// to explain the usage of ptrs in following
    				// lh block. gbm[1] may be used as back ptr.
    struct list_head lhb[0];	// lh array block 
  } gn;
};

/* glue wrapper/node logic */
#define GBM_LH_NODE_TYPE_SIZE		4
#define GBM_LH_NODE_TYPE_SHIFT		28
#define GBM_LH_NODE_TYPE_MASK		((1<<GBM_LH_NODE_TYPE_SIZE-1) << GBM_LH_NODE_TYPE_SHIFT)
#define GBM_FORMAT_TYPE_MASK		0x08000000
#define GBM_FT0_OFFSET_SIZE		15
#define GBM_FT0_VALIDBITS_SIZE		9
#define GBM_FT0_VALIDBITS_SHIFT		(GBM_FT0_OFFSET_SIZE + 3)
#define GBM_FT0_VALIDBITS_MASK		((1<<GBM_FT0_VALIDBITS_SIZE-1) << GBM_FT0_VALIDBITS_SHIFT)
#define GBM_FT1_1ST_OFFSET_SIZE		10
#define GBM_FT1_1ST_LH_ITEMS		3
#define GBM_FT1_2ND_OFFSET_SIZE		30
#define GBM_FT1_2ND_LH_ITEMS		14
#define GBM_FT1_VALIDBITS_SIZE		(GBM_FT1_1ST_LH_ITEMS + GBM_FT1_2ND_LH_ITEMS)
#define GBM_FT1_VALIDBITS_SHIFT		GBM_FT1_1ST_OFFSET_SIZE
#define GBM_FT1_VALIDBITS_MASK		((1<<(GBM_FT1_VALIDBITS_SIZE-1)) << GBM_FT1_VALIDBITS_SHIFT)

struct glue_info_entry {
  unsigned char head_nr;
  unsigned char node_nr;
  unsigned char gli_type; 	// lhead-1ton, lhead-mton, lnode, lnode_backpt
  char desc[59];
};

#define GIE_TABLE_SIZE 	GBM_FT1_VALIDBITS_SIZE	// bigger of type 0 and type 1
struct glue_info {
  struct glue_info_entry gie_tbl[GIE_TABLE_SIZE];
  unsigned char gbm_ftype;	// gbm format type
};

/* glue_type and ln_node_type 
 * so, glue_lh_node_type = 3*glue_type + ln_node_type */
enum {GL_T_PHONTRANS, GL_T_PHON, GL_T_RFIX, GL_T_VOCA, GL_T_GRAM, GL_T_SENTENCE, GL_T_MAX};
enum {LN_T_ALL, LN_T_UW=LN_T_ALL, LN_T_MW, LN_T_TW, LN_T_MAX};

/* glue lh node type about */
enum { GLT_UW_PHONSYM, GLT_MW_PHONSYM, GLT_TW_PHONSYM, GLT_UW_PHON, GLT_MW_PHON,
  GLT_TW_PHON, GLT_UW_RFIX, GLT_MW_RFIX, GLT_TW_RFIX, GLT_UW_VOCA, GLT_MW_VOCA,
  GLT_TW_VOCA, GLT_UW_GRAM, GLT_MW_GRAM, GLT_TW_GRAM, GLT_SENTENCE, GLT_MAX };

/* glue lh node offset options for every kinds of glue lh_node type */
enum { GLOF_UW_PHONSYM_TODO };
enum { GLOF_MW_PHONSYM_TODO };
enum { GLOF_TW_PHONSYM_TODO };
enum { GLOF_UW_PHON_TODO };
enum { GLOF_MW_PHON_TODO };
enum { GLOF_TW_PHON_TODO };

enum { GLOF_MW_RFIX_TYPE_LHEAD };	// for rf uw
enum { GLOF_MW_RFIX_TYPE_LNODE, GLOF_TW_RFIX_USAGE_LHEAD, GLOF_MW_RFIX_INSTANCE_LHEAD/*B1*/ };
enum { GLOF_TW_RFIX_USAGE_LNODE, GLOF_MW_RFIX_USAGE_INSTANCE_LHEAD/*C1*/ };

enum { GLOF_MW_VOCA_TYPEWORDS_LHEAD/*A1*/, GLOF_TW_VOCA_TYPEUSAGE_LHEAD };
enum { GLOF_MW_VOCA_BASEWORDS_LNODE,  GLOF_UW_VOCA_TYPE_LHEAD/*A2*/, GLOF_SW_VOCA_EXAMPLEV_LHEAD/*E1*/,
	GLOF_MW_VOCA_BASEWORDS_LHEAD, GLOF_TW_VOCA_MEANINGUSAGE_LHEAD, GLOF_MW_VOCA_RTINCLUDED_LHEAD/*B2*/,
	GLOF_TW_VOCA_WORDSALLG_LHEAD/*D1*/, GLOF_TW_VOCA_USAGE_INCLUDED_LHEAD/*C2*/ }; 
enum { GLOF_TW_VOCA_MEANINGUSAGE_LNODE, GLOF_TW_VOCA_TYPEUSAGE_LNODE, GLOF_TW_VOCA_DUMMY1, GLOF_SW_VOCA_EXAMPLEVUSAGE_LHEAD/*F1*/ };

enum { GLOF_MW_GRAM_GTYPE_LHEAD };
enum { GLOF_MW_GRAM_GTYPE_LNODE, GLOF_TW_GRAM_GUSAGE_LHEAD, GLOF_SW_GRAM_EXAMPLEG_LHEAD/*G1*/ };
enum { GLOF_TW_GRAM_GUSAGE_LNODE, GLOF_MW_GRAM_VWORDS_LHEAD/*D2*/, GLOF_SW_GRAM_EXAMPLEUSAGE_LHEAD/*H1*/ };

enum { GLOF_MW_SENT_VUSED_LHEAD/*E2*/, GLOF_TW_SENT_VUSEDUSAGE_LHEAD/*F2*/, 
	GLOF_MW_SENT_GUSED_LHEAD/*G2*/, GLOF_TW_SENT_GUSEDUSAGE_LHEAD/*H2*/ };

enum { GLIT_LHEAD_1TON, GLIT_LHEAD_MTON, GLIT_LNODE, GLIT_LNODE_TOP_BACKPT, GLIT_LNODE_BOTTOM_BACKPT, GLIT_DUMMY };

//inline unsigned bbits(unsigned n) __attribute__((always_inline));
unsigned bbits(unsigned n);
//inline unsigned is_offset_valid(unsigned *gn_bitmap, unsigned gn_type) __attribute__((always_inline));
unsigned is_offset_valid(unsigned *gn_bitmap, unsigned gn_type);
//inline void mark_offset(unsigned *gn_bitmap, unsigned gn_type, unsigned validness) __attribute__((always_inline));
void mark_offset(unsigned *gn_bitmap, unsigned gn_type, unsigned validness);
void set_offset(unsigned *gn_bitmap, unsigned gn_type, unsigned offset);
unsigned get_offset(unsigned *gn_bitmap, unsigned gn_type);


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
 * OTHER -> u.uw_list (list of UWrapper objects) */
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

struct ltsys;
int go_lbox(struct ltsys *lts, int lblevel, int go_lbox_type, int *puser_gave_cnt, int ugc_original);

#endif /* _VS_H_ */

