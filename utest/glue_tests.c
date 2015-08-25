#include "CuTest.h"
#include "vs.h"
#include "glue_tests.h"

CuSuite* glue_getsuite()
{
  CuSuite *suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, test_bbits);
  SUITE_ADD_TEST(suite, test_set_get_offset);
  return suite;
}
 
void test_bbits(CuTest *tc)
{
  CuAssertIntEquals(tc, 1, bbits(0));
  CuAssertIntEquals(tc, 1, bbits(1));
  CuAssertIntEquals(tc, 1, bbits(2));
  CuAssertIntEquals(tc, 1, bbits(3));
  CuAssertIntEquals(tc, 2, bbits(4));
  CuAssertIntEquals(tc, 2, bbits(5));
  CuAssertIntEquals(tc, 2, bbits(6));
  CuAssertIntEquals(tc, 2, bbits(7));
  CuAssertIntEquals(tc, 3, bbits(8));
  CuAssertIntEquals(tc, 3, bbits(9));
  CuAssertIntEquals(tc, 3, bbits(10));
  CuAssertIntEquals(tc, 3, bbits(11));
  CuAssertIntEquals(tc, 3, bbits(12));
  CuAssertIntEquals(tc, 3, bbits(13));
}

void test_set_get_offset(CuTest *tc)
{
}


