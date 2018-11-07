
//get whether the provided byte is a valid character (as opposed to an additional byte to a codepoint)
char UTF8_isByteValidCharacter(unsigned char c);

//get the next character assuming we could be anywhere
unsigned char* UTF8_getNextChar(unsigned char* s);

//get the next character assuming the pointer provided points to a valid one
unsigned char* UTF8_getNextChar_fromValid(unsigned char* s);

//gets the pointer to the character at position x
unsigned char* UTF8_getPos(unsigned char* s, unsigned x);


//length of string in characters
unsigned strlenUTF8(unsigned char* s);

//size of string in bytes
unsigned strsizUTF8(unsigned char* s);

//obtain length of the provided string if truncated at maxLen bytes, end provides the address of the byte after the last character
unsigned strlen_sizcapUTF8(unsigned char* s, unsigned maxLen, unsigned char** end);

//allocate and copy string
unsigned char* strdupUTF8(unsigned char* s);

//allocate space for an ANSI string representing the provided UTF8 one, nullt returns the pointer to the null terminator, dflt is 
//what will replace characters not representable in ANSI
char* strdupUTF8toANSI(unsigned char* s, char** nullt, char dflt);

//strdup where a max character length is provided.
//s will be truncated to the max length provided, the string can be shorter than maxLen
char* strdupUTF8toANSI_ml(unsigned char* s, unsigned maxLen, char** nullt, char dflt);

//strdup where a max size is provided.
//s will be truncated on the character that reaches maxSiz
char* strdupUTF8toANSI_ms(unsigned char* s, unsigned maxSiz, char** nullt, char dflt);