#include "seatest.h"
#include "wizparser.h"

int map_to_zero_or_one(int i){
    if (i != 0) return 1;
    return i;
}
void test_is_valid_input(){
    int isValid = 0;
    isValid = wizparse_str("proc main() a:=5; end");
    assert_int_equal(0, isValid);
}

void test_stmt_missing_semicolon_invalid(){
    int is_valid;
    is_valid = map_to_zero_or_one( wizparse_str("proc main() a:=5 end"));
    assert_int_equal(1, is_valid);
    
}
/* 
void test_strings_equal()
{
	char *s = "hello";
	assert_string_equal("hello", s);
}

void test_arrays_equal()
{
	unsigned char expected_bytes[] = { 1, 2, 3};
	unsigned char buffer[5];
	int i;

	// put 5 bytes in
	for(i=0; i<5; i++) buffer[i]=i+1;

	// only check the first 3
	assert_n_array_equal(expected_bytes, buffer, 3);
}

void test_bits()
{
	assert_bit_set(0, 0x01);
	assert_bit_set(2, 0x04);
	assert_bit_not_set(3, 0x02);	
}

void test_strings()
{
	char *s = "hello";
	assert_string_equal("hello", s);
	assert_string_contains("blah", "why say blah?");
	assert_string_doesnt_contain("blah", "why say hello?");
	assert_string_ends_with("h?", "why say blah?");
	assert_string_starts_with("why", "why say blah?");
}

*/

void test_fixture_one( void )
{
	test_fixture_start();               // starts a fixture
	run_test(test_is_valid_input);
	run_test(test_stmt_missing_semicolon_invalid);
	test_fixture_end();                 // ends a fixture
}
