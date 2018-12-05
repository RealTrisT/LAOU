#include <stdio.h>
#include <string>
#include "json.h"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

int main(int argc, char const *argv[]){
	JSON::ParseError err = {0};
	char yeet[4000] = {0};

	fread(yeet, 1, 4000, stdin);

	JSON::Element* jsonObj = JSON::parse(yeet, &err);
	printf("object: %p\n",jsonObj);
	if(jsonObj){
		try{
			printf("%s\n",
				(*jsonObj)["stupid_boy"][1].getString().c_str()	
			);
		}catch( const char* yee ){
			puts(yee);
		}
	}else{
		std::string elstr = yeet; elstr.insert(err.strerror-yeet, KRED "|" KNRM);
		printf("----------%s---------\n%s", err.errorstr, elstr.c_str());
		
	}

	return 0;
}