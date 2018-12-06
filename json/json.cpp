#include "json.h"
#include "stdlib.h"
#include <string.h>

#include <stdio.h>

char getDelimiterEndChar(char delimiterBegin){
	if(delimiterBegin != '{' && delimiterBegin != '[')return -1;
	return delimiterBegin+2;
}

//returns pointer to the "closing" quotation marks
char* getQuotationsEnd(const char* s){
	if(*(s++) != '"')return 0;
	while(*s && !(*s == '"' && *(s-1) != '\\'))s++;
	return *s ? (char*)s : (char*)0;
}

//returns pointer to the closing delimiter
char* getDelimiterEnd(const char* s){
	char delimiterBegin = *s;
	char delimiterEnd = getDelimiterEndChar(delimiterBegin);
	unsigned delimiterScope = 1;

	if(delimiterEnd == -1)return 0;	//doesnt start on delimiter
	
	s++;
	do{ 
		if(*s == '"'){s = getQuotationsEnd(s)+1; if(s == (const char*)1)return 0;}
		if(*s == delimiterBegin)delimiterScope++;
		else 					delimiterScope--;			
	}while(delimiterScope && *(++s));
	if(!delimiterScope)return (char*)s;
	else return 0;
}

//returns pointer to after the last space character found
char* getAfterSpace(const char* s){
	while(*s && (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r'))s++;
	return (char*)s;
}

JSON::Element* detectTypeAndInstanciate(char firstLetter){
	if(firstLetter == '{'){			//object
		return new JSON::Object();
	}else if(firstLetter == '['){	//array
		return new JSON::Array();
	}else if(firstLetter == 'n'){	//null
		return new JSON::Null();
	}else if(firstLetter == 'f'){	//false
		return new JSON::BooleanValue();
	}else if(firstLetter == 't'){	//true
		return new JSON::BooleanValue();
	}else if(firstLetter == '"'){	//string
		return new JSON::StringValue();
	}else{ 							//number
		return new JSON::NumberValue();
	}
}

char* sizecopy(const char* str, unsigned len){
	char* res = new char[len+1];
	memcpy(res, str, len);
	res[len] = '\0';
	return res;
}

void sizedelete(const char* str){
	delete[] str;
}

#define parsefail(str, i) if(err){err->errorstr = str; err->strerror = i;}

//receives a pointer to the delimiting character
bool JSON::Object::parse(const char* s, const char** e, ParseError* err){	const char* lastSpos = s;
	s++;
	for (;;){
		/////////////////////////////////////////////////////////////////////
		///////////////////////////////START/////////////////////////////////
		/////////////////////////////////////////////////////////////////////
		s = getAfterSpace(s);											//remove any type of whitespace before our object actually starts

		if(!*s){
			parsefail("Json string ended assumingly permaturely.", lastSpos);
			goto destroyProgressAndFailObj;
		}else if(*s != '"'){
			parsefail("Bad name formatting, spec dictates use of quotes is required.", lastSpos);
			goto destroyProgressAndFailObj;
		}lastSpos = s;

		/////////////////////////////////////////////////////////////////////
		////////////////////////////MEMBER NAME//////////////////////////////
		/////////////////////////////////////////////////////////////////////

		char* strbegin = (char*)(s+1);									//beginning of the name string
		s = getQuotationsEnd(s);										//find end of name string

		if(!s){
			parsefail("No closing quotes for this field name.", lastSpos);
			goto destroyProgressAndFailObj;
		}else if(s == strbegin){
			parsefail("Empty field name.", lastSpos);
			goto destroyProgressAndFailObj;
		}lastSpos = s;

		unsigned nameLen = s - strbegin;								//name string length
		this->fieldNames.push_back(new char[nameLen+1]);				//allocation to new element of vector
		memcpy(this->fieldNames[this->fieldAmount], strbegin, nameLen);	//copy of the string
		this->fieldNames[this->fieldAmount][nameLen] = '\0';			//null-terminate

		/////////////////////////////////////////////////////////////////////
		/////////////////////////////INDICATOR///////////////////////////////
		/////////////////////////////////////////////////////////////////////

		s = getAfterSpace(s+1);
		if(!*s){
			parsefail("No closing quotes for this field name.", lastSpos+1);
			goto destroyProgressAndFailObj_n;
		}else if(*s != ':'){
			parsefail("Missing \":\" to indicate object corresponding to name.", lastSpos+1);
			goto destroyProgressAndFailObj_n;
		}lastSpos = s;

		s = getAfterSpace(s+1);
		if(!*s){
			parsefail("Json string ended assumingly permaturely.", lastSpos+1);
			goto destroyProgressAndFailObj_n;
		}lastSpos = s;

		/////////////////////////////////////////////////////////////////////
		//////////////////////////////MEMBER/////////////////////////////////
		/////////////////////////////////////////////////////////////////////

		this->fields.push_back(detectTypeAndInstanciate(*s));
		if(!this->fields[this->fieldAmount]->parse(s, &s, err)) goto destroyProgressAndFailObj_i; lastSpos = s;

		/////////////////////////////////////////////////////////////////////
		/////////////////////////////SEPARATOR///////////////////////////////
		/////////////////////////////////////////////////////////////////////

		s = getAfterSpace(s);

		if(*s == '}'){
			this->fieldAmount++; break;
		}else if(*s == ','){
			this->fieldAmount++; s++;
		}else if(!*s){
			parsefail("Json string ended assumingly permaturely.", lastSpos);
			goto destroyProgressAndFailObj_n;
		}else{
			parsefail("Bad or missing separator.", lastSpos);
			goto destroyProgressAndFailObj_i;
		}lastSpos = s;
	}

	*e = s+1;
	return true;


destroyProgressAndFailObj_i:
	delete this->fields[this->fieldAmount];			//destroy current instance
destroyProgressAndFailObj_n:
	delete[] this->fieldNames[this->fieldAmount];	//destroy current name
destroyProgressAndFailObj:
	this->destroy();								//destroy all the ones that were created in previous iterations of the loop
	return false;
}

void JSON::Object::destroy(){
	for (unsigned i = 0; i < this->fieldAmount; ++i){
		delete[] this->fieldNames[i];
		this->fields[i]->destroy(); 
		delete this->fields[i];
	}
	this->fieldNames.clear();
	this->fields.clear();
}

bool JSON::Array::parse(const char* s, const char** e, ParseError* err){	const char* lastSpos = s;
	s++;
	for(;;){
		s = getAfterSpace(s);
		if(!*s){parsefail("Json string ended assumingly permaturely.", lastSpos+1); goto destroyProgressAndFailArr;} lastSpos = s;

		this->elements.push_back(detectTypeAndInstanciate(*s));
		if(!this->elements[this->elementAmount]->parse(s, &s, err))goto destroyProgressAndFailArr; lastSpos = s;

		s = getAfterSpace(s);
		if(*s == ','){
			this->elementAmount++; s++;
		}else if(*s == ']'){
			this->elementAmount++; break;
		}else if(!*s){
			parsefail("Json string ended assumingly permaturely.", lastSpos);
			goto destroyProgressAndFailArr;
		}else{ 
			parsefail("Bad or missing separator.", lastSpos);
			goto destroyProgressAndFailArr; 
		}lastSpos = s;
	}

	*e = s+1;
	return true;

destroyProgressAndFailArr:
	delete this->elements[this->elementAmount];
	this->destroy();
	return false;
}

void JSON::Array::destroy(){
	for (unsigned i = 0; i < this->elementAmount; ++i){
		this->elements[i]->destroy();
		delete this->elements[i];
	}this->elements.clear();
}

bool JSON::Null::parse(const char* s, const char** e, ParseError* err){
	if(*(unsigned int*)s == *(unsigned int*)"null"){
		*e = s+4; 
		return true;
	}
	parsefail("This is not a valid value.", s);
	return false;
}

void JSON::Null::destroy(){
	return;
}

bool JSON::NumberValue::parse(const char* s, const char** e, ParseError* err){
	char dotCount = 0, eCount = 0;
	unsigned nrStringSize = 0; char* elstr = 0;
	const char* ts = s;

	for (;;++ts){
		if(*ts <= '9' && *ts >= '0'){
			continue;
		}else if(*ts == 'e' || *ts == 'E'){
			if(ts == s)goto destroyProgressAndFailNum;
			eCount++; continue;
		}else if(*ts == '.'){
			if(ts == s)goto destroyProgressAndFailNum; //NOTE: this might be allowed maybe? .25 seems like something that'd be readable
			dotCount++; continue;
		}else break;
	}

	if(ts == s || eCount > 1 || dotCount > 1)goto destroyProgressAndFailNum;

	nrStringSize = ts - s;
	elstr = new char[nrStringSize+1];
	memcpy(elstr, s, nrStringSize);
	elstr[nrStringSize] = '\0';

	if(!dotCount && !eCount){
		this->type = NUMTYPE_INTEGER;
		this->data = (void*)new long long int;
		*(long long int*)(this->data) = atoi(elstr);
	}else{
		this->type = NUMTYPE_FLOATINGPOINT;
		this->data = (void*)new double;
		*(double*)(this->data) = atof(elstr);
	}

	delete[] elstr;
	*e = s + nrStringSize;
	return true;
destroyProgressAndFailNum:
	parsefail("This is not a valid value.", s);
	return false;
}

void JSON::NumberValue::destroy(){
	if(this->type == NUMTYPE_FLOATINGPOINT) delete (double*)data;
	else 									delete (long long int*)data;	
}


bool JSON::BooleanValue::parse(const char* s, const char** e, ParseError* err){
	if(*(unsigned*)s == *(unsigned*)"true"){
		*e = s+4;
		this->data = true;
	}else if(*(unsigned*)s == *(unsigned*)"fals" && s[4] == 'e'){
		*e = s+5;
		this->data = false;
	}else {parsefail("This is not a valid value.", s); return false;}
	return true;
}

void JSON::BooleanValue::destroy(){
	return;
}

bool JSON::StringValue::parse(const char* s, const char** e, ParseError* err){
	char* end = (char*)getQuotationsEnd(s);
	if(!*end){
		parsefail("Json string ended assumingly permaturely.", s);
		return false;
	}
	s++;

	unsigned length = end-s;

	this->istring = new char[length+1];
	memcpy((void*)this->istring, s, length);
	((char*)this->istring)[length] = '\0';

	*e = s+length+1; //+1 for the '"'
	return true;
}

void JSON::StringValue::destroy(){
	delete[] this->istring;
}








std::string JSON::StringValue::getString(){
	return this->istring;
}

bool JSON::Null::isNull(){
	return true;
}

bool JSON::BooleanValue::getBool(){
	return this->data;
}

long long int JSON::NumberValue::getInt(){
	if(this->type == NUMTYPE_FLOATINGPOINT)throw "This is a floating point value.";
	return *(long long int*)this->data;
}

double JSON::NumberValue::getDouble(){
	if(this->type == NUMTYPE_INTEGER)throw "This is an integer value.";
	return *(double*)this->data;
}

JSON::Element& JSON::Object::operator[](const char* s){
	for (unsigned i = 0; i < this->fieldAmount; ++i){
		if(!strcmp(s, this->fieldNames[i]))return *this->fields[i];
	}throw "No field found with such name.";
}

JSON::Element& JSON::Array::operator[](unsigned i){
	if(i >= this->elementAmount)throw "Out of bounds index for array.";
	return *this->elements[i];
}







JSON::Element* JSON::parse(const char* s, ParseError* err){
	s = getAfterSpace(s);
	Element* result = detectTypeAndInstanciate(*s);
	if(!result->parse(s, &s, err)){
		delete result;
		return 0;
	}return result;
}

void JSON::destroy(Element* el){
	delete el;
}