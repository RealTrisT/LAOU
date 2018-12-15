#ifndef HEADER_HTTPSTATUS
#define HEADER_HTTPSTATUS

#include "../DynamicDataStructs/DynamicBuffer.h"

struct HttpStatus{
	static bool Build(HttpStatus* me, char* buffer, unsigned length, char** end = 0);
	
	float 	 version;
	unsigned errorcode;
	char* 	 descriptor;
	~HttpStatus();
};


#endif