#ifndef LEPTJSON_H_
#define LEPTJSON_H_

#include <string>
#include <stack>

namespace leptjson {
	typedef enum {
		LEPT_NULL,
		LEPT_FALSE,
		LEPT_TRUE,
		LEPT_NUMBER,
		LEPT_STRING,
		LEPT_ARRAY,
		LEPT_OBJECT
	} lept_type;

	class LeptValue {
	public:
		~LeptValue() { if (type == LEPT_STRING) str.~str(); }
		LeptValue(lept_type _type) :type(_type),number(0) {}
		lept_type lept_get_type();
		double lept_get_number();
		union {
			double number;
			std::string str;
		};

		LeptValue& operator=(double d) {
			if (type == LEPT_STRING) str.~str();
			type = LEPT_NUMBER;
			number = d;
			return *this;
		}

		LeptValue& operator=(std::string s) {
			if (type == LEPT_STRING)
				str = s;
			else 
				new (&str) std::string(s);
			type = LEPT_STRING;
			return *this;
		}

		lept_type type;
	};

	class LeptJson {
	public:
		LeptJson(const std::string& _json) :context(_json) ,pos(context.begin()){}
		int remain_length() {
			return context.end() - pos;
		}
		std::string context;
		std::string::iterator pos;

		std::stack<LeptValue> value_stack;
		std::stack<char> char_stack;

	};

	class LeptJsonParser{
	public:
		typedef enum {
			LEPT_PARSE_OK = 0,
			LEPT_PARSE_EXPECT_VALUE,
			LEPT_PARSE_INVALID_VALUE,
			LEPT_PARSE_ROOT_NOT_SINGULAR,
			LEPT_PARSE_NUMBER_TOO_BIG
		} parse_status;

		LeptJsonParser(const std::string&);
		static parse_status lept_parse(const std::string & json,LeptValue* v);
	private:
		const std::string json;
		std::string::const_iterator key;
		static void lept_parse_whitespace(LeptJson &json);
		static parse_status lept_parse_value(LeptJson &json,LeptValue *);
		//static parse_status lept_parse_null(LeptJson &json,LeptValue *);
		//static parse_status lept_parse_false(LeptJson &json,LeptValue *);
		//static parse_status lept_parse_true(LeptJson &json,LeptValue *);
		static parse_status lept_parse_number(LeptJson &json, LeptValue *);
		static parse_status lept_parse_literal(LeptJson &json, LeptValue *v, const std::string& literal, lept_type type);
		static parse_status lept_parse_string(LeptJson &json, LeptValue *v);
	};



}










#endif
