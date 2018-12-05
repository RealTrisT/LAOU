
struct http{
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
	}reqType type;

	char version1, version2;

	headerElement* headerFields;
	
	char* start;
	char* contentStart;

	struct headerElement{
		headerElement* next;
		char* name;
		char* value;
	}
};

bool HttpParse(char* s, http* result){

}

bool HttpRelease(http* del){
	delete del->headerFields;
	
}