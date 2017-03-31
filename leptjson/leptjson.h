#ifndef LEPTJSON_H_
#define LEPTJSON_H_

#include <string>

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
		LeptValue(lept_type _type) :type(_type) {}
		lept_type lept_get_type();
		double letp_get_num();
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

	};

	class LeptJsonParser{
	public:
		typedef enum {
			LEPT_PARSE_OK = 0,
			LEPT_PARSE_EXPECT_VALUE,
			LEPT_PARSE_INVALID_VALUE,
			LEPT_PARSE_ROOT_NOT_SINGULAR
		} parse_status;

		LeptJsonParser(const std::string&);
		parse_status lept_parse(LeptValue* v);
		static parse_status lept_parse(const std::string & json,LeptValue* v);
	private:
		const std::string json;
		std::string::const_iterator key;
		static void lept_parse_whitespace(LeptJson &json);
		static parse_status lept_parse_value(LeptJson &json,LeptValue *);
		static parse_status lept_parse_null(LeptJson &json,LeptValue *);
		static parse_status lept_parse_false(LeptJson &json,LeptValue *);
		static parse_status lept_parse_true(LeptJson &json,LeptValue *);
		static parse_status lept_parse_literal(LeptJson &json, LeptValue *v, const std::string& literal, lept_type type);
	};



}










#endif
