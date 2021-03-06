#ifndef _CVSPOOL_H_
#define _CVSPOOL_H_

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include "msm.h"

/* Compact Variable Size Pool(cvspool) suits for storing objects like strings.
 * It's memory is 4Bytes aligned for every slot.
 *
 * An object need occupy several continous slots, namely a busy node.
 * In this busy node, there are 8Bytes(2 slots) for meta data, and all remain
 * Bytes for string itself.
 *
 * For a free node, we use 12Bytes to store meta data. So in one node, there
 * at least is: 2_busy_meta+1_object_content = 2_free_meta+1_free_slot */

#define FREE_GUARD0	0xF5	// guard at free head
#define FREE_GUARD1	0xFFF7	// guard at free tail
#define BUSY_GUARD0	0xFFF8	// guard at busy head
#define BUSY_GUARD1	0xFFF9  // guard at busy tail

#define NULL_SLOTABS	0xFFFFF	// indicat to the end of next/prev list
#define MIN_NODE_SIZE	3	// 3_free_meta (unite in slot)

/* Turnning on this will increase mem usage in busy node */
//#define CVSP_DEBUG

/* cvspool modes (external) */
#define CVSP_M_AUTOEX	(1<<0)	// auto expand ??
#define CVSP_M_NEEDID	(1<<1)

/* node ID encoding in 32bits */
union node_id {
	unsigned long l;
	struct {
		unsigned long up:2;	/* must be 2 LSB bits, in LittleEndian */
		unsigned long pool:4;	/* poolid, check range in 0~15 */
		unsigned long trunk:6;	/* trunk id, start from 0 */
		unsigned long slot:20;	/* slot offset from t->mem, inside bnode */
	} s;
};

/* for linking free nodes, 8Bytes long at minimal.
 * max size of 64K*4=256KBytes for a free node due to len's type u_int_16 */
typedef struct cvspool_free_node0 {
	u_int8_t pad;		// free guard pad
	u_int8_t idx;		// prev/next free node idx in blk. (blk of size MAX_BLK_SIZE bytes)
	u_int16_t len_1;	// how many slots of this node, (actually len_1+1 slots, should >=3slots)
	u_int16_t prev;		// prev free node offset(index of every 4 Bytes) in a MAX_BLK_SIZE fnode
	u_int16_t next;		// next free node offset(index of every 4 Bytes) in a MAX_BLK_SIZE fnode
} cvspool_fnode0;

/* we need tail space to merge prev free node */
typedef struct cvspool_free_node1 {
	u_int16_t len_1;	// save len-1 again, for pre-merging
	u_int16_t g1;		// free guard1
} cvspool_fnode1;


/* busy nodes left-most slot, 4Bytes long */
typedef struct cvspool_busy_node0 {
	u_int16_t g0;		// busy guard0
	u_int16_t len_1;	// how many slots of this node (actually len_1+1 slots, should >=3slots)
} cvspool_bnode0;

#if 0
/* we may consider to ignore cvspool_bnode1, for store letters ASAP */
/* busy nodes right-most slot, 4Bytes long */
typedef struct cvspool_busy_node1 {
	u_int16_t pad;	// padding, can be used for '\0' character
#ifdef CVSP_DEBUG
	u_int16_t g1;		// busy guard1
#endif
} cvspool_bnode1;
#endif

/* trunk of size (16*64*1024(slots) = 4MBytes), namely 16 bloks */
typedef struct cvspool_trunk_s {
	unsigned int id;		// trunk id
	u_int32_t t_total;		// in slots
	u_int32_t t_used;		// in slots, inited with 0
	unsigned int t_total_mem;	// total mem length, in bytes
	char *mem;			// value: mem == free_head
	cvspool_fnode0 *free_head;	// circle list with this pre-allocated head
	struct cvspool_trunk *next;
	struct cvspool_trunk **pprev;	// originally, we can use list_head here,
					// but we DO NOT want to rewrite old
					// code. We use a back pointer of pointer.
} cvspool_trunk;

/* cvs pool */
typedef struct cvs_pool {
	u_int32_t total;		// in slots
	u_int32_t used;			// inited with 0
	u_int32_t total_mem;		// buffer total bytes
	unsigned int mode;
	unsigned int trunk_ocnt;	// original trunk count
	unsigned int trunk_cnt;		// actual total trunk count
	cvspool_trunk *trunk_list;
	cvspool_trunk **btrunk_list;	// back list of trunk, with &trunk:pprev as node;
	cvspool_trunk *cached_trunk;	// cached chunk ptr for accelrating id/node conversion

	/* fields for ser */
	unsigned int pool_id;
	struct idr *trunk_idr;		/* for patching objs while serring, used for storing slab_ser_ctx */
} cvspool;

