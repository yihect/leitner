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
  unsigned gbm=0, gbma[2]={0};

  //for gbm format type 0
  mark_offset(&gbm, 8, true);
  CuAssertIntEquals(tc, 0x04000000, gbm);
  CuAssertTrue(tc, is_offset_valid(&gbm, 8));
  mark_offset(&gbm, 0, true);
  CuAssertIntEquals(tc, 0x04040000, gbm);
  CuAssertTrue(tc, is_offset_valid(&gbm, 0));
  set_offset(&gbm, 7, 4);
  CuAssertIntEquals(tc, 0x06040000, gbm);
  CuAssertIntEquals(tc, 4, get_offset(&gbm, 7));
  set_offset(&gbm, 7, 5);
  CuAssertIntEquals(tc, 0x06040400, gbm);
  CuAssertIntEquals(tc, 5, get_offset(&gbm, 7));

  //for gbm format type 1
  gbma[0] |= GBM_FORMAT_TYPE_MASK;
  mark_offset(gbma, 16, true);
  CuAssertIntEquals(tc, 0x0c000000, gbma[0]);
  CuAssertTrue(tc, is_offset_valid(gbma, 16));
  mark_offset(&gbma, 0, true);
  CuAssertIntEquals(tc, 0x0c000400, gbma[0]);
  CuAssertTrue(tc, is_offset_valid(gbma, 0));
  set_offset(gbma, 13, 13);
  CuAssertIntEquals(tc, 0x0c800400, gbma[0]);
  CuAssertIntEquals(tc, 0x28000000, gbma[1]);
  CuAssertIntEquals(tc, 13, get_offset(gbma, 13));
  set_offset(gbma, 14, 9);
  CuAssertIntEquals(tc, 0x0d800401, gbma[0]);
  CuAssertIntEquals(tc, 0x28000000, gbma[1]);
  CuAssertIntEquals(tc, 9, get_offset(gbma, 14));
  set_offset(gbma, 7, 4);
  CuAssertIntEquals(tc, 4, get_offset(gbma, 7));
  set_offset(gbma, 15, 8);
  CuAssertIntEquals(tc, 8, get_offset(gbma, 15));
}


