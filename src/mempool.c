/*******************************************************************
 * Aaron Logue 2003
 * Simple lock/release style memory pool for holding pointerless BSP data.
 * Chunks may move around in memory while not locked.
 * We know how many chunks we need ahead of time.
 * No lock counts; we're just after the memory consolidation here.
 *
 * The best way to use this is to choose a small number of chunks
 * and a large total pool size.  When alloc_mem_chunk is called,
 * it's best to have no chunks locked.  Locks and releases are fast.
 * One chunk per tile would be good.
 *
 * If we find an unused fragment that is larger than the requested
 * size, we must decide between using it as-is (and wasting some
 * memory) or splitting the fragment and leaving an even smaller
 * unused fragment in the pool.  The former is more efficient from
 * a memory-usage standpoint, and the latter is more efficient from
 * the standpoint of reducing time-consuming free fragment coalescing.
 *
 * nosplit is the amount of unused memory below which a fragment will
 * be reused as-is.
 *
 * To reduce memory pool thrashing, compute nosplit thus:
 * (poolsize / chunks / N) where N is somewhere around 10 or 20.
 * 1/N is the portion of memory that will be sacrificed to reduce
 * thrashing.
 *******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mempool.h"
 
#define MEM_FREE_SIG 0x45455246
#define MEM_ALOC_SIG 0x434F4C41
#define MEM_LOCK_SIG 0x4B434F4C

#define Pool ((mem_pool_t *)pool)

/*******************************************************************
 * init_mem_pool
 *
 * returns a pointer to main memory pool
 *******************************************************************/
void * init_mem_pool(int chunks, int size, int nosplit)
{
   mem_pool_t    * pool;
   mem_handle_t  * handles;
   mem_chunk_t   * chunk;
   int           i;

   if (nosplit < sizeof(mem_chunk_t)) {
      nosplit = sizeof(mem_chunk_t);
   }

   pool = (mem_pool_t *)malloc(sizeof(mem_pool_t));
   if (pool) {
      handles = (mem_handle_t *)malloc(chunks * 2 * sizeof(mem_handle_t));
      if (handles) {
         for (i=0;i<chunks*2;i++) {
            handles[i].ptr = NULL;
         }
         chunk = (mem_chunk_t *)malloc(size + chunks * 2 * sizeof(mem_chunk_t));
         if (chunk) {
            pool->chunks      = chunks * 2;
            pool->size        = size;
            pool->alloced     = 0;
            pool->free        = size + chunks * 2 * sizeof(mem_chunk_t) - sizeof(mem_chunk_t);
            pool->handles     = handles;
            pool->last        = chunk;
            pool->nosplit     = nosplit;
            pool->num_allocs  = 0;
            pool->num_slides  = 0;
            
         
            handles[0].ptr    = chunk;
            handles[0].status = MEM_FREE_SIG;
            handles[0].size   = size + chunks * 2 * sizeof(mem_chunk_t) - sizeof(mem_chunk_t);
            
            chunk->guard0     = MEM_FREE_SIG;
            chunk->next       = NULL;
            chunk->prev       = NULL;
            chunk->handle     = 0;
            chunk->guard1     = MEM_FREE_SIG;
         } else {
            free(handles);
            free(pool);
            pool = NULL;
         }
      } else {
         free(pool);
         pool = NULL;
      }
   }
   return (void *)pool;
}

/*******************************************************************
 * free_mem_pool
 *******************************************************************/
void free_mem_pool(void * pool)
{
   free(Pool->handles[0].ptr);
   free(Pool->handles);
   free(pool);
}

/*******************************************************************
 * lock_mem_chunk
 *
 * Given a handle, lock memory in place and return a pointer.
 *******************************************************************/
void * lock_mem_chunk(void * pool, int handle)
{
   void * result;
   result = NULL;
   
   if (Pool->handles[handle].status == MEM_ALOC_SIG) {
      if (Pool->handles[handle].ptr->guard0 == MEM_ALOC_SIG &&
          Pool->handles[handle].ptr->guard1 == MEM_ALOC_SIG) {
         /* Transit chunk to locked state */
         Pool->handles[handle].status      = MEM_LOCK_SIG;
         Pool->handles[handle].ptr->guard0 = MEM_LOCK_SIG;
         Pool->handles[handle].ptr->guard1 = MEM_LOCK_SIG;
         result = (void *)Pool->handles[handle].ptr;
         result = (void *)((char *)result + sizeof(mem_chunk_t));
      } else {
         /* error - memory corruption */
printf("memory corruption\n");         
      }
   } else {
      /* error - chunk was not in a released state */
printf("memory corruption\n");         
   }
   return result;
}


