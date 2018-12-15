#include "httpHeaders.h"

#include <string.h>

HttpHeaders::~HttpHeaders(){
	for (int i = 0; i < headers.size(); ++i){
		delete[] headers[i].name;
		delete[] headers[i].value;
	}
}

bool HttpHeaders::Build(HttpHeaders* me, char* buffer, unsigned length){
	unsigned i = 0;																	//index
	while(true){																	//while we have headers
		unsigned begin_index = i;													//the beginning of the current header is here
		if(((length - i - 1) > 2) && *(short*)&buffer[i] == *(short*)"\r\n")break;	//if the next thing is \r\n, this is the end of the headers

		for (; i < length; i++)if(buffer[i] == ':')break;							//go forward till we find the first ':'
		if(buffer[i] != ':')goto BAD;												//if it's not ':' means it's the end and that it's invalid
		unsigned colon_index = i;													//save this index

		for (; i < length-1; i++)if(*(short*)&buffer[i] == *(short*)"\r\n")break;	//search for the end of current header field (\r\n)
		if(buffer[i] != '\r')goto BAD;												//again, if it's not what's desired, fail
		unsigned linebreak_index = i;												//save
		i+=2;																		//push forward by 2, so it now points to the next header field, or the finishing \r\n

		unsigned namesize = colon_index - begin_index;								//build
		char* name = new char[namesize+1];											//the
		memcpy(name, buffer + begin_index, namesize);								//name
		name[namesize] = '\0';														//my friend

		unsigned valuesize = linebreak_index - (colon_index+1);						//build
		char* value = new char[valuesize];											//the
		memcpy(value, buffer + colon_index + 1, valuesize);							//value
		value[valuesize] = '\0';													//my friend

		me->headers.push_back({name, value});										//and add it to our magical vector
	}																				//if we break outta the loop, means headers are over
	return true;																	//so succ_ess
BAD:
	for (int i = 0; i < me->headers.size(); ++i){									//otherwise
		delete[] me->headers[i].name;												//destroy all
		delete[] me->headers[i].value;												//the evidence
	}
	return false;																	//and fail
}

char* HttpHeaders::get(char* header){
	char* result = 0;
	for (unsigned i = 0; i < this->headers.size(); ++i){
		if(!strcmp(this->headers[i].name, header)){
			result = this->headers[i].value;
			break;
	}	}
	return result;
}