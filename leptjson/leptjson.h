#ifndef LEPTJSON_H_
#define LEPTJSON_H_

#include <string>
#include <stack>
#include <vector>


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
	class LeptMember;

	class LeptValue {
	public:
		~LeptValue() { if (type == LEPT_STRING) str.~str(); }

		LeptValue(lept_type _type) :type(_type),number(0){}

		LeptValue(const LeptValue & rhs) : type(rhs.type) {
			switch (rhs.type) {
			case LEPT_NUMBER:
				number = rhs.number;
				break;
			case LEPT_STRING:
				new(&str) std::string(rhs.str);
				break;
			case LEPT_ARRAY:
				new(&arr) std::vector<LeptValue>(rhs.arr);
				break;
			case LEPT_OBJECT:
				new(&obj) std::vector<LeptMember>(rhs.obj);
				break;
			}
		}

		//TODO: 拷贝赋值运算符是有问题的，虽然在程序里没有用到
		LeptValue& operator=(const LeptValue & rhs){
			switch (rhs.type) {
			case LEPT_NUMBER:
				number = rhs.number;
				break;
			case LEPT_STRING:
				str = rhs.str;
				break;
			case LEPT_ARRAY:
				arr = rhs.arr;
				break;
			}
			return *this;
		}

		lept_type lept_get_type();
		double lept_get_number();
		LeptValue* lept_get_array_element(size_t n);
		std::string lept_get_object_key(size_t n);
		LeptValue * lept_get_object_value(size_t n);
		union {
			double number;
			std::string str;
			std::vector<LeptValue> arr;
			std::vector<LeptMember> obj;
		};

		LeptValue& operator=(double d) {
			if (type == LEPT_STRING) str.~str();
			type = LEPT_NUMBER;
			number = d;
			return *this;
		}

		LeptValue& operator=(const std::string& s) {
			if (type == LEPT_STRING)
				str = s;
			else {
				if (type == LEPT_ARRAY)
					arr.~arr();
				if (type == LEPT_OBJECT)
					obj.~obj();
				new (&str) std::string(s);
			}
			type = LEPT_STRING;
			return *this;
		}

		LeptValue& operator=(std::vector<LeptValue>& _arr) {
			if (type == LEPT_ARRAY)
				arr = _arr;
			else {
				if (type == LEPT_STRING)
					str.~str();
				if (type == LEPT_OBJECT)
					obj.~obj();
				new (&arr) std::vector<LeptValue>(_arr);
			}
			type = LEPT_ARRAY;
			return *this;
		}

		LeptValue& operator=(std::vector<LeptMember>& _obj) {
			if (type == LEPT_OBJECT)
				obj = _obj;
			else {
				if (type == LEPT_STRING)
					str.~str();
				if (type == LEPT_ARRAY)
					arr.~arr();
				new (&obj) std::vector<LeptMember>(_obj);
			}
			type = LEPT_OBJECT;
			return *this;
		}

		lept_type type;
	};

	class LeptMember {
	public:
		LeptMember(const std::string &_key, const LeptValue &_value) :key(_key), value(_value) {}
		std::string key;
		LeptValue value;
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

	};

	class LeptJsonParser{
	public:
		typedef enum {
			LEPT_PARSE_OK = 0,
			LEPT_PARSE_EXPECT_VALUE,
			LEPT_PARSE_INVALID_VALUE,
			LEPT_PARSE_ROOT_NOT_SINGULAR,
			LEPT_PARSE_NUMBER_TOO_BIG,
			LEPT_PARSE_INVALID_STRING_ESCAPE,
			LEPT_PARSE_MISS_QUOTATION_MARK,
			LEPT_PARSE_INVALID_STRING_CHAR,
			LEPT_PARSE_INVALID_UNICODE_HEX,
			LEPT_PARSE_INVALID_UNICODE_SURROGATE,
			LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
			LEPT_PARSE_MISS_KEY,
			LEPT_PARSE_MISS_COLON,
			LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET
		} parse_status;

		LeptJsonParser(const std::string&);
		static parse_status lept_parse(const std::string & json,LeptValue* v);
	private:
		const std::string json;
		std::string::const_iterator key;

		static int lept_parse_hex(LeptJson &json,unsigned *u);
		static void lept_encode_utf8(std::string& str, unsigned u);
		static void lept_parse_whitespace(LeptJson &json);
		static parse_status lept_parse_value(LeptJson &json,LeptValue *);
		//static parse_status lept_parse_null(LeptJson &json,LeptValue *);
		//static parse_status lept_parse_false(LeptJson &json,LeptValue *);
		//static parse_status lept_parse_true(LeptJson &json,LeptValue *);
		static parse_status lept_parse_number(LeptJson &json, LeptValue *);
		static parse_status lept_parse_literal(LeptJson &json, LeptValue *v, const std::string& literal, lept_type type);
		static parse_status lept_parse_string_raw(LeptJson &json, std::string &str);
		static parse_status lept_parse_string(LeptJson &json, LeptValue *v);
		static parse_status lept_parse_array(LeptJson &json, LeptValue *v);
		static parse_status lept_parse_object(LeptJson &json, LeptValue *v);

	};

	class LeptJsonStringifier {
	public:
		typedef enum {
			LEPT_STRINGIFY_OK = 0,
			LEPT_STRINGIFY_INVALID_TYPE
		} stringify_status;

		static stringify_status lept_stringify(LeptValue *v,std::string& json);
	private:
		static stringify_status lept_stringify_value(LeptValue  *v,std::string & json);
	};



}

#endif