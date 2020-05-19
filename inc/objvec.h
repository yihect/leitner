#ifndef _OBJVEC_H_
#define _OBJVEC_H_

#include "cvspool.h"
//#include "rbtree.h"

/* an simple object vector pool build on cvspool */

/* id for every objv block, which returned by objv_alloc() */
#define OBJS_ID_MIN	1

/* objvec mode */
#define OBJV_M_NEEDID	CVSP_M_NEEDID

struct objvec;
struct objv_obj_ops {
	void (*ser_ctor_high)(struct objvec *mpl, void *obj);	/* high ctor for obj deser */
	void (*ser_ctor_low)(struct objvec *mpl, void *obj);	/* low ctor for obj deser */
	void (*ser_dtor)(struct objvec *mpl, void *obj);	/* dtor for obj ser */
};

#if 0
struct nodevec {
	struct rb_node node;
	char *nodept;
};
#endif

struct objvec {
	cvspool *cp;
	unsigned int size;	// obj size, in bytes

	unsigned int mode;
	unsigned pool_id;
	struct objv_obj_ops *oops;
	struct idr *busy_vec_idr;	/* for storing busy vectors */
	//struct rb_root node_tree;
};

struct objvec *objv_init(unsigned int osize, unsigned int mode, struct objv_obj_ops *oops);
void objv_exit(struct objvec *ov);

char *objv_alloc(struct objvec *ov, unsigned obj_cnt);
char *objv_realloc(struct objvec *ov, char *oldobjs, unsigned new_cnt,
		   void (*adjust_fn)(char *oldobjs, unsigned oldsize,
				     char *newobjs, unsigned newsize, void *adj_pram),
		   void *adjparam);
void objv_free(struct objvec *ov, char *objs);

unsigned int objvec_ser_getlen(struct ser_root_data *srd, void *mpl);
char *objvec_ser(struct ser_root_data *srd, void *mpl, char *dst);
char *objvec_deser_high(struct ser_root_data *srd, void *mpl, char *src);
void objvec_deser_low(struct ser_root_data *srd, void *mpl);

#endif /* _OBJVEC_H_ */
