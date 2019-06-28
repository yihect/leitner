#ifndef _OBJPOOL_H_
#define _OBJPOOL_H_

//#include "stdbool.h"
#include "msm.h"

struct slab;
struct slab_ser_ctx {
	struct slab *slab, *sered_slab;
};

struct slab {
	void *free;
	unsigned int id;
	struct slab *next;
};

typedef struct slab slab_t;

struct objp_obj_ops;
struct objpool {
	int obj_size;
	int real_obj_size;
	int slab_size;
	int slab_nr;
	int obj_nr;
	int free_obj_nr;
	slab_t *slab_head;
	slab_t *recent_slab;
	unsigned int mode;

	/* fields for ser */
	unsigned int pool_id;
	struct objp_obj_ops *oops;
	struct idr *slab_idr;	/* for patching objs while serring, used for storing slab_ser_ctx */
};

typedef struct objpool objpool_t;

#if 0
typedef union objp_ser_ctx {
	void *objpt;	/* pointer of obj */
	struct {
		unsigned int pad:6;
		unsigned int objp_id:10;	/* id of objpool instance, see POOLID_MAX */
		unsigned int slab_id:9;		/* start from 0, namely id of slab_head */
		unsigned int obj_off:7;		/* offset of obj, step as real_obj_size */
	} c;
} objp_sctx_t;
#endif

typedef struct objp_obj_ops {
	void (*ser_ctor_high)(objpool_t *mpl, void *obj);	/* high ctor for obj deser */
	void (*ser_ctor_low)(objpool_t *mpl);	/* low ctor for obj deser */
	void (*ser_dtor)(objpool_t *mpl);	/* dtor for obj ser */
} objp_oops_t;


/* objpool modes (external) */
#define OBJP_M_NEEDID	(1<<0)

objpool_t *create_objpool(int obj_size, unsigned int mode, objp_oops_t *oops);
void destroy_objpool(objpool_t *mpl);

void *objpool_alloc(objpool_t *mpl);
void *objpool_zalloc(objpool_t *mpl);

void objpool_free(objpool_t *mpl, void *mem);

/* functions for serialization
 * para mpl is pointer of objpool_t */
void *get_sered_obj(objpool_t *mpl, void *obj);

unsigned int objpool_ser_getlen(struct ser_root_data *srd, void *mpl);
char *objpool_ser(struct ser_root_data *srd, void *mpl, char *dst);
char *objpool_deser_high(struct ser_root_data *srd, void *mpl, char *src);
void objpool_deser_low(struct ser_root_data *srd, void *mpl);

/* ser|deser ser_ctx pointer:
 * the *POINTER* is in/out parament, of type union objp_ser_ctx */
void ser_objp_content(objpool_t *mpl, void **p_other_objp);
void deser_objp_content(objpool_t *mpl, void **p_other_objp);

#endif /* _OBJPOOL_H_ */


