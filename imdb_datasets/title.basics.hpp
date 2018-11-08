struct TitleEntry{
	enum TitleType{
		tTVEPISODE, tMOVIE, tTVSERIES, tSHORT,
		tTVMOVIE, tTVSHORT, tTVMINISERIES, tTVSPECIAL, 
		tVIDEO, tVIDEOGAME, tTypeAmount, tINVALID
	}; static const char* titleTypeStrings[tTypeAmount];

	TitleEntry(
		char* imdbID, 
		char* titleType, 
		unsigned char* primaryTitle,
		unsigned char* originalTitle,
		char* isAdult,
		char* startYear,
		char* endYear,
		char* runtimeMinutes,
		char* genres				//TODO: fix genres - bit flags
	);

	unsigned imdbID;
	TitleType titleType;
	unsigned char* primaryTitle;
	unsigned char* originalTitle;
	bool isAdult;
	unsigned startYear;
	unsigned endYear;
	unsigned runtimeMinutes;
	char* genres;


	static TitleType type(char* s);

private:
	unsigned nr(char* s);

	bool isItAdult(char* s);
};