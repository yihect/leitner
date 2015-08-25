
#include <stdio.h>
#include "CuTest.h"
#include "glue_tests.h"


void RunAllTests(void)
{
	CuString *output = CuStringNew();
	CuSuite *suite = CuSuiteNew();

	CuSuiteAddSuite(suite,glue_getsuite());

	CuSuiteRun(suite);
	CuSuiteSummary(suite,output);
	CuSuiteDetails(suite,output);
	printf("%s\n", output->buffer);
}





