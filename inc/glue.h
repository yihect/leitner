#ifndef _GLUE_H_
#define _GLUE_H_

#include "idr.h"
#include "rbtree.h"
#include "list.h"
#include "enum_factory.h"

/* meta data format in gbm[], see comment in glue.c */
#define GBM_BDT_SIZE		4
#define GBM_BDT_SHIFT		28
#define GBM_BDT_MASK		((1<<GBM_BDT_SIZE-1) << GBM_BDT_SHIFT)
#define GBM_WT_SIZE		2
#define GBM_WT_SHIFT		26
#define GBM_WT_MASK		((1<<GBM_WT_SIZE-1) << GBM_WT_SHIFT)
#define GBM_VALIDBITS_SIZE	26
#define GBM_VALIDBITS_SHIFT	0
#define GBM_VALIDBITS_MASK	((1<<GBM_VALIDBITS_SIZE-1) << GBM_VALIDBITS_SHIFT)

/* glue info etc. */
struct glue_info_entry {
	unsigned char head_nr;
	unsigned char node_nr;
	unsigned char gli_type;	// lhead-1ton, lhead-mton, lnode, lnode_backpt
	char *desc;
};

#define GIE_TABLE_SIZE	GBM_VALIDBITS_SIZE
struct glue_info {
	struct glue_info_entry gie_tbl[GIE_TABLE_SIZE];
	unsigned char gbm_ftype;	// gbm format type
};

/* wrapper types, such as uw/mw/tw etc. */
#define WRAPPER_TYPES(XX)					\
	XX(WT_UW, ,542)			/* upper wrapper */	\
	XX(WT_MW, ,334)			/* middle wrapper */	\
	XX(WT_TW, ,516)			/* terminal wrapper */	\
	XX(WT_WTMAX, ,10395551)					\

DECLARE_ENUM(wrapper_types, WRAPPER_TYPES)

/* must be included after wrapper_types */
#include "glue_eng.h"

/* glue info node types, no need of enum magic */
enum {
	GLIT_LHEAD_1TON,
	GLIT_LHEAD_MTON,
	GLIT_LNODE,
	GLIT_LNODE_TOP_BACKPT,
	GLIT_LNODE_BOTTOM_BACKPT,
	GLIT_DUMMY,
};


/* glue wrapper for UW/MW/TW */
struct glue_wrapper {
	void *datapt;			// back pt to data(pt_uw_data etc.)
	union
	{
		struct rb_node n;	// (MW) rb_node into rb tree in dataroot
		void *extpt;		// for UW, this extra pt may point to XXX_idrs[0]
					// in dataroot; for TW, no use of this pt
	} u;

	/* NOTE: total size of all above members should be even_num*sizeof(long) */
	struct glue_node
	{
		unsigned int gbm[2];	// glue meta data (bitmap,type etc) which be used
					// to explain the usage of ptrs in following
					// lh block.
		struct list_head lhb[0];	// lh array block
	} gn;
};


/* data part encoded into dataroot for rf/voc/gram etc.
 *
 * Note: for different BDTs, XXX_uw_list[] may be an indirect or direct list.
 *
 *	Take pt for example, there is a summary UW, which includes several
 *	sublist composed of other sub-UWs, so this is an indirect list.
 *	And another example, thinking of v BDT, which contains other UWs directly
 *	in this list, so it's a direct list.
 */
#define data_part(id, name, ts_max)			\
	struct idr name##_idrs[2];	/* mw & tw */	\
	struct list_head name##_uw_list[(ts_max)];	\
	struct rb_root name##_mw_rb

/* data root for this system */
struct dataroot {
	struct idr all_uw_idr;	/* all uw_id -> uw_ptr */

	/* data part for rf/voc/gram etc */
	data_part(BDT_PT, phontran, TS_PT_PTMAX);
	data_part(BDT_PG, phongram, TS_PG_PGMAX);
	data_part(BDT_RF, rootfix, TS_RF_RFMAX);
	data_part(BDT_V, voca, TS_V_VMAX);
	data_part(BDT_G, gram, TS_G_GMAX);
	data_part(BDT_S, sent, TS_S_SMAX);
};

/* data root declation */
extern struct dataroot dr;

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

struct rfvg_all_struct
{
  struct type_struct rfvg_ts; 		// total UW count && UW ptr blk
  struct type_struct rf_ts[TS_RF_RFMAX]; 	// r_f type struct
  struct rb_root rf_rb;			// rf tree
  struct type_struct g_ts[TS_G_GMAX];
  struct rb_root g_rb;
  struct type_struct v_ts[TS_V_VMAX];	// all types of vocab
  struct rb_root v_rb;			// v tree
};

#endif /* _GLUE_H_ */
