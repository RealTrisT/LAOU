#ifndef HEADER_HTTP
#define HEADER_HTTP

#include "httpTransfer.h"
#include "httpHeaders.h"
#include "httpStatus.h"

struct HttpResponse{
	HttpResponse();
	~HttpResponse();
	bool isOver();	//done
	bool isBad();	//done
	void add(char* buffer, unsigned bufferSize);

	HttpStatus		status;
	HttpHeaders 	headers;
	HttpTransfer* 	content;

private:
	bool bad;
	bool over;

	DynamicBuffer buffer;
	bool headersPresent;

	bool processHeaders();
};

#endif