#include "title.basics.hpp"

extern "C"{
	#include "../utf8/utf8.h"
}

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

TitleEntry::TitleEntry(
		char* imdbID, 
		char* titleType, 
		unsigned char* primaryTitle,
		unsigned char* originalTitle,
		char* isAdult,
		char* startYear,
		char* endYear,
		char* runtimeMinutes,
		char* genres
) : imdbID(nr(imdbID+2)), //+2 for the "tt"
	titleType(type(titleType)),
	primaryTitle(strdupUTF8(primaryTitle)),
	originalTitle(strdupUTF8(originalTitle)),
	isAdult(isItAdult(isAdult)),
	startYear(nr(startYear)),
	endYear(nr(endYear)),
	runtimeMinutes(nr(runtimeMinutes)),
	genres((char*)"unimplemented")
{}

TitleEntry::TitleType TitleEntry::type(char* s){
	TitleType result = tINVALID;
	for (unsigned i = 0; i < (unsigned)tTypeAmount; ++i){
		if(!strcmp(s, titleTypeStrings[i])){result = (TitleType)i; break;}
	}
	return result;
}

unsigned TitleEntry::nr(char* s){
	if(*(unsigned short*)s == *(unsigned short*)"\\N")return 0xFFFFFFFF;
	return strtoul(s, nullptr, 10);
}

bool TitleEntry::isItAdult(char* s){
	return *s-'0';
}

const char* TitleEntry::titleTypeStrings[TitleEntry::tTypeAmount] = {
	"tvEpisode",
	"movie",
	"tvSeries",
	"short",
	"tvMovie",
	"tvShort",
	"tvMiniSeries",
	"tvSpecial",
	"video",
	"videoGame"
};