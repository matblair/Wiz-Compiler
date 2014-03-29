#include "seatest.h"
#include "wizparser.h"

void test_isvalid_assignment(){
    int isValid = 0;
    isValid = wizparse_str("proc main() a:=5; end");
    assert_int_equal(0, isValid);
}


void test_isvalid_while(){
    int isValid = 0;
    isValid = wizparse_str("proc main() while  true do a:=5; od  end");
    assert_int_equal(0, isValid);
}


void  test_suite_stmts( void )
{
	test_fixture_start();               // starts a fixture
	run_test(test_isvalid_assignment);
    run_test(test_isvalid_while);
	test_fixture_end();                 // ends a fixture
}
