#include "CuTest.h"
#include "vs.h"
#include "parser_tests.h"



CuSuite* parser_getsuite()
{
  CuSuite *suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, test_parser);
  return suite;
}
 
/* tests */
void test_parser(CuTest *tc)
{
  ltd_parse_file("./utest/test_parse.ltd");
}



