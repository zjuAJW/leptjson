// LeptJsonParser.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "leptjson.h"
#include <iostream>
#include <cerrno>

#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

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

	LeptValue * LeptValue::lept_get_array_element(size_t n) {
		assert(type == LEPT_ARRAY);
		assert(0 < n < arr.size());
		return &arr[n];
	}

	std::string LeptValue::lept_get_object_key(size_t n) {
		assert(type == LEPT_OBJECT);
		assert(n < obj.size());
		return obj[n].key;
	}

	LeptValue * LeptValue::lept_get_object_value(size_t n) {
		assert(type == LEPT_OBJECT);
		assert(n < obj.size());
		return &obj[n].value;
	}

	LeptJsonParser::LeptJsonParser(const string& _json) :json(_json), key(json.begin()) {}

	LeptJsonParser::parse_status LeptJsonParser::lept_parse(const string& _json, LeptValue* v) {
		LeptJson json(_json);
		assert(v != NULL);
		v->type = LEPT_NULL;
		LeptJsonParser::lept_parse_whitespace(json);
		if (json.remain_length() <= 0)
			return LEPT_PARSE_EXPECT_VALUE;
		parse_status ret;
		if ((ret = lept_parse_value(json, v)) == LEPT_PARSE_OK) {
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
		case '[':
			return lept_parse_array(json, v);
		case '{':
			return lept_parse_object(json, v);
		default:
			return lept_parse_number(json, v);
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
		auto start = json.pos;
		if (*json.pos == '-') { ++json.pos; }
		if (json.remain_length() > 0 && *json.pos == '0') { ++json.pos; }
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
		double result = strtod(string(start, json.pos).c_str(), NULL);
		if (errno == ERANGE)
			return LEPT_PARSE_NUMBER_TOO_BIG;
		*v = result;
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
				if (*(json.pos + i) != literal[i + 1]) {
					return LEPT_PARSE_INVALID_VALUE;
				}
			}
		}
		json.pos += (literal.size() - 1);
		v->type = type;
		return LEPT_PARSE_OK;
	}

	LeptJsonParser::parse_status LeptJsonParser::lept_parse_string_raw(LeptJson &json, string &_str) {
		EXPECT(json.pos, '\"');
		unsigned u, u2;
		while (1) {
			if (json.remain_length() <= 0) {
				return LEPT_PARSE_MISS_QUOTATION_MARK;
			}
			switch (*json.pos) {
			case '\"':
				++json.pos;
				return LEPT_PARSE_OK;
			case '\\':
				if (json.remain_length() > 0) ++json.pos;
				else { return LEPT_PARSE_MISS_QUOTATION_MARK; }
				switch (*json.pos) {
				case '\"': _str += "\""; ++json.pos; break;
				case '\\': _str += "\\"; ++json.pos; break;
				case '/':  _str += "/"; ++json.pos; break;
				case 'b':  _str += "\b"; ++json.pos; break;
				case 'f':  _str += "\f"; ++json.pos; break;
				case 'n':  _str += "\n"; ++json.pos; break;
				case 'r':  _str += "\r"; ++json.pos; break;
				case 't':  _str += "\t"; ++json.pos; break;
				case 'u':
					++json.pos;
					if (!lept_parse_hex(json, &u))
						return LEPT_PARSE_INVALID_UNICODE_HEX;
					json.pos += 4;
					if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
						if (*json.pos++ != '\\')
							return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
						if (*json.pos++ != 'u')
							return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
						if (!lept_parse_hex(json, &u2))
							return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
						if (u2 < 0xDC00 || u2 > 0xDFFF)
							return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
						u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
						json.pos += 4;
					}
					lept_encode_utf8(_str, u);
					break;
				default:
					return LEPT_PARSE_INVALID_STRING_ESCAPE;
				}
				break;
			default:
				if ((unsigned char)*json.pos < 0x20) {
					return LEPT_PARSE_INVALID_STRING_CHAR;
				}
				_str += *json.pos;
				++json.pos;
			}
		}
	}

	LeptJsonParser::parse_status LeptJsonParser::lept_parse_string(LeptJson &json, LeptValue *v) {
		string _str("");
		parse_status ret;
		if ((ret = lept_parse_string_raw(json, _str)) == LEPT_PARSE_OK) {
			*v = _str;
		}
		return ret;
	}

	int LeptJsonParser::lept_parse_hex(LeptJson &json, unsigned *u) {
		char ch;
		*u = 0;
		for (int i = 0; i < 4; i++) {
			*u <<= 4;
			ch = *(json.pos + i);
			if (ch >= '0' && ch <= '9')  *u |= ch - '0';
			else if (ch >= 'A' && ch <= 'F')  *u |= ch - ('A' - 10);
			else if (ch >= 'a' && ch <= 'f')  *u |= ch - ('a' - 10);
			else return 0;
		}
		return 1;
	}

	void LeptJsonParser::lept_encode_utf8(string &str, unsigned u) {
		if (u <= 0x7F)
			str.push_back(u);
		else if (u <= 0x7FF) {
			str.push_back(0xC0 | ((u >> 6) & 0xFF));
			str.push_back(0x80 | (u & 0x3F));
		}
		else if (u <= 0xFFFF) {
			str.push_back(0xE0 | ((u >> 12) & 0xFF));
			str.push_back(0x80 | ((u >> 6) & 0x3F));
			str.push_back(0x80 | (u & 0x3F));
		}
		else {
			assert(u <= 0x10FFFF);
			str.push_back(0xF0 | ((u >> 18) & 0xFF));
			str.push_back(0x80 | ((u >> 12) & 0x3F));
			str.push_back(0x80 | ((u >> 6) & 0x3F));
			str.push_back(0x80 | (u & 0x3F));
		}
	}

	LeptJsonParser::parse_status LeptJsonParser::lept_parse_array(LeptJson &json, LeptValue *v) {
		EXPECT(json.pos, '[');
		lept_parse_whitespace(json);
		parse_status ret;
		v->type = LEPT_ARRAY;
		new(&v->arr) std::vector<LeptValue>;
		if (*json.pos == ']') {
			++json.pos;
			return LEPT_PARSE_OK;
		}
		while (1) {
			lept_parse_whitespace(json);
			if (json.remain_length() <= 0)
				return LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
			auto element = new LeptValue(LEPT_NULL);
			if ((ret = lept_parse_value(json, element)) != LEPT_PARSE_OK)
				return ret;
			v->arr.push_back(*element);
			lept_parse_whitespace(json);
			if (*json.pos == ',') ++json.pos;
			else if (*json.pos == ']') {
				++json.pos;
				return LEPT_PARSE_OK;
			}
			else
				return LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
		}
	}

	LeptJsonParser::parse_status LeptJsonParser::lept_parse_object(LeptJson &json, LeptValue *v) {
		EXPECT(json.pos, '{');
		parse_status ret;
		std::vector<LeptMember> result;
		while (1) {
			string key("");
			LeptValue value(LEPT_NULL);
			lept_parse_whitespace(json);
			if (json.remain_length() <= 0) {
				return LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
			}
			if (*json.pos == '}') {
				*v = result;
				++json.pos;
				return LEPT_PARSE_OK;
			}
			if (*json.pos != '\"')
				return LEPT_PARSE_MISS_KEY;
			if ((ret = lept_parse_string_raw(json, key)) != LEPT_PARSE_OK)
				return ret;
			lept_parse_whitespace(json);
			if (json.remain_length() <= 0 || *json.pos != ':')
				return LEPT_PARSE_INVALID_VALUE;
			++json.pos;
			lept_parse_whitespace(json);
			if ((ret = lept_parse_value(json, &value)) != LEPT_PARSE_OK)
				return ret;
			LeptMember m(key, value);
			result.push_back(m);
			lept_parse_whitespace(json);
			if (json.remain_length() > 0 && *json.pos == ',') ++json.pos;
		}
	}

	LeptJsonStringifier::stringify_status LeptJsonStringifier::lept_stringify(LeptValue *v, std::string& json) {
		assert(v != NULL);
		json = "";
		stringify_status ret;
		ret = lept_stringify_value(v, json);
		return ret;
	}

	LeptJsonStringifier::stringify_status LeptJsonStringifier::lept_stringify_value(LeptValue  *v, std::string & json) {
		assert(v != NULL);
		stringify_status ret;
		switch (v->type) {
		case LEPT_NULL:
			json += "null"; break;
		case LEPT_FALSE:
			json += "false"; break;
		case LEPT_TRUE:
			json += "true"; break;
		case LEPT_NUMBER:
			char buffer[32];
			sprintf(buffer, "%.17g", v->number);
			json += string(buffer);
			break;
		case LEPT_STRING:
			LeptJsonStringifier::lept_stringify_string(v, json);
			break;
		case LEPT_ARRAY:
			json += "[";
			if (!empty(v->arr)) {
				for (auto element : v->arr) {
					if ((ret = lept_stringify_value(&element, json)) != LEPT_STRINGIFY_OK)
						return ret;
					json += ",";
				}
				json.pop_back();
			}
			json += "]";
			break;
		case LEPT_OBJECT:
			json += "{";
			if (!empty(v->obj)) {
				for (auto element : v->obj) {
					json += "\"";
					json += element.key;
					json += "\":";
					if ((ret = lept_stringify_value(&element.value, json)) != LEPT_STRINGIFY_OK)
						return ret;
					json += ",";
				}
				json.pop_back();
			}
			json += "}";
			break;
		default:
			return LEPT_STRINGIFY_INVALID_TYPE;
		}
		return LEPT_STRINGIFY_OK;
	}

	LeptJsonStringifier::stringify_status LeptJsonStringifier::lept_stringify_string(LeptValue  *v, std::string & json) {
		json += "\"";
		for (auto ch : v->str) {
			switch (ch) {
			case '\"': json += "\\\""; break;
			case '\\': json += "\\\\"; break;
			case '/':  json += "/"; break;
			case '\b':  json += "\\b"; break;
			case '\f':  json += "\\f"; break;
			case '\n':  json += "\\n"; break;
			case '\r':  json += "\\r"; break;
			case '\t':  json += "\\t"; break;
			default:
				if (ch < 0x20) {
					char buffer[7];
					sprintf(buffer, "\\u%04X", ch);
					json += buffer;
				}
				else
					json += ch;
			}
		}
		json += "\"";
		return LEPT_STRINGIFY_OK;
	}
}
