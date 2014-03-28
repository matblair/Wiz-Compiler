#include "seatest.h"

void test_fixture_one( void );
void test_suite_program_outline(void);
void test_suite_proc_args(void);

void all_tests( void )
{
	test_fixture_one();
    test_suite_program_outline();
    test_suite_proc_args();
	// add new test fixtures here.
}

void my_suite_setup( void )
{
	//printf("I'm done before every single test in the suite\r\n");
}

void my_suite_teardown( void )
{
	//printf("I'm done after every single test in the suite\r\n");
}

int main( int argc, char** argv )
{
	suite_setup(my_suite_setup);
	suite_teardown(my_suite_teardown);
	run_tests(all_tests);	
	return 0;
}

