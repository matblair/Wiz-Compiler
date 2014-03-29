#include "seatest.h"
#include "wizparser.h"

void test_isvalid_intdecl(){
    int isValid = 0;
    isValid = wizparse_str("proc main() int a; a:=5;  end");
    assert_int_equal(0, isValid);
}


void test_isvalid_floatdecl(){
    int isValid = 0;
    isValid = wizparse_str("proc main() float a; a:=5;  end");
    assert_int_equal(0, isValid);
}

void test_isvalid_declbool(){
    int isValid = 0;
    isValid = wizparse_str("proc main() bool a; a:=5;  end");
    assert_int_equal(0, isValid);
}

void test_isvalid_declarray(){
    int isValid = 0;
    isValid = wizparse_str("proc main() bool ab[ 1..2 ];a:=5; end");
    assert_int_equal(0, isValid);
}

void test_isvalid_declarraymutidim(){
    int isValid = 0;
    isValid = wizparse_str("proc main() bool ab[ 1..2 , 2..10];a:=5; end");
    assert_int_equal(0, isValid);
}

void test_isinvalid_declarraydimendwithcomma(){
    int isValid = 0;
    isValid = wizparse_str("proc main() bool ab[ 1..2, ];a:=5; end");
    assert_true( isValid != 0);
}

void test_isinvalid_declnoarryidentifier(){
    int isValid = 0;
    isValid = wizparse_str("proc main() bool [ 1..2, ];a:=5; end");
    assert_true( isValid != 0);
}

void  test_suite_declarations( void )
{
	test_fixture_start();               // starts a fixture
    test_isvalid_declarray();
	run_test(test_isvalid_intdecl);
	run_test(test_isvalid_declbool);
	run_test(test_isvalid_floatdecl);
    run_test(test_isvalid_declarraymutidim);
	run_test(test_isinvalid_declarraydimendwithcomma);
    run_test(test_isinvalid_declnoarryidentifier);
    test_fixture_end();                 // ends a fixture
}
