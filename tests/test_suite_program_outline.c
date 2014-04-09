#include "seatest.h"
#include "pizstringparser.h"

void test_isvalid_singleproc(){
    int isValid = 0;
    isValid = wizparse_str("proc main() a:=5; end");
    assert_int_equal(0, isValid);
}

void test_isinvalid_justproc(){
    int is_valid;
    is_valid =  wizparse_str("proc ");
    assert_true( is_valid != 0);
    
}

void test_isvalid_morethanoneproc(){
    int is_valid;
    is_valid =  wizparse_str("proc main() a:=5; end proc main2() j:=10; end");
    assert_true(is_valid ==0);
}

void test_isinvalid_procemptybody(){
    int is_valid;
    is_valid =  wizparse_str("proc main() end");
    assert_true(is_valid !=0 );
}

void  test_suite_program_outline( void )
{
	test_fixture_start();               // starts a fixture
    run_test(test_isvalid_singleproc);
	run_test(test_isvalid_morethanoneproc);
    run_test(test_isinvalid_procemptybody);
    run_test(test_isinvalid_justproc);
	test_fixture_end();                 // ends a fixture
}