/*******************************************************************
 * release_mem_chunk
 *
 * Given a handle, release it, which allows alloc_mem_chunk to move it.
 *******************************************************************/
void release_mem_chunk(void * pool, int handle)
{
   if (Pool->handles[handle].status == MEM_LOCK_SIG) {
      if (Pool->handles[handle].ptr->guard0 == MEM_LOCK_SIG &&
          Pool->handles[handle].ptr->guard1 == MEM_LOCK_SIG) {
         /* Transit chunk to released state */
         Pool->handles[handle].status      = MEM_ALOC_SIG;
         Pool->handles[handle].ptr->guard0 = MEM_ALOC_SIG;
         Pool->handles[handle].ptr->guard1 = MEM_ALOC_SIG;
      } else {
         /* error - memory corruption */
printf("memory corruption\n");         
      }
   } else {
      /* error - chunk was not in a locked state */
printf("memory corruption\n");         
   }
}

/*******************************************************************
 * merge_chunks
 *
 * Try to keep things tidy after a chunk is freed. Returns handle of
 * chunk that things were merged to, or 0 if no merging occurred.
 *******************************************************************/
int merge_chunks(void * pool, int handle)
{
   mem_chunk_t * chunk;
   int result;

   result = 0;
   /* Attempt to merge with previous chunk */
   chunk = Pool->handles[handle].ptr->prev;
   if (chunk && Pool->handles[chunk->handle].status == MEM_FREE_SIG) {
      /* Make previous chunk leapfrog the original chunk */
      Pool->handles[chunk->handle].ptr->next = Pool->handles[handle].ptr->next;
      /* Fixup next chunk's prev ptr */
      if (Pool->handles[chunk->handle].ptr->next == NULL) {
         Pool->last = Pool->handles[chunk->handle].ptr;
      } else {
         Pool->handles[chunk->handle].ptr->next->prev = Pool->handles[chunk->handle].ptr;
      }
      /* Previous chunk grows by this much */
      Pool->handles[chunk->handle].size +=
         Pool->handles[handle].size + sizeof(mem_chunk_t);
      /* Destroy the original chunk header (not really necessary) */
      Pool->handles[handle].ptr->guard0 = 0;
      Pool->handles[handle].ptr->next   = NULL;
      Pool->handles[handle].ptr->prev   = NULL;
      Pool->handles[handle].ptr->guard1 = 0;
      /* free the handle for reassignment */
      Pool->handles[handle].ptr = NULL;
      /* Because we eliminated a mem_chunk_t, we get a little bonus */
      Pool->free += sizeof(mem_chunk_t);
      /* Make it look like the previous chunk is the original chunk */
      handle = chunk->handle;
      result = handle;
   }
   /* Attempt to merge with next chunk */
   chunk = Pool->handles[handle].ptr->next;
   if (chunk && Pool->handles[chunk->handle].status == MEM_FREE_SIG) {
      /* leapfrog the next chunk */
      Pool->handles[handle].ptr->next = Pool->handles[chunk->handle].ptr->next;
      /* Fixup next chunk's prev ptr */
      if (Pool->handles[handle].ptr->next == NULL) {
         Pool->last = Pool->handles[handle].ptr;
      } else {
         Pool->handles[handle].ptr->next->prev = Pool->handles[handle].ptr;
      }
      /* chunk grows by this much */
      Pool->handles[handle].size +=
         Pool->handles[chunk->handle].size + sizeof(mem_chunk_t);
      /* Destroy the chunk header (not really necessary) */
      Pool->handles[chunk->handle].ptr->guard0 = 0;
      Pool->handles[chunk->handle].ptr->next   = NULL;
      Pool->handles[chunk->handle].ptr->prev   = NULL;
      Pool->handles[chunk->handle].ptr->guard1 = 0;
      /* free the handle for reassignment */
      Pool->handles[chunk->handle].ptr = NULL;
      /* Because we eliminated a mem_chunk_t, we get a little bonus */
      Pool->free += sizeof(mem_chunk_t);
      /* indicate that a merge took place */
      result = handle;
   }
   return result;
}

/*******************************************************************
 * free_mem_chunk
 *
 * given a handle, free it and try to consolidate it with neighbors
 *******************************************************************/
