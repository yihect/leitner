#ifndef _MEMMAN_TESTS_
#define _MEMMAN_TESTS_

#include "CuTest.h"
#include "vs.h"


CuSuite* glue_getsuite();

#if 0
/* helper functions, macro */
void init_mem_node(struct mem_node *pn, char *addr, 
						unsigned long len, struct mem_node *pnext);

struct mem_node *find_node(struct mem_node *list, char *addr);
int node_num(struct mem_node *list);
#endif




/* tests */
void test_bbits(CuTest *tc);
void test_set_get_offset(CuTest *tc);
void test_setoffset(CuTest *tc);



#endif /* _MEMMAN_TESTS_ */	



