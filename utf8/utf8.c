#include "utf8.h"
#include <stdlib.h>
#include <string.h>

char UTF8_isByteValidCharacter(unsigned char c){return (c & 0xC) != 0x80;}

unsigned char* UTF8_getNextChar_fromValid(unsigned char* s){
	if(!(s[0] & 0x80)){return s+1;}
	char pushForward = 2;
	if(s[0] & 0x20){
		pushForward++;
	if(s[0] & 0x10)
		pushForward++;
	}return s+pushForward;
}

unsigned char* UTF8_getNextChar(unsigned char* s){
	if((s[0] & 0xC) == 0x80){
		for(s++;(s[0] & 0xC) == 0x80;s++);
		return s;
	}else{
		return UTF8_getNextChar_fromValid(s);
	}
}

unsigned char* UTF8_getPos(unsigned char* s, unsigned x){
	for (unsigned i = 0; i < x; ++i)
		s = UTF8_getNextChar_fromValid(s);
	return s;
}


unsigned strlenUTF8(unsigned char* s){
	unsigned size = 0;
	for (; *s; s = UTF8_getNextChar_fromValid(s), ++size);
	return size;
}

unsigned strsizUTF8(unsigned char* s){
	unsigned char* initS = s;
	for (; *s; s = UTF8_getNextChar_fromValid(s)); return s-initS;
}

unsigned strlen_sizcapUTF8(unsigned char* s, unsigned maxLen, unsigned char** end){
	unsigned char* maxPtr = s + maxLen;
	unsigned char* prevS = s;
	unsigned size = 0;

	for (; *s; size++){						//if *s is not the end
		prevS = s;							//save our s
		s = UTF8_getNextChar_fromValid(s);	//get the next character's address
		if(s > maxPtr)break;				//if the next character is greater than maximum size, we stop, if it's not, we keep going
	}										//and we increase the size
	if(end)*end = prevS;					//at this point prevS will be the address to the end of the last character that doesn't go out of bounds
	return size;							//so say maxlen is 2, and the character is 3 bytes long, size will be 0, and prevS will be the initial s
}

unsigned char* strdupUTF8(unsigned char* s){
	unsigned size = strsizUTF8(s);
	unsigned char* result = (unsigned char*)malloc(size+1);

	memcpy(result, s, size);

	result[size] = 0;
	return result;
}

char* strdupUTF8toANSI(unsigned char* s, char** nullt, char dflt){
	if(!UTF8_isByteValidCharacter(*s))return 0; //broken string
	unsigned len = strlenUTF8(s);
	char* result = (char*)malloc(len+1);

	for (unsigned i = 0; *s; s = UTF8_getNextChar_fromValid(s), i++){
		if(!(s[0] & 0x80))result[i] = *s;
		else result[i] = dflt;
	}

	if(nullt)*nullt = s;

	result[len] = '\0';
	return result;
}

char* strdupUTF8toANSI_ml(unsigned char* s, unsigned maxLen, char** nullt, char dflt){
	if(!UTF8_isByteValidCharacter(*s))return 0; 										//broken string
	char* result = 0;
	unsigned len = strlenUTF8(s);														//get the length of the string
	result = (len > maxLen) ? (char*)malloc(maxLen+1) : (char*)malloc(len+1); 			//allocate space for the smallest between length and maxLen

	for (unsigned i = 0; *s && (i < maxLen); s = UTF8_getNextChar_fromValid(s), i++){	//while we haven't reached the end, or maximum allowed length,
		if(!(s[0] & 0x80))result[i] = *s;												// keep adding
		else result[i] = dflt;															// to the string
	}

	if(nullt)*nullt = s;

	result[(len > maxLen)?maxLen:len] = '\0';
	return result;
}

char* strdupUTF8toANSI_ms(unsigned char* s, unsigned maxSiz, char** nullt, char dflt){
	if(!UTF8_isByteValidCharacter(*s))return 0; 			//broken string
	unsigned maxLen = strlen_sizcapUTF8(s, maxSiz, 0);		//retreive maximum possible s length with the size we are given
	return strdupUTF8toANSI_ml(s, maxLen, nullt, dflt);
}