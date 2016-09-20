#include "CuTest.h"
#include "mempool.h"
#include "mempool_tests.h"

CuSuite* mempool_getsuite()
{
  CuSuite *suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, test_init_mempool);
  SUITE_ADD_TEST(suite, test_alloc_mempool);
  return suite;
}
 
void test_init_mempool(CuTest *tc)
{
  printf("test_init_mempool() testing...\n");
  int s;

  mem_pool_t *p = (mem_pool_t *)init_mem_pool(1024, 120*1024*1024, 32);
  s = 120*1024*1024+(2*1024-1)*sizeof(mem_chunk_t);
  mem_pool_stats(p);
  CuAssertIntEquals(tc, s, p->free);
  CuAssertIntEquals(tc, s, p->handles[0].size);
  CuAssertPtrEquals(tc, p->handles[0].ptr, p->last);

  free_mem_pool(p);
  printf("\n");
}

void test_alloc_mempool(CuTest *tc)
{
  printf("test_alloc_mempool() testing...\n");
  printf("sizeof(mem_chunk_t) is %d\n", (int)sizeof(mem_chunk_t));
  int s;
  mem_pool_t *p = (mem_pool_t *)init_mem_pool(1024, 120*1024*1024, 32);

  mem_pool_stats(p);
  int h1 = alloc_mem_chunk(p, 60*1024*1024);
  s = 60*1024*1024+(2*1024-2)*sizeof(mem_chunk_t);
  mem_pool_stats(p);
  CuAssertIntEquals(tc, s, p->free);
  CuAssertIntEquals(tc, s, p->handles[0].size);
  //CuAssertPtrEquals(tc, p->handles[0].ptr, p->last);


  free_mem_pool(p);
  printf("\n");
}



