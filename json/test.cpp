#include <stdio.h>
#include "json.h"

int main(int argc, char const *argv[]){
	JSON::Element* jsonObj = JSON::parse("\
{\
	\"stupid_boy\": [\
		1, \
		\"stupid boy position 2\", \
		1.8, \
		true, \
		false, \
		null,\
		{\
			\"AYE\" :	421,\
			\"AYO\" :   13.37,\
			\"EYE\" :	null\
		} \
	]\
}");
	printf("object: %p\n",jsonObj);
	printf("%s\n", ((JSON::StringValue&)((JSON::Array&)(*(JSON::Object*)
		
		jsonObj)["stupid_boy"])
		[1])
		.string

	);
	return 0;
}