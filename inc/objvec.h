#ifndef _OBJVEC_H_
#define _OBJVEC_H_

#include "cvspool.h"

/* an simple object vector pool build on cvspool */

typedef void (*objctor)(char *obj, void *arg);
typedef void (*objdtor)(char *obj);

struct objvec {
	cvspool *cp;
	unsigned int size;	// in bytes
	objctor ctor;
	void *arg;
	objdtor dtor;
};

struct objvec *objv_init(unsigned int osize, objctor ctor, void *arg, objdtor dtor);
void objv_exit(struct objvec *ov);

char *objv_alloc(struct objvec *ov, unsigned obj_cnt);
char *objv_realloc(struct objvec *ov, char *oldobjs, unsigned new_cnt,
		   void (*adjust_fn)(char *oldobjs, char *newobjs, void *adj_pram),
		   void *adjparam);
void objv_free(struct objvec *ov, char *objs);

#endif /* _OBJVEC_H_ */
