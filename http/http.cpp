#include "http.h"

#include <stdlib.h>
#include <string.h>

HttpResponse::HttpResponse(){
	this->bad = false;
	this->over = false;
	this->headersPresent = false;
}

HttpResponse::~HttpResponse(){
	if(this->content)delete this->content;
}

void HttpResponse::add(char* buffer, unsigned buffersize){
	if(headersPresent){
		if(!this->over){
			if(content->add(buffer, buffersize)){
				if(content->finished())this->over = true;
			}else{
				this->bad = true;
			}
		}
	}else{
		this->buffer.add(buffer, buffersize);
		for (unsigned i = 0; i < this->buffer.length-1; ++i){
			if(*(unsigned*)&this->buffer.buffer[i] == *(unsigned*)"\r\n\r\n"){
				this->headersPresent = true;
				if(this->processHeaders()){
					if(this->content){
						this->content->add(buffer+i+4, this->buffer.length-i-4);
						this->over = this->content->finished();
					}else this->over = true;
				}else this->bad = true;
				this->buffer.clear();
				break;
		}	}
	}
}

bool HttpResponse::isOver(){
	return this->over;
}

bool HttpResponse::isBad(){
	return this->bad;
}

bool HttpResponse::processHeaders(){
	char* endOfStatusHeader = this->buffer.buffer;
	if(HttpStatus::Build(&this->status, endOfStatusHeader, this->buffer.length, &endOfStatusHeader)){
		if(HttpHeaders::Build(&this->headers, endOfStatusHeader, this->buffer.length - (endOfStatusHeader - this->buffer.buffer))){
			char* te = this->headers.get((char*)"Transfer-Encoding");
			if(te){				//if thransfer-encoding exists, then it's prolly chunked (TODO: do not just assum chunked)
				this->content = new ChunkedTransfer();
			}else{				//if theres no transfer encoding then it's prolly content-length
				char* lenStr = this->headers.get((char*)"Content-Length");			//get the contentlength string
				if(lenStr){															//if we got it
					while(*lenStr == ' ')lenStr++;									//skip whitespaces for atoi
					
					char* lenStrEnd = lenStr;										//for  the end of the number
					while(*lenStrEnd >= '0' && *lenStrEnd <= '9')lenStrEnd++;		//find the end of the number
					
					unsigned numberSize = lenStrEnd - lenStr;						//lengti
					char numberBuffer[numberSize + 1];								//create buffer on stacko
					memcpy(numberBuffer, lenStr, numberSize);						//copy to buffer on stacko
					numberBuffer[numberSize] = '\0';								//null terminate stacko
					unsigned nmbr = atoi(lenStr);									//get numbur
					
					this->content = new NonChunkedTransfer(nmbr);					//creat ting
				}else{																//if no transferencoding or messagebody
					this->content = 0;												//then it is a response with no bodi
				}
			}
		}else return false;															//if we could parse the headers, fuckall
	}else return false;

	return true;																//succ_ess
}

