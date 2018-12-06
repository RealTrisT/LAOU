
struct RequestHttp{
	enum reqType{
		REQTYPE_GET,
		REQTYPE_HEAD,
		REQTYPE_POST,
		REQTYPE_PUT,
		REQTYPE_DELETE,
		REQTYPE_CONNECT,
		REQTYPE_OPTIONS,
		REQTYPE_TRACE,
		REQTYPE_PATCH
	}type;

	float version;
	
	char* path; unsigned pathLen;

	char* start;
	char* headerStart;
	char* contentStart;
};

struct ResponseHttp{
	float version;
	unsigned errorCode;
	char* errorCodeDescriptor;
	unsigned errorCodeDescriptorLen;
	char* headerStart;
	char* contentStart;
};

struct headerElement{
	char* name;  unsigned nameLen;
	char* value; unsigned valueLen;
};

bool HttpParseRequest(char* s, RequestHttp* result){
	result->start = s;
	if(*(unsigned*)s == *(unsigned*)"GET "){
		result->type = RequestHttp::REQTYPE_GET;
		s += 4;
	}else if(*(unsigned*)s == *(unsigned*)"POST" && s[4] == ' '){
		result->type = RequestHttp::REQTYPE_HEAD;
		s += 5;
	}else if(*(unsigned*)s == *(unsigned*)"PUT "){
		result->type = RequestHttp::REQTYPE_PUT;
		s += 4;
	}else if(*(unsigned*)s == *(unsigned*)"DELE" && *(unsigned short*)&s[4] == *(unsigned short*)"TE" && s[6] == ' '){
		result->type = RequestHttp::REQTYPE_DELETE;
		s += 7;
	}else if(*(unsigned*)s == *(unsigned*)"PATC" && *(unsigned short*)&s[4] == *(unsigned short*)"H "){
		result->type = RequestHttp::REQTYPE_PATCH;
		s += 6;
	}else if(*(unsigned*)s == *(unsigned*)"HEAD" && s[4] == ' '){
		result->type = RequestHttp::REQTYPE_HEAD;
		s += 5;
	}else if(*(unsigned*)s == *(unsigned*)"CONN" && *(unsigned*)&s[4] == *(unsigned*)"ECT "){
		result->type = RequestHttp::REQTYPE_CONNECT;
		s += 8;
	}else if(*(unsigned*)s == *(unsigned*)"OPTI" && *(unsigned*)&s[4] == *(unsigned*)"ONS "){
		result->type = RequestHttp::REQTYPE_OPTIONS;
		s += 8;
	}else if(*(unsigned*)s == *(unsigned*)"TRAC" && *(unsigned short*)&s[4] == *(unsigned short*)"E "){
		result->type = RequestHttp::REQTYPE_TRACE;
		s += 6;
	}else{
		return false;
	}
	result->path = s;

	while(*s != ' ' && *s != 0)s++;
	if(!*s)return 0;

	result->pathLen = (unsigned)(s - result->path);

	s++;

	if(*(unsigned*)s != *(unsigned*)"HTTP" || s[4] != '/')return false;

	s += 5;

	char* temp = s;
	unsigned versLength = 0;
	while(*temp == '.' || (*temp >= '0' && *temp <= '9')){versLength++; temp++;}
	if(!versLength)return 0;

	char versn[versLength+1];
	for (unsigned i = 0; i < versLength; ++i)versn[i] = s[i];
	versn[versLength] = '\0';

	result->version = atof(versn);

	s += versLength;

	result->headerStart = s;

	if(s[0] == '\0' || s[1] == '\0' || s[2] == '\0' || s[3] == '\0')return false;

	result->contentStart = 0;
	while(s[3] != '\0')if(*(unsigned*)s++ == *(unsigned*)"\r\n\r\n"){result->contentStart = s+3; break;}
	
	if(!result->contentStart)return false;
}




