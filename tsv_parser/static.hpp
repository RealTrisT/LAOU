#include <vector>
#include <stdlib.h>

struct StaticTsvArr{
	StaticTsvArr(unsigned fieldAmount, void(*fin)(void*), char(*isWhat)(void**) = (char(*)(void**))isWhat_Char);
	~StaticTsvArr();

	unsigned fieldAmount;
	
private:
	//this function goes:
	//each call, it increases the pointer so it points to the next character
	//if end of string was reached, returns 0
	//if a tab value was found, returns 1
	//otherwise returns -1
	char (*isWhat)(void**);
	void (*fin)(void*);

	std::vector<void**> entry;

public:
	bool feed(void* str);
	unsigned size();
	void* operator()(unsigned i, unsigned f);

private:
	static char isWhat_Char(char**c);
};