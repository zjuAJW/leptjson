#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%s")
#define EXPECT_EQ_SIZE_T(expect,actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%zu")
#define TEST_ERROR(error, json)\
	do{\
		LeptValue v(LEPT_FALSE);\
		EXPECT_EQ_INT(error,LeptJsonParser::lept_parse(json,&v));\
		EXPECT_EQ_INT(LEPT_NULL, v.type);\
	}while(0)

#define TEST_NUMBER(expect, json)\
	do{\
		LeptValue v(LEPT_FALSE);\
		EXPECT_EQ_INT(LeptJsonParser::LEPT_PARSE_OK,LeptJsonParser::lept_parse(json,&v));\
		EXPECT_EQ_INT(LEPT_NUMBER,v.type);\
		EXPECT_EQ_DOUBLE(expect, v.number);\
	}while(0)

#define TEST_STRING(expect, json)\
	do{\
		LeptValue v(LEPT_FALSE);\
		EXPECT_EQ_INT(LeptJsonParser::LEPT_PARSE_OK,LeptJsonParser::lept_parse(json,&v));\
		EXPECT_EQ_INT(LEPT_STRING,v.type);\
		EXPECT_EQ_STRING(expect, v.str);\
		v.~v();\
	}while(0)

using namespace leptjson;

static void test_parse_null() {
	LeptValue v(LEPT_TRUE);
	EXPECT_EQ_INT(LeptJsonParser::LEPT_PARSE_OK, LeptJsonParser::lept_parse("null",&v));
	EXPECT_EQ_INT(LEPT_NULL, v.type);
}

static void test_parse_false() {
	LeptValue v(LEPT_TRUE);
	EXPECT_EQ_INT(LeptJsonParser::LEPT_PARSE_OK, LeptJsonParser::lept_parse("false", &v));
	EXPECT_EQ_INT(LEPT_FALSE, v.type);
}

static void test_parse_true() {
	LeptValue v(LEPT_FALSE);
	EXPECT_EQ_INT(LeptJsonParser::LEPT_PARSE_OK, LeptJsonParser::lept_parse("true", &v));
	EXPECT_EQ_INT(LEPT_TRUE, v.type);
}

static void test_parse_number() {
	TEST_NUMBER(0.0, "0");
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(-1.0, "-1");
	TEST_NUMBER(1.5, "1.5");
	TEST_NUMBER(-1.5, "-1.5");
	TEST_NUMBER(3.1416, "3.1416");
	TEST_NUMBER(1E10, "1E10");
	TEST_NUMBER(1e10, "1e10");
	TEST_NUMBER(1E+10, "1E+10");
	TEST_NUMBER(1E-10, "1E-10");
	TEST_NUMBER(-1E10, "-1E10");
	TEST_NUMBER(-1e10, "-1e10");
	TEST_NUMBER(-1E+10, "-1E+10");
	TEST_NUMBER(-1E-10, "-1E-10");
	TEST_NUMBER(1.234E+10, "1.234E+10");
	TEST_NUMBER(1.234E-10, "1.234E-10");
	TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

	/* the smallest number > 1 */
	TEST_NUMBER(1.0000000000000002, "1.0000000000000002");
	/* minimum denormal */
	TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324");
	TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	/* Max subnormal double */
	TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");
	TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	/* Min normal positive double */
	TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");
	TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	/* Max double */
	TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");
	TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse_root_not_singular() {
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_ROOT_NOT_SINGULAR, "null x");

#if 1
	/* invalid number */
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_ROOT_NOT_SINGULAR, "0x0");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_ROOT_NOT_SINGULAR, "0x123");
#endif
}

static void test_parse_expect_value() {
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_EXPECT_VALUE, "");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_VALUE, "nul");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_VALUE, "?");
#if 1
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_VALUE, "+0");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_VALUE, "+1");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_VALUE, "INF");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_VALUE, "inf");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_VALUE, "NAN");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_VALUE, "nan");
#endif
}

/* ... */

static void test_parse_number_too_big() {
#if 0
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309");
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309");
#endif
}

static void test_parse_string() {
	//TEST_STRING("", "\"\"");
	//TEST_STRING("Hello", "\"Hello\"");
	//TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
	//TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
	TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
	TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
	TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

static void test_parse_missing_quotation_mark() {
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_MISS_QUOTATION_MARK, "\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_invalid_unicode_hex() {
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse_array() {
	size_t i, j;
	//LeptValue v(LEPT_NULL);

	//EXPECT_EQ_INT(LeptJsonParser::LEPT_PARSE_OK, LeptJsonParser::lept_parse("[ ]",&v));
	//EXPECT_EQ_INT(LEPT_ARRAY, v.lept_get_type());
	//EXPECT_EQ_SIZE_T(0, v.array.size());
	
	LeptValue v2(LEPT_NULL);
	EXPECT_EQ_INT(LeptJsonParser::LEPT_PARSE_OK, LeptJsonParser::lept_parse("[ null , false , true , 123 , \"abc\" ]",&v2));
	EXPECT_EQ_INT(LEPT_ARRAY, v2.lept_get_type());
	EXPECT_EQ_SIZE_T(5, v2.array.size());
	EXPECT_EQ_INT(LEPT_NULL, v2.lept_get_array_element(0)->lept_get_type());
	EXPECT_EQ_INT(LEPT_FALSE, v2.lept_get_array_element(1)->lept_get_type());
	EXPECT_EQ_INT(LEPT_TRUE, v2.lept_get_array_element(2)->lept_get_type());
	EXPECT_EQ_INT(LEPT_NUMBER, v2.lept_get_array_element(3)->lept_get_type());
	EXPECT_EQ_INT(LEPT_STRING, v2.lept_get_array_element(4)->lept_get_type());
	EXPECT_EQ_DOUBLE(123.0, v2.lept_get_array_element(3)->number);
	EXPECT_EQ_STRING("abc", v2.lept_get_array_element(4)->str);
	
	LeptValue v3(LEPT_NULL);
	EXPECT_EQ_INT(LeptJsonParser::LEPT_PARSE_OK, LeptJsonParser::lept_parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]",&v3));
	EXPECT_EQ_INT(LEPT_ARRAY, v3.lept_get_type());
	EXPECT_EQ_SIZE_T(4, v3.array.size());
	for (i = 0; i < 4; i++) {
		LeptValue* a = v3.lept_get_array_element(i);
		EXPECT_EQ_INT(LEPT_ARRAY, a->lept_get_type());
		EXPECT_EQ_SIZE_T(i, a->array.size());
		for (j = 0; j < i; j++) {
			LeptValue* e = a->lept_get_array_element(j);
			EXPECT_EQ_INT(LEPT_NUMBER, e->lept_get_type());
			EXPECT_EQ_DOUBLE((double)j, e->number);
		}
	}
}

static void test_parse() {
	/*test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_number();
	test_parse_string();
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big();
	test_parse_missing_quotation_mark();
	test_parse_invalid_string_escape();
	test_parse_invalid_string_char();
	test_parse_invalid_unicode_hex();
	test_parse_invalid_unicode_surrogate();*/
	test_parse_array();
	/* ... */
}




int main() {
	test_parse();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
#ifdef _WINDOWS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	return main_ret;
	//unsigned u = 0x24;
	//std::string s1 = "";
	//s1.insert(s1.size(), 1, u);
	//std::string s2(s1.begin(),s1.begin()+1);
	//std::cout << s1<< std::endl;
}