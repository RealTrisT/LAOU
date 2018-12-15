#include "httpStatus.h"

#include <string.h>
#include <stdlib.h>

bool HttpStatus::Build(HttpStatus* me, char* buffer, unsigned length, char** statusend){
	char* end = buffer+length;

	if(length <= 5)return false; 		//has no status
	if(!(*(unsigned*)buffer == *(unsigned*)"HTTP" && buffer[4] == '/'))return false;
	buffer+=5;

	//get the http version
	char* bufferbegin = buffer; unsigned numberlength = 0;
	while(buffer != end && ((*buffer >= '0' && *buffer <= '9') || *buffer == '.')){buffer++; numberlength++;}
	if(buffer == end || !numberlength)return false;
	char versionnumbuffer[numberlength+1];
	memcpy(versionnumbuffer, bufferbegin, numberlength);
	versionnumbuffer[numberlength] = '\0';


	//skip whitespass
	while(buffer != end && *buffer == ' ')buffer++; if(buffer == end)return false;

	//get the error code
	bufferbegin = buffer; numberlength = 0;
	while(buffer != end && (*buffer >= '0' && *buffer <= '9')){buffer++; numberlength++;}
	if(buffer == end || !numberlength)return false;
	char errornumbuffer[numberlength];
	memcpy(errornumbuffer, bufferbegin, numberlength);
	errornumbuffer[numberlength] = 0;

	//skip whitespass
	while(buffer != end && *buffer == ' ')buffer++; if(buffer == end)return false;

	//get the error descriptor
	bufferbegin = buffer; numberlength = 0;
	while((buffer+1) != end && *(short*)buffer != *(short*)"\r\n"){buffer++; numberlength++;}
	if ((buffer+1) == end || !numberlength)return false;

	//APPLY EVERYTHING
	me->descriptor = new char[numberlength + 1];
	memcpy(me->descriptor, bufferbegin, numberlength);
	me->descriptor[numberlength] = '\0';

	me->errorcode = atoi(errornumbuffer);
	me->version   = atof(versionnumbuffer);

	if(statusend)*statusend = buffer+2;
	return true;
}

HttpStatus::~HttpStatus(){
	delete[] this->descriptor;
}