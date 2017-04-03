#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

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
	TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
	TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}

static void test_parse() {
	//test_parse_null();
	//test_parse_true();
	//test_parse_false();
	//test_parse_number();
	//test_parse_expect_value();
	//test_parse_invalid_value();
	//test_parse_root_not_singular();
	//test_parse_number_too_big();
	test_parse_string();
	/* ... */
}



int main() {
	test_parse();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
	//std::string s("Hello\nWorld");
	//std::cout << s << std::endl;
}