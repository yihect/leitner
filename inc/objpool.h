#ifndef _OBJPOOL_H_
#define _OBJPOOL_H_

struct slab {
    void *free;
    struct slab *next;
};

typedef struct slab slab_t;

struct objpool {
    int obj_size;
    int real_obj_size;
    int slab_size;
    int slab_nr;
    int obj_nr;
    int free_obj_nr;
    slab_t *slab_head;
    slab_t *recent_slab;
};

typedef struct objpool objpool_t;

objpool_t *create_objpool(int obj_size);
void destroy_objpool(objpool_t *mpl);

void *objpool_alloc(objpool_t *mpl);
void *objpool_zalloc(objpool_t *mpl);

void objpool_free(objpool_t *mpl, void *mem);

#endif /* _OBJPOOL_H_ */