bool HttpParseResponse(char* s, ResponseHttp* result){
	if(*(unsigned*)s != *(unsigned*)"HTTP" || s[4] != '/')return false;				//if the next 5 bytes dont make "HTTP/", invalid
	s += 5;																			//push pointer forward
	char* temp = s;																	//create a temporary variable for storing the end of the version "string"
	unsigned versLength = 0;														//and another for storing it's length
	while(*temp == '.' || (*temp >= '0' && *temp <= '9')){versLength++; temp++;}	//while it has characters that would be in a version string, increase
	if(!versLength)return 0;														//if there are no version characters, bad request, fail
	char versn[versLength+1];														//allocate temp buffer on the stack for our version string
	for (unsigned i = 0; i < versLength; ++i)versn[i] = s[i];						//copy the version over
	versn[versLength] = '\0';														//null terminate
	result->version = atof(versn);													//convert to float and attrib
	s += versLength;																//adjust our pointer to after the version
	while(*s == ' ')s++;															//find next char after space/s
	if(!*s)return false;															//if reached end of string, fail
	temp = s;																		//store s in temp, we will use temp to store the end of the error code
	unsigned errcodeLen = 0;														//same thing here, save length
	while(*temp >= '0' && *temp <= '9'){errcodeLen++; temp++;}						//while these are numbers, go on increasing the length and temp pointer
	if(!errcodeLen)return false;													//if it's got no error code, fail
	char errorCode[errcodeLen+1];													//allocate buffer for it
	for (unsigned i = 0; i < errcodeLen; ++i)errorCode[i] = s[i];					//copy stuff over
	errorCode[errcodeLen] = '\0';													//terminate
	result->errorCode = atoi(errorCode);											//convert to int and put it in the struct
	while(*s != ' ' && *s != 0)s++;													//skip next space/s
	if(!*s)return false;															//if found end of string fail
	if(!s[1])return false;															//if the next character after that is end of string fail
	temp = s;																		//temp will now retain the pointer to the beginning of the descriptor
	while(*(short*)s != *(short*)"\r\n" && s[1] != 0)s++;							//and we'll try and find the end of the line
	if(!s[1])return false;															//if we've reached end of string, fail
	result->errorCodeDescriptor = temp;												//set the error code descriptor to it's pointer (capitain obvious)
	result->errorCodeDescriptorLen = s - temp;										//set it's length to it's length -.-
	result->headerStart = s+=2;														//increase the pointer to after the new line, and set header start to it
	if(!s[0] || !s[1] || !s[2] || !s[3])return false;								//check in advance that none of the next 4 characters are the end of the string, if they are, fail
	while(*(unsigned*)s != *(unsigned*)"\r\n\r\n" && s[3] != '\0')s++;				//find the \r\n\r\n (delimits content start)
	if(!s[3])return false;															//if found end of string instead fail
	result->contentStart = s+=4;													//otherwise set it
	return true;																	//and happy ending
}







bool HttpHeaderParse(char* headerStart, char** headerEnd, headerElement* el){
	el->name = headerStart;
	while(
		*headerStart != ' ' && 
		*headerStart != ':' && 
		*headerStart != '\0' && 
		*(short*)headerStart != *(short*)"\r\n"
	)headerStart++;

	if(headerStart == el->name || !*headerStart || *(short*)headerStart == *(short*)"\r\n")return false;
	el->nameLen = headerStart - el->name;

	while(*headerStart != ':' && *headerStart && *(short*)headerStart != *(short*)"\r\n")headerStart++;
	if(!*headerStart || *(short*)headerStart == *(short*)"\r\n")return false;

	headerStart++;
	while(*headerStart == ' ')headerStart++;
	el->value = headerStart;

	if(!headerStart[0] || !headerStart[1])return false; //if theres a zero this is invalid http

	while(*(short*)headerStart != *(short*)"\r\n" && headerStart[1] != '\0')headerStart++;

	if(headerStart[1] == '\0')return 0;

	el->valueLen = headerStart - el->value;

	return true;
}