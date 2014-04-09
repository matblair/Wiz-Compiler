#include "seatest.h"
#include "pizstringparser.h"

void test_isvalid_true(){
    int isValid = 0;
    isValid = wizparse_str("proc main()  a:= true; end");
    assert_true( isValid==0);
}


void test_isvalid_exprgt(){
    int isValid = 0;
    isValid = wizparse_str("proc main()  a:= a>b; end");
    assert_true( isValid==0);
}

void test_isvalid_exprgte(){
    int isValid = 0;
    isValid = wizparse_str("proc main()  a:= a>=b; end");
    assert_true( isValid==0);
}

void test_isvalid_exprlt(){
    int isValid = 0;
    isValid = wizparse_str("proc main()  a:= a<b; end");
    assert_true( isValid==0);
}

void test_isvalid_exprlte(){
    int isValid = 0;
    isValid = wizparse_str("proc main()  a:= a<=b; end");
    assert_true( isValid==0);
}

void test_isvalid_expreq(){
    int isValid = 0;
    isValid = wizparse_str("proc main()  a:= a=b; end");
    assert_true( isValid==0);
}

void  test_suite_expr_bool( void )
{
	test_fixture_start();               // starts a fixture
	run_test(test_isvalid_true);
	run_test(test_isvalid_exprlt);
	run_test(test_isvalid_exprlte);
	run_test(test_isvalid_exprgt);
	run_test(test_isvalid_exprgte);
	run_test(test_isvalid_expreq);
    test_fixture_end();                 // ends a fixture
}
