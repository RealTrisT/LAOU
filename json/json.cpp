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

//receives a pointer to the delimiting character
bool JSON::Object::parse(const char* s, const char** e){
	for (;;){
		s = getAfterSpace(s+1);
		if(!*s || *s != '"')return false;
		char* strbegin = (char*)(s+1);
		s = getQuotationsEnd(s);
		if(!s || s == strbegin)return false;

		unsigned nameLen = s - strbegin;								//name string length
		this->fieldNames.push_back(new char[nameLen+1]);				//allocation to new element of vector
		memcpy(this->fieldNames[this->fieldAmount], strbegin, nameLen);	//copy of the string
		this->fieldNames[this->fieldAmount][nameLen] = '\0';			//null-terminate

		s = getAfterSpace(s+1);
		if(!*s || *s != ':')goto destroyProgressAndFailObj_n;				//TODO: nullcheck probably unnecessary
		s = getAfterSpace(s+1);
		if(!*s)goto destroyProgressAndFailObj_n;							//TODO: nullcheck probably unnecessary

		this->fields.push_back(detectTypeAndInstanciate(*s));
		if(!this->fields[this->fieldAmount]->parse(s, &s))goto destroyProgressAndFailObj_i;

		s = getAfterSpace(s);

		if(*s == '}'){
			this->fieldAmount++; break;
		}else if(*s == ','){
			this->fieldAmount++; s++;
		}else goto destroyProgressAndFailObj_i;
	}

	*e = s+1;
	return true;


destroyProgressAndFailObj_i:
	delete this->fields[this->fieldAmount];			//destroy current instance
destroyProgressAndFailObj_n:
	delete[] this->fieldNames[this->fieldAmount];	//destroy current name

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

bool JSON::Array::parse(const char* s, const char** e){
	for(;;){
		s = getAfterSpace(s+1);
		this->elements.push_back(detectTypeAndInstanciate(*s));
		if(!this->elements[this->elementAmount]->parse(s, &s))goto destroyProgressAndFailArr;
		s = getAfterSpace(s);
		if(*s == ','){
			this->elementAmount++; s++;
		}else if(*s == ']'){
			this->elementAmount++; break;
		}else goto destroyProgressAndFailArr;
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

bool JSON::Null::parse(const char* s, const char** e){
	if(*(unsigned int*)s == *(unsigned int*)"null"){
		*e = s+4; 
		return true;
	}
	return false;
}

void JSON::Null::destroy(){
	return;
}

bool JSON::NumberValue::parse(const char* s, const char** e){
	char dotCount = 0, eCount = 0;
	const char* ts = s;

	for (;;++ts){
		if(*ts <= '9' && *ts >= '0'){
			continue;
		}else if(*ts == 'e' || *ts == 'E'){
			if(ts == s)return false;
			eCount++; continue;
		}else if(*ts == '.'){
			if(ts == s)return false; //NOTE: this might be allowed maybe? .25 seems like something that'd be readable
			dotCount++; continue;
		}else break;
	}

	if(ts == s || eCount > 1 || dotCount > 1)return false;

	unsigned nrStringSize = ts - s;
	char* elstr = new char[nrStringSize+1];
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
}

void JSON::NumberValue::destroy(){
	if(this->type == NUMTYPE_FLOATINGPOINT) delete (double*)data;
	else 									delete (long long int*)data;	
}


bool JSON::BooleanValue::parse(const char* s, const char** e){
	if(*(unsigned*)s == *(unsigned*)"true"){
		*e = s+4;
		this->data = true;
	}else if(*(unsigned*)s == *(unsigned*)"fals" && s[4] == 'e'){
		*e = s+5;
		this->data = false;
	}else return false;
	return true;
}

void JSON::BooleanValue::destroy(){
	return;
}

bool JSON::StringValue::parse(const char* s, const char** e){
	char* end = (char*)getQuotationsEnd(s);
	if(!*end)return false;
	s++;

	unsigned length = end-s;

	this->string = new char[length+1];
	memcpy((void*)this->string, s, length);
	((char*)this->string)[length] = '\0';

	*e = s+length+1; //+1 for the '"'
	return true;
}

void JSON::StringValue::destroy(){
	delete[] this->string;
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









































JSON::Element* JSON::parse(const char* s){
	s = getAfterSpace(s);
	Element* result = detectTypeAndInstanciate(*s);
	if(!result->parse(s, &s)){
		delete result;
		return 0;
	}return result;
}

void JSON::destroy(Element* el){
	delete el;
}