/* a cvs_pool may be composed of several blks that its max size is MAX_BLK_SIZE bytes.
 * There are (UNIT16_MAX) slots in a a max-sized blk, and all free nodes in a cvspool
 * are connected by prev/next blk index (coded in cvspool_free_node:pos field) and
 * prev/next in-blk offset */
#define MAX_BLK_SIZE	((UINT16_MAX+1)<<2)	// in Bytes, position starts from 0
#define MAX_TRUNK_SIZE	((MAX_BLK_SIZE)<<4)	// max 16 blocks coded in fnode::idx
#define BLKIDX_OF_SLOTABS(abs)	((abs)/((MAX_BLK_SIZE)>>2))	// in slots
#define IBOFF_OF_SLOTABS(abs)	((abs)%((MAX_BLK_SIZE)>>2))
#define PNBLKIDX(pidx,nidx)	(((pidx)<<4)+(nidx))
#define PREV_BLKIDX(pnidx)	((pnidx)>>4)
#define NEXT_BLKIDX(pnidx)	((pnidx)&0x0f)	// 4bits for prev & next
#define SLOTABS_POS(idx,off)	(((idx)*((MAX_BLK_SIZE)>>2))+(off))

/* prev/next free node of fn node in t trunk */
#define NEXT_FNODE(t, fn)	({ int ab=SLOTABS_POS(NEXT_BLKIDX((fn)->idx), ((fn)->next)); \
				 ((cvspool_fnode0 *)((t)->mem+(ab<<2))); })
#define PREV_FNODE(t, fn)	({ int ab=SLOTABS_POS(PREV_BLKIDX((fn)->idx), ((fn)->prev)); \
				 ((cvspool_fnode0 *)((t)->mem+(ab<<2))); })

/* for eache fnode in trunk */
#define for_each_fnode(t, fn)		\
	for (*fn=NEXT_FNODE(t, t->free_head); (*fn!=t->free_head); *fn=NEXT_FNODE(t,(*fn)))

/* for each fnode in cvspool */
#define for_each_fnode_cp(cp, t, fn)		\
	for (*t=cp->trunk_list; (*t!=NULL); *t=((*t)->next)) \
		for (*fn=NEXT_FNODE((*t), (*t)->free_head); (*fn!=(*t)->free_head); *fn=NEXT_FNODE((*t),(*fn)))

/* for each trunk in cvspool */
#define for_each_trunk(cp, t)		\
	for (*t=cp->trunk_list; (*t!=NULL); *t=((*t)->next))

#if 0
#define for_each_trunk_rev(cp, t, tmp)		\
	for (tmp=cp->btrunk_list, *t=container_of(tmp, cvspool_trunk, next); \
	     ((tmp!=NULL)&&((*t)!=NULL)); tmp=((*t)->pprev))
#endif

unsigned int slots_num_adjust(unsigned int slots_num);

void set_fnode_len(cvspool_fnode0 *fn0, unsigned int len);
int get_fnode_len(cvspool_fnode0 *fn0);

unsigned long id_of_node(cvspool *cvsp, char *str);
char *node_from_id(cvspool *cvsp, unsigned long id);

char *iter_bnode(cvspool_trunk *t);
void cvsp_for_each_bnode(cvspool *cvsp, int (*fn)(void *p, void *data), void *data);
unsigned int get_bnode_mem_len(cvspool *cvsp, char *mem);

cvspool_trunk *add_new_trunk(cvspool *sp, unsigned int t_slot_size);
int cvsp_init(cvspool **cvsp, unsigned int slots_num, unsigned int mode);
void cvsp_destroy(cvspool *cvsp);

char *cvsp_alloc(cvspool *cvsp, unsigned int size);
void cvsp_free(cvspool *cvsp, char *str);

unsigned int cvspool_ser_getlen(struct ser_root_data *srd, void *mpl);
char *cvspool_ser(struct ser_root_data *srd, void *mpl, char *dst);
char *cvspool_deser_high(struct ser_root_data *srd, void *mpl, char *src);
void cvspool_deser_low(struct ser_root_data *srd, void *mpl);

#endif


