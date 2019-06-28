
#include <stdio.h>
#include "CuTest.h"
#include "glue_tests.h"
#include "mempool_tests.h"
#include "objpool_tests.h"
#include "cvspool_tests.h"
#include "objvec_tests.h"
#include "bitmap_tests.h"
#include "idr_tests.h"
#include "msm_tests.h"
#include "parser_tests.h"


void RunAllTests(void)
{
	CuString *output = CuStringNew();
	CuSuite *suite = CuSuiteNew();

	CuSuiteAddSuite(suite, glue_getsuite());
	CuSuiteAddSuite(suite, mempool_getsuite());
	CuSuiteAddSuite(suite, objpool_getsuite());
	CuSuiteAddSuite(suite, cvspool_getsuite());
	CuSuiteAddSuite(suite, objvec_getsuite());
	CuSuiteAddSuite(suite, bitmap_getsuite());
	CuSuiteAddSuite(suite, parser_getsuite());
	CuSuiteAddSuite(suite, idr_getsuite());
	CuSuiteAddSuite(suite, msm_getsuite());

	CuSuiteRun(suite);
	CuSuiteSummary(suite,output);
	CuSuiteDetails(suite,output);
	printf("%s\n", output->buffer);
}





