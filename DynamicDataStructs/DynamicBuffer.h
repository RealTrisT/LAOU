#ifndef HEADER_DYNBUFFER
#define HEADER_DYNBUFFER

#include <vector>

struct DynamicBuffer{
	DynamicBuffer();
	~DynamicBuffer();

	char* buffer;
	unsigned length;

	std::vector<char**> dependencies;

	void addDependency(char** ptr);
	void remDependency(char** ptr);
	void add(char* buffer, unsigned length);
	void removeStart(unsigned amount);
	void removeEnd(unsigned amount);
	void clear();
};

#endif