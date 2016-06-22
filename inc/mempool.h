#ifndef _INCLUDE_MEMPOOL
#define _INCLUDE_MEMPOOL

typedef struct mem_chunk_s {
   int guard0;
   struct mem_chunk_s * next;
   struct mem_chunk_s * prev;
   int handle;
   int guard1;
} mem_chunk_t;

typedef struct mem_handle_s {
   mem_chunk_t * ptr; /* can be changed by alloc_mem_chunk if status is FREE */
   int status;        /* FREE or LOCK */
   int size;          /* size of this chunk (could be derived from ptr->next) */
} mem_handle_t;

typedef struct mem_pool_s {
   int chunks;        /* max number of handles we can give out */
   int size;          /* original size requested */
   int alloced;       /* How much memory has been allocated */
   int free;          /* how much memory is free */
   mem_handle_t * handles; /* Pointer to memory pool handle array */
   mem_chunk_t * last;/* keep track of last chunk in pool */
   int nosplit;       /* reuse a fragment as-is if wasted space below this */
   int num_allocs;    /* Total number of allocations */
   int num_slides;    /* Number of times fragment copies were needed */
} mem_pool_t;


void * init_mem_pool(int chunks, int size, int reuse);
void * lock_mem_chunk(void * pool, int handle);
void   release_mem_chunk(void * pool, int handle);
void   free_mem_chunk(void * pool, int handle);
int    alloc_mem_chunk(void * pool, int size);
void   free_mem_pool(void * pool);
void   mem_pool_stats(void * pool);

#endif

