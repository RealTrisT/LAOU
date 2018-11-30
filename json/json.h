#include <vector>
#include <string>
//TODO: JSON should prolly be a namespace
struct JSON{
	JSON(const char* s);

	enum JSONTYPE{
		JSONTYPE_OBJECT,
		JSONTYPE_ARRAY,
		JSONTYPE_NULL,
		JSONTYPE_BOOLEAN,
		JSONTYPE_STRING,
		JSONTYPE_NUMBER,
	};

	struct Element{
		Element(JSONTYPE jt) : type(jt){}
		JSONTYPE type;
		virtual bool parse(const char*, const char**) = 0;
		virtual void destroy() = 0;
	};

	struct Object : public Element{
		Object() : Element(JSONTYPE_OBJECT){}
		Element& operator[](const char*);


		bool parse(const char*, const char**);
		void destroy();

		std::vector<char*> 			fieldNames;
		std::vector<Element*>	fields;

		unsigned fieldAmount;
	};

	struct Array : public Element{
		Array() : Element(JSONTYPE_ARRAY){}
		Element& operator[](unsigned);

		bool parse(const char*, const char**);
		void destroy();	

		std::vector<Element*> elements;
		unsigned elementAmount;
	};

	struct Null : public Element{
		Null() : Element(JSONTYPE_NULL){}

		bool parse(const char* s, const char** e);
		void destroy();	
	};

	struct NumberValue : public Element{
		enum NUMTYPE{
			NUMTYPE_INTEGER,
			NUMTYPE_FLOATINGPOINT,
		};

		NumberValue() : Element(JSONTYPE_NUMBER){}

		bool parse(const char* s, const char** e);
		void destroy();

		NUMTYPE type;
		void* data;
	};

	struct BooleanValue : public Element{
		BooleanValue() : Element(JSONTYPE_BOOLEAN){}

		bool parse(const char* s, const char** e);
		void destroy();

		bool data;		
	};

	struct StringValue : public Element{
		StringValue() : Element(JSONTYPE_STRING){}

		bool parse(const char* s, const char** e);
		void destroy();

		const char* string;
	};


	static Element* parse(const char* s);
	static void destroy(Element*);
};