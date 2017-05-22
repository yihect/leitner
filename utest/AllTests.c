
#include <stdio.h>
#include "CuTest.h"
#include "glue_tests.h"
#include "mempool_tests.h"
#include "parser_tests.h"


void RunAllTests(void)
{
	CuString *output = CuStringNew();
	CuSuite *suite = CuSuiteNew();

	CuSuiteAddSuite(suite, glue_getsuite());
	CuSuiteAddSuite(suite, mempool_getsuite());
	CuSuiteAddSuite(suite, parser_getsuite());

	CuSuiteRun(suite);
	CuSuiteSummary(suite,output);
	CuSuiteDetails(suite,output);
	printf("%s\n", output->buffer);
}





