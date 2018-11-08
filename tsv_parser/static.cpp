#include "static.hpp"

StaticTsvArr::StaticTsvArr(unsigned fieldAmount, void(*fin)(void*), char(*isWhat)(void**)) 
			: fieldAmount(fieldAmount), fin(fin), isWhat(isWhat){}

StaticTsvArr::~StaticTsvArr(){
	unsigned s = entry.size(); 
	for(unsigned i = 0; i < s; ++i){
		fin(entry[i][0]);
		free(entry[i]);
	}
}

//assume string is fucked if returned false
bool StaticTsvArr::feed(void* str){
	void** curr = (void**)malloc(sizeof(void*)*fieldAmount);
	unsigned currIndex = 0;
	char t = 0;

	curr[currIndex++] = str;

	while(t = isWhat(&str))if(t == 1)curr[currIndex++] = str;

	if(currIndex != this->fieldAmount){free(curr); return false;}

	entry.push_back(curr);

	return true;
}

unsigned StaticTsvArr::size(){
	return entry.size();
} 

void* StaticTsvArr::operator()(unsigned i, unsigned f){
	return entry[i][f];
}

char StaticTsvArr::isWhat_Char(char**c){
	char v = *((*c)++);
	if(v == '\0')return 0;
	else if(v == '\t'){(*c)[-1] = '\0'; return 1;}
	else return -1;
}