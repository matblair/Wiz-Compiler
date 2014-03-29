#include "seatest.h"
#include "wizparser.h"

void test_isvalid_noargs(){
    int isValid = 0;
    isValid = wizparse_str("proc main() a:=5; end");
    assert_int_equal(0, isValid);
}


void test_isvalid_onevalarg(){
    int is_valid;
    is_valid =  wizparse_str("proc main(val int a) a:=5; end ");
    assert_true(is_valid == 0);
}

void test_isvalid_multvalarg(){
    int is_valid;
    is_valid =  wizparse_str("proc main(val int a, val int b) a:=5; end ");
    assert_true(is_valid == 0);
}

void test_isvalid_refarg(){
    int is_valid;
    is_valid =  wizparse_str("proc main(ref int a) a:=5; end ");
    assert_true(is_valid == 0);
}

void test_isvalid_argarray(){
    int is_valid;
    is_valid =  wizparse_str("proc main(ref int a[1..2]) a:=5; end ");
    assert_true(is_valid == 1);
}

void test_isvalid_refvalmutiarg(){
    int is_valid;
    is_valid =  wizparse_str("proc main(ref int a, val int a) a:=5; end ");
    assert_true(is_valid == 0);
}

void test_isvalid_arginttype(){
    int is_valid;
    is_valid =  wizparse_str("proc main(ref int a) a:=5; end ");
    assert_true(is_valid == 0);
}

void test_isvalid_argbooltype(){
    int is_valid;
    is_valid =  wizparse_str("proc main(ref bool a) a:=5; end ");
    assert_true(is_valid == 0);
}


void test_isvalid_argfloattype(){
    int is_valid;
    is_valid =  wizparse_str("proc main(ref float a) a:=5; end ");
    assert_true(is_valid == 0);
}

void test_isvalid_morethan3args(){
    int is_valid;
    is_valid =  wizparse_str("proc main(ref float a, val int a, ref int b) a:=5; end ");
    assert_true(is_valid == 0);
}


void test_isinvalid_argsendwithcomma(){
    int is_valid;
    is_valid =  wizparse_str("proc main(ref int a, ) a:=5; end ");
    assert_true(is_valid != 0);
}

void  test_suite_proc_args( void )
{
	test_fixture_start();               // starts a fixture
	run_test(test_isvalid_noargs);
    run_test(test_isvalid_multvalarg);
    run_test(test_isvalid_refarg);
    run_test(test_isvalid_onevalarg);
    run_test(test_isvalid_refvalmutiarg);
    run_test(test_isvalid_morethan3args);
    run_test(test_isvalid_arginttype);
    run_test(test_isvalid_argbooltype);
    run_test(test_isvalid_argfloattype);
    run_test( test_isvalid_argarray);
    run_test(test_isinvalid_argsendwithcomma);
	test_fixture_end();                 // ends a fixture
}
