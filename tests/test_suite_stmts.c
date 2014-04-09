#include "seatest.h"
#include "pizstringparser.h"

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
void test_isvalid_ifthen(){
    int isValid = 0;
    isValid = wizparse_str("proc main() if   true then  a:=5; fi  end");
    assert_int_equal(0, isValid);
}

void test_isvalid_ifthenelse(){
    int isValid = 0;
    isValid = wizparse_str("proc main() if   true then  a:=5; else y:= 5 ; fi  end");
    assert_int_equal(0, isValid);
}

void test_isinvalid_whilebodynostatements(){
    int isValid = 0;
    isValid = wizparse_str("proc main() while  true do  od  end");
    assert_true( isValid != 0);
}

void test_isinvalid_whilenocond(){
    int isValid = 0;
    isValid = wizparse_str("proc main() while   do j:= 10;  od  end");
    assert_true( isValid != 0);
}
void  test_suite_stmts( void )
{
	test_fixture_start();               // starts a fixture
	run_test(test_isvalid_assignment);
    run_test(test_isvalid_ifthen);
    run_test(test_isvalid_ifthenelse);
    run_test(test_isvalid_while);
    run_test(test_isinvalid_whilebodynostatements);
    run_test(test_isinvalid_whilenocond);
	test_fixture_end();                 // ends a fixture
}