void free_mem_chunk(void * pool, int handle)
{
   if (Pool->handles[handle].status == MEM_ALOC_SIG) {
      if (Pool->handles[handle].ptr->guard0 == MEM_ALOC_SIG &&
          Pool->handles[handle].ptr->guard1 == MEM_ALOC_SIG) {
         /* release chunk back into pool */
         Pool->handles[handle].status      = MEM_FREE_SIG;
         Pool->handles[handle].ptr->guard0 = MEM_FREE_SIG;
         Pool->handles[handle].ptr->guard1 = MEM_FREE_SIG;
         Pool->alloced -= Pool->handles[handle].size;
         Pool->free    += Pool->handles[handle].size;
         while (handle = merge_chunks(pool, handle)) {}
      } else {
         /* error - memory corruption */
printf("memory corruption\n");         
      }
   } else {
      /* error - chunk was not in a locked state */
printf("memory corruption\n");         
   }
}

/*******************************************************************
 * slide_chunk_up
 *
 * This function makes the following assumptions:
 * chunk->guard0 is MEM_ALOC_SIG
 * chunk->next is not NULL and chunk->next->guard0 is MEM_FREE_SIG
 *******************************************************************/
void slide_chunk_up(void * pool, mem_chunk_t * chunk)
{
   char * source;
   char * dest;
   mem_chunk_t * newchunk;
   mem_chunk_t freegoo;

   Pool->num_slides++;

   /* Save free chunk header data off before we overwrite it */
   freegoo.handle = chunk->next->handle;
   freegoo.next   = chunk->next->next;

   /* Compute location of destination mem_chunk_t */
   newchunk = (mem_chunk_t *)((char *)chunk + Pool->handles[chunk->next->handle].size + sizeof(mem_chunk_t));
   /* Location of destination data (probably overlaps) */
   dest = (char *)newchunk + sizeof(mem_chunk_t);
   source = (char *)chunk + sizeof(mem_chunk_t);
   /* Copy the chunk contents */
   memmove(dest, source, Pool->handles[chunk->handle].size);
   
   newchunk->guard0 = chunk->guard0;
   newchunk->next   = freegoo.next;
   if (newchunk->next == NULL) {
      Pool->last = newchunk;
   } else {
      newchunk->next->prev = newchunk;
   }
   newchunk->prev   = chunk;
   newchunk->handle = chunk->handle;
   newchunk->guard1 = chunk->guard1;
   
   chunk->guard0    = MEM_FREE_SIG;
   chunk->next      = newchunk;
   chunk->handle    = freegoo.handle;
   chunk->guard1    = MEM_FREE_SIG;
   
   Pool->handles[chunk->handle].ptr = chunk;
   Pool->handles[newchunk->handle].ptr = newchunk;
}

/*******************************************************************
 * attempt_to_alloc
 *
 * This function attempts to allocate the requested amount of memory
 * without doing any sliding.  If the smallest closest chunk is over
 * twice the requested size, then it will split the chunk.
 *******************************************************************/
int attempt_to_alloc(void * pool, int size)
{
   int handle = 0;
   int unused_handle = 0;
   int small_handle = 0;

   /* find the smallest fragment bigger than or equal to size */
   while (handle < Pool->chunks) {
      if (Pool->handles[handle].ptr) {
         if (Pool->handles[handle].status == MEM_FREE_SIG) {
            if (Pool->handles[handle].size >= size) {
               if (small_handle == 0 || Pool->handles[handle].size <= Pool->handles[small_handle].size) {
                  small_handle = handle;
               }
            }
         }
      } else {
         unused_handle = handle;
      }
      handle++;
   }

   /* if we found none, see if the handle 0 pool is large enough */
   if (small_handle == 0 &&
         Pool->handles[0].size >= size + (int)sizeof(mem_chunk_t)) {
      /* split the handle 0 pool by taking a piece off the end */
      handle = unused_handle;
      
      Pool->handles[handle].ptr         =
         (mem_chunk_t *)((char *)Pool->handles[0].ptr + (Pool->handles[0].size - size));
      Pool->handles[handle].status      = MEM_ALOC_SIG;
      Pool->handles[handle].size        = size;

      Pool->handles[handle].ptr->guard0 = MEM_ALOC_SIG;
      Pool->handles[handle].ptr->next   = Pool->handles[0].ptr->next;
      /* Stay on top of last chunk if it changed */
      if (Pool->handles[handle].ptr->next == NULL) {
         Pool->last = Pool->handles[handle].ptr;
      } else {
         Pool->handles[handle].ptr->next->prev = Pool->handles[handle].ptr;
      }
      Pool->handles[handle].ptr->prev   = Pool->handles[0].ptr;
      Pool->handles[handle].ptr->handle = handle;
      Pool->handles[handle].ptr->guard1 = MEM_ALOC_SIG;
      
      Pool->handles[0].ptr->next = Pool->handles[handle].ptr;
      Pool->handles[0].size -= (size + sizeof(mem_chunk_t));

      Pool->alloced += size;
      Pool->free    -= (size + sizeof(mem_chunk_t));

      return handle;
   }

   /* if we found a small non-0 handle that's large enough, use it */
   if (small_handle) {
      handle = small_handle;
      /*
       * If we found an unused fragment that's bigger than the requested
       * size, we must decide between using it as-is (and wasting some
       * memory) or splitting the fragment and leaving an even smaller
       * unused fragment in the pool.  The former is more efficient from
       * a memory-usage standpoint, and the latter is more efficient from
       * the standpoint of reducing time-consuming free fragment coalescing.
       *
       * In order to split, the remaining free fragment must be at least
       * the size of one mem_chunk_t, so the minimum amount of wasted space
       * will be sizeof(mem_chunk_t).  We let the caller specify the amount
       * of unused space below which a fragment will be reused as-is.
       */
      if (size >= Pool->handles[handle].size - Pool->nosplit) {
         Pool->handles[handle].status      = MEM_ALOC_SIG;
         Pool->handles[handle].ptr->guard0 = MEM_ALOC_SIG;
         Pool->handles[handle].ptr->guard1 = MEM_ALOC_SIG;
         
         Pool->alloced += Pool->handles[handle].size;
         Pool->free    -= Pool->handles[handle].size;

         return handle;
      } else {

         /* Decide whether to make empty chunk next or prev */
         if (Pool->handles[handle].ptr->next &&
             Pool->handles[handle].ptr->next->guard0 == MEM_FREE_SIG) {
            /* Split to merge empty chunk with next */
            Pool->handles[unused_handle].ptr = (mem_chunk_t *)
               ((char *)Pool->handles[handle].ptr + size + sizeof(mem_chunk_t));
            Pool->handles[unused_handle].status = MEM_FREE_SIG;
            Pool->handles[unused_handle].size =
               Pool->handles[handle].size - size - sizeof(mem_chunk_t);

            Pool->handles[unused_handle].ptr->guard0 = MEM_FREE_SIG;
            Pool->handles[unused_handle].ptr->next = Pool->handles[handle].ptr->next;
            /* Stay on top of last chunk if it changed */
            if (Pool->handles[unused_handle].ptr->next == NULL) {
               Pool->last = Pool->handles[unused_handle].ptr;
            } else {
               Pool->handles[unused_handle].ptr->next->prev = Pool->handles[unused_handle].ptr;
            }
            Pool->handles[unused_handle].ptr->prev = Pool->handles[handle].ptr;
            Pool->handles[unused_handle].ptr->handle = unused_handle;
            Pool->handles[unused_handle].ptr->guard1 = MEM_FREE_SIG;

            Pool->handles[handle].status = MEM_ALOC_SIG;
            Pool->handles[handle].size   = size;
            Pool->handles[handle].ptr->guard0 = MEM_ALOC_SIG;
            Pool->handles[handle].ptr->next = Pool->handles[unused_handle].ptr;
            Pool->handles[handle].ptr->guard1 = MEM_ALOC_SIG;
            
            Pool->alloced += size;
            Pool->free    -= (size + sizeof(mem_chunk_t));

            /* Merge with free next chunk */
            while (unused_handle = merge_chunks(pool, unused_handle)) {}

            return handle;
         }

         /* Split and attempt to merge empty chunk with prev */
         handle = unused_handle;
      
         Pool->handles[handle].ptr         = (mem_chunk_t *)
            ((char *)Pool->handles[small_handle].ptr + (Pool->handles[small_handle].size - size));
         Pool->handles[handle].status      = MEM_ALOC_SIG;
         Pool->handles[handle].size        = size;

         Pool->handles[handle].ptr->guard0 = MEM_ALOC_SIG;
         Pool->handles[handle].ptr->next   = Pool->handles[small_handle].ptr->next;
         /* Stay on top of last chunk if it changed */
         if (Pool->handles[handle].ptr->next == NULL) {
            Pool->last = Pool->handles[handle].ptr;
         } else {
            Pool->handles[handle].ptr->next->prev = Pool->handles[handle].ptr;
         }
         Pool->handles[handle].ptr->prev   = Pool->handles[small_handle].ptr;
         Pool->handles[handle].ptr->handle = handle;
         Pool->handles[handle].ptr->guard1 = MEM_ALOC_SIG;
      
         Pool->handles[small_handle].ptr->next = Pool->handles[handle].ptr;
         Pool->handles[small_handle].size -= (size + sizeof(mem_chunk_t));

         Pool->alloced += size;
         Pool->free    -= (size + sizeof(mem_chunk_t));

         /* Merge with free prev chunk */
         while (small_handle = merge_chunks(pool, small_handle)) {}

         return handle;
      }   
   }
   /* Indicate failure */
   return 0;
}

/*******************************************************************
 * alloc_mem_chunk
 *
 * Creates a memory chunk and returns a handle
 * This function may move unlocked chunks around to try to consolidate
 *
 * Find the smallest unused chunk of memory that's big enough
 *   if we have sufficient handles remaining, split and return handle
 *   otherwise, just return handle
 * If no chunks found, slide released chunk up and try to merge
 *******************************************************************/
int alloc_mem_chunk(void * pool, int size)
{
   int result;
   mem_chunk_t * chunk;
   int handle;

   if (size <= 0) {
      return 0;
   }
 
   if (Pool->free < size) {
      /* we are out of memory - no amount of consolidation would help */
      return 0;
   }

   Pool->num_allocs++;
   
   result = attempt_to_alloc(pool, size);

   if (!result) {
      /*
       * Walk backwards through the pool, identify moveable chunk with a
       * next that's free and slide up to bubble free space downward.
       */
      chunk = Pool->last;
      while (chunk) {
         if (chunk->guard0 == MEM_ALOC_SIG &&
             chunk->next && chunk->next->guard0 == MEM_FREE_SIG) {
             
            slide_chunk_up(pool, chunk); /* chunk becomes the free chunk */
            if (handle = merge_chunks(pool, chunk->handle)) {
               if (Pool->handles[handle].size >= size + (int)sizeof(mem_chunk_t)) {
                  /* we successfully coalesced a free chunk large enough to use */
                  result = attempt_to_alloc(pool, size);
                  break;
               } else {
                  /* reset the slider and try again */
                  chunk = Pool->last;
               }
            } else {
               if (Pool->handles[0].size >= size + (int)sizeof(mem_chunk_t)) {
                  /* we successfully coalesced all the way back to handle 0 */
                  result = attempt_to_alloc(pool, size);
                  break;
               }
            }
         }
         chunk = chunk->prev;
      }   
   }
   return result;
}

/*******************************************************************
 * mem_pool_stats
 *******************************************************************/
void mem_pool_stats(void * pool)
{
   mem_chunk_t * chunk;
   mem_chunk_t * prev;
   char status[5];
   char * endptr;

   printf("Alloced %d  /  Free %d   /   NumAllocs %d   /   NumSlides %d\n",
      Pool->alloced, Pool->free, Pool->num_allocs, Pool->num_slides);

   prev = NULL;
   chunk = Pool->handles[0].ptr;

   while (chunk) {
   
      if (chunk->guard0 == MEM_FREE_SIG && chunk->guard1 == MEM_FREE_SIG) {
         strcpy(status, "FREE");
      } else
      if (chunk->guard0 == MEM_ALOC_SIG && chunk->guard1 == MEM_ALOC_SIG) {
         strcpy(status, "ALOC");
      } else
      if (chunk->guard0 == MEM_LOCK_SIG && chunk->guard1 == MEM_LOCK_SIG) {
         strcpy(status, "LOCK");
      } else {
         strcpy(status, "CRAP");
      }

      endptr = (char *)chunk + sizeof(mem_chunk_t) + Pool->handles[chunk->handle].size;

      printf("[%08lux - %08lux] <%08lux >%08lux  %3d  %s  %7d\n",
         (unsigned long)chunk,
         (unsigned long)endptr,
         (unsigned long)chunk->prev,
         (unsigned long)chunk->next,
         chunk->handle,
         status,
         Pool->handles[chunk->handle].size);

      prev = chunk;
      chunk = chunk->next;
      if (chunk) {
         if ((unsigned long)endptr < (unsigned long)chunk) {
            printf("Error: %lu orphaned bytes follows chunk\n", (char *)chunk - endptr);
         }
         if ((unsigned long)endptr > (unsigned long)chunk) {
            printf("Error: chunk overlaps next chunk by %lu bytes\n", endptr - (char *)chunk);
         }
         if (chunk->prev != prev) {
            printf("Error: handle %3d chunk->prev chain broken\n", chunk->handle);
         }
      }
      
   }
   if (Pool->last != prev) {
      printf("Error: Pool->last %08lux but prev is %08lux\n", (unsigned long)Pool->last, (unsigned long)prev);
   }
}
