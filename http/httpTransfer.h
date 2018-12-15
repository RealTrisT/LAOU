#ifndef HEADER_HTTPTRANSFER
#define HEADER_HTTPTRANSFER

#include "../DynamicDataStructs/DynamicBuffer.h"

struct HttpTransfer{
	virtual bool add(char* buffer, unsigned length) = 0;
	virtual bool finished() = 0;
	virtual unsigned getSize() = 0;
	virtual char* getBuffer() = 0;
};

struct ChunkedTransfer : public HttpTransfer{
	ChunkedTransfer();
	bool add(char* buffer, unsigned length);
	bool finished();
	unsigned getSize();
	char* getBuffer();
private:
	enum States{
		LAST_COMPLETE,		//zero sized chunk
		LAST_SEPARATOR_P1,	//read chunk, ready to read \r, after that becomes LAST_SEPARATOR_P2
		LAST_SEPARATOR_P2,	//read \r, ready to read \n, after that becomes LAST_COMPLETE
		LAST_INCOMPLETE,	//awaiting rest of chunk, after that becomes LAST_SEPARATOR
		LAST_PENDING,		//awaiting number read, when full number is present, will read and go to LAST_INCOMPLETE
	} state;
	unsigned leftOver;		//only matters in LAST_INCOMPLETE
	DynamicBuffer buffer;	//only matters in LAST_PENDING
	DynamicBuffer data;
};

struct NonChunkedTransfer : public HttpTransfer{
	NonChunkedTransfer(unsigned length);
	bool add(char* buffer, unsigned length);
	bool finished();
	unsigned getSize();
	char* getBuffer();
private:
	DynamicBuffer data;
	unsigned length;
};

#endif