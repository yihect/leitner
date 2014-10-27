#ifndef _MEMMAN_TESTS_
#define _MEMMAN_TESTS_

#include "CuTest.h"
#include "memman.h"


CuSuite* memmam_getsuite();

/* helper functions, macro */
void init_mem_node(struct mem_node *pn, char *addr, 
						unsigned long len, struct mem_node *pnext);

struct mem_node *find_node(struct mem_node *list, char *addr);
int node_num(struct mem_node *list);




/* tests */
void test_mem_init(CuTest *tc);
void test_mem_alloc(CuTest *tc);
void test_mem_combine(CuTest *tc);
void test_mem_free(CuTest *tc);



#endif /* _MEMMAN_TESTS_ */	



