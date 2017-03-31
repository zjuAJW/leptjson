#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define TEST_ERROR(expect, json)\
	do{\
		LeptValue v(LEPT_FALSE);\
		EXPECT_EQ_INT(expect,LeptJsonParser::lept_parse(json,&v));\
		EXPECT_EQ_INT(LEPT_NULL, v.type);\
	}while(0)

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

static void test_parse_root_not_singular() {
	LeptValue v(LEPT_FALSE);
	v.type = LEPT_FALSE;
	EXPECT_EQ_INT(LeptJsonParser::LEPT_PARSE_OK, LeptJsonParser::lept_parse("null x", &v));
	EXPECT_EQ_INT(LEPT_NULL, v.type);
}

static void test_parse_expect_value() {
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_EXPECT_VALUE, "");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_VALUE, "nul");
	TEST_ERROR(LeptJsonParser::LEPT_PARSE_INVALID_VALUE, "?");
}

/* ... */

static void test_parse() {
	test_parse_null();
	test_parse_false();
	test_parse_true();
	test_parse_invalid_value();
    test_parse_expect_value();
	/* ... */
}

int main() {
	test_parse();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
}