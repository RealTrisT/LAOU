#ifndef HEADER_HTTPHEADERS
#define HEADER_HTTPHEADERS

#include <vector>

struct HttpHeaders{
	static bool Build(HttpHeaders* me, char* buffer, unsigned length);
	~HttpHeaders();

	struct header{char* name; char* value;};
	std::vector<header> headers;

	char* get(char* header);
};

#endif