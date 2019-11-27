#include <stdint.h>
#include "CuTest.h"
#include "util.h"
#include "util_tests.h"

CuSuite* util_getsuite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_str_hash);
	return suite;
}

void test_str_hash(CuTest *tc)
{
	struct hash_arg ha = {6, 6, 'A', 26, "_-+=", UINT32_MAX};
	CuAssertIntEquals(tc, 9021325, str_hash("UW_VV_AT_THE_BANK", &ha));
	CuAssertIntEquals(tc, 268093709, str_hash("UW_VV_WORKING_LIFE", &ha));
	CuAssertIntEquals(tc, 263231206, str_hash("UW_VV_WEATHER", &ha));
	CuAssertIntEquals(tc, 199250826, str_hash("UW_VV_QUANTITIES", &ha));

	ha.from = 5;
	CuAssertIntEquals(tc, 132676353, str_hash("TS_V_LEISURE_TIME", &ha));

	ha.from = 4;
	CuAssertIntEquals(tc, 447, str_hash("BDT_RF", &ha));
}


