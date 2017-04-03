// LeptJsonParser.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "leptjson.h"
#include <assert.h>
#include <iostream>

inline void EXPECT(std::string::iterator& key, char ch) {assert(*key == (ch)); key++;}
inline int is_digit(char ch) { return ch >= '0' && ch <= '9'; }
inline int is_digit19(char ch) { return ch >= '1' && ch <= '9'; }

using std::string;
using std::cout;
using std::endl;

namespace leptjson {

	double LeptValue::lept_get_number() {
		assert(type == LEPT_NUMBER);
		return number;
	}
	lept_type LeptValue::lept_get_type() {
		return type;
	}
	LeptJsonParser::LeptJsonParser(const string& _json) :json(_json),key(json.begin()) {}

	LeptJsonParser::parse_status LeptJsonParser::lept_parse(const string& _json, LeptValue* v) {
		LeptJson json(_json);
		assert(v != NULL);
		v->type = LEPT_NULL;
		LeptJsonParser::lept_parse_whitespace(json);
		if (json.remain_length() <= 0)
			return LEPT_PARSE_EXPECT_VALUE;
		parse_status ret;
		if ((ret = lept_parse_value(json,v)) == LEPT_PARSE_OK) {
			LeptJsonParser::lept_parse_whitespace(json);
			if (json.remain_length() > 0) {
				v->type = LEPT_NULL;
				ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
			}
		}
		return ret;

	}


	void LeptJsonParser::lept_parse_whitespace(LeptJson &json) {
		if (json.pos == json.context.end())
			return;
		if (*json.pos == ' ' || *json.pos == '\t' || *json.pos == '\n' || *json.pos == '\r') {
			++json.pos;
		}
	}

	LeptJsonParser::parse_status LeptJsonParser::lept_parse_value(LeptJson &json, LeptValue *v) {
		switch (*json.pos) {
		case 'n':
			return lept_parse_literal(json, v, "null", LEPT_NULL);
		case 't':
			return lept_parse_literal(json, v, "true", LEPT_TRUE);
		case '\"':
			return lept_parse_string(json, v);
		case 'f':
			return lept_parse_literal(json, v, "false", LEPT_FALSE);
		default:
			return lept_parse_number(json,v);
		}
	}

	/*(LeptJsonParser::parse_status LeptJsonParser::lept_parse_null(LeptJson &json,LeptValue * v) {
		EXPECT(json.pos, 'n'); 
		if (json.remain_length() < 3 || *(json.pos) != 'u' || *(json.pos + 1) != 'l' || *(json.pos + 2) != 'l')
			return LEPT_PARSE_INVALID_VALUE;
		json.pos += 3;
		v->type = LEPT_NULL;
		return LEPT_PARSE_OK;
	}

	LeptJsonParser::parse_status LeptJsonParser::lept_parse_false(LeptJson &json,LeptValue * v) {
		EXPECT(json.pos, 'f');
		if (json.remain_length() < 4 || *(json.pos) != 'a' || *(json.pos + 1) != 'l' || *(json.pos + 2) != 's' || *(json.pos + 3) != 'e')
			return LEPT_PARSE_INVALID_VALUE;
		json.pos += 4;
		v->type = LEPT_FALSE;
		return LEPT_PARSE_OK;
	}

	LeptJsonParser::parse_status LeptJsonParser::lept_parse_true(LeptJson &json,LeptValue * v) {
		EXPECT(json.pos, 't');
		if (json.remain_length() < 3 || *(json.pos) != 'r' || *(json.pos + 1) != 'u' || *(json.pos + 2) != 'e')
			return LEPT_PARSE_INVALID_VALUE;
		json.pos += 3;
		v->type = LEPT_TRUE;
		return LEPT_PARSE_OK;
	}*/

	LeptJsonParser::parse_status LeptJsonParser::lept_parse_number(LeptJson &json, LeptValue * v) {
			if (*json.pos == '-') { ++json.pos;}
			if (json.remain_length() > 0 && *json.pos == '0') { ++json.pos;}
			else {
				if (json.remain_length() <= 0 || !is_digit19(*json.pos)) return LEPT_PARSE_INVALID_VALUE;
				while (json.remain_length() > 0 && is_digit(*json.pos)) ++json.pos;

			}
			if (json.remain_length() > 0 && *json.pos == '.') {
				++json.pos;
				if (json.remain_length() <= 0 || !is_digit(*json.pos)) return LEPT_PARSE_INVALID_VALUE;
				while (json.remain_length() > 0 && isdigit(*json.pos)) ++json.pos;

			}
			if (json.remain_length() > 0 && (*json.pos == 'e' || *json.pos == 'E')) {
				++json.pos;
				if (json.remain_length() <= 0) return LEPT_PARSE_INVALID_VALUE;
				if (*json.pos == '-' || *json.pos == '+') ++json.pos;
				if (json.remain_length() <= 0 || !is_digit(*json.pos)) return LEPT_PARSE_INVALID_VALUE;
				while (json.remain_length() > 0 && is_digit(*json.pos)) ++json.pos;
			}

		errno = 0;
		v->number = strtod(json.context.c_str(), NULL);
		if (errno == ERANGE && (v->number == HUGE_VAL || v->number == -HUGE_VAL))
			return LEPT_PARSE_NUMBER_TOO_BIG;
		v->type = LEPT_NUMBER;
		return LEPT_PARSE_OK;
	}

	LeptJsonParser::parse_status LeptJsonParser::lept_parse_literal(LeptJson &json, LeptValue *v, const string& literal, lept_type type) {
		EXPECT(json.pos, literal[0]);
		if (json.remain_length() < (literal.size() - 1)) {
			return LEPT_PARSE_INVALID_VALUE;
		}
		else {
			for (int i = 0; i < literal.size() - 1; ++i) {
				if (*(json.pos + i) != literal[i + 1]){
					return LEPT_PARSE_INVALID_VALUE;
				}
			}
		}
		json.pos += (literal.size() - 1);
		v->type = type;
		return LEPT_PARSE_OK;
	}

	LeptJsonParser::parse_status LeptJsonParser::lept_parse_string(LeptJson &json, LeptValue *v) {
		EXPECT(json.pos, '\"');
		auto start = json.pos;
		while (json.remain_length() > 0 && *json.pos!= '\"') {
			++json.pos;
		}
		if (json.remain_length() <= 0 && json.pos!=start)
			return LEPT_PARSE_INVALID_VALUE;
		else if(json.pos!=start){
			*v = string(start,json.pos);
			++json.pos;
			return LEPT_PARSE_OK;
		}
		else {
			*v = string("");
			++json.pos;
			return LEPT_PARSE_OK;
		}
	}
}
