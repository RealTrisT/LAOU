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

	struct ParseError{
		const char* errorstr;
		const char* strerror;
	};

	struct Element{
		friend struct JSON;
		virtual Element& operator[](const char* n){	throw "Unimplemented for this type";}
		virtual Element& operator[](unsigned i){	throw "Unimplemented for this type";}
		virtual std::string getString(){			throw "Unimplemented for this type";}
		virtual double getDouble(){					throw "Unimplemented for this type";}
		virtual long long int getInt(){				throw "Unimplemented for this type";}
		virtual bool getBool(){						throw "Unimplemented for this type";}
		virtual bool isNull(){						return false;						}
		JSONTYPE type;
	private:
		Element(JSONTYPE jt) : type(jt){}
	protected:
		virtual bool parse(const char*, const char**, ParseError* = 0) = 0;
		virtual void destroy() = 0;
	};

	struct Object : public Element{
		Object() : Element(JSONTYPE_OBJECT){}

		Element& operator[](const char*);


		bool parse(const char*, const char**, ParseError* = 0);
		void destroy();

		std::vector<char*> 			fieldNames;
		std::vector<Element*>	fields;

		unsigned fieldAmount;
	};

	struct Array : public Element{
		Array() : Element(JSONTYPE_ARRAY){}
		
		Element& operator[](unsigned);

		bool parse(const char*, const char**, ParseError* = 0);
		void destroy();	

		std::vector<Element*> elements;
		unsigned elementAmount;
	};

	struct Null : public Element{
		Null() : Element(JSONTYPE_NULL){}

		bool parse(const char*, const char**, ParseError* = 0);
		void destroy();	

		bool isNull();
	};

	struct NumberValue : public Element{
		enum NUMTYPE{
			NUMTYPE_INTEGER,
			NUMTYPE_FLOATINGPOINT,
		};

		NumberValue() : Element(JSONTYPE_NUMBER){}

		bool parse(const char*, const char**, ParseError* = 0);
		void destroy();

		NUMTYPE type;
		void* data;

		long long int getInt();
		double getDouble();
	};

	struct BooleanValue : public Element{
		BooleanValue() : Element(JSONTYPE_BOOLEAN){}

		bool parse(const char*, const char**, ParseError* = 0);
		void destroy();

		bool getBool();

		bool data;		
	};

	struct StringValue : public Element{
		StringValue() : Element(JSONTYPE_STRING){}

		bool parse(const char*, const char**, ParseError* = 0);
		void destroy();

		std::string getString();

		const char* istring;
	};


	static Element* parse(const char* s, ParseError* = 0);
	static void destroy(Element*);
};
