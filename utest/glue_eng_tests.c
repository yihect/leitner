#include "CuTest.h"
#include "vs.h"
#include "glue_eng_tests.h"

CuSuite* glue_eng_getsuite()
{
  CuSuite *suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, test_glue_eng);
  return suite;
}

void test_glue_eng(CuTest *tc)
{
  CuAssertIntEquals(tc, 0, get_business_data_types_value("BDT_PT"));
  CuAssertIntEquals(tc, 1, get_business_data_types_value("BDT_PG"));
  CuAssertIntEquals(tc, 6, get_business_data_types_value("BDT_BDTMAX"));

  CuAssertIntEquals(tc, 0, get_uw_types_value("UW_PT_VOWEL_SUMMARY"));
  CuAssertIntEquals(tc, 137, get_uw_types_value("UW_XX_MAX"));

  CuAssertStrEquals(tc, "TS_RF_SUFFIX", get_rootfixes_ts_types_string(TS_RF_SUFFIX));
  CuAssertStrEquals(tc, "UW_RF_DUMB_PF", get_uw_types_string(UW_RF_DUMB_PF));
}

