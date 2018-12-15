#include "httpTransfer.h"

#include <string.h>

char* ChunkedTransfer::getBuffer(){
	return this->data.buffer;
}

unsigned ChunkedTransfer::getSize(){
	return this->data.length;
}

//-1 : invalid
// 0 : success
// 1 : not enough data
char isNumber(char* buffer, unsigned length, char** numberend){
	char numberOfDigits = 0;
	for (unsigned i = 0; i < length; ++i){
		if(buffer[i] == '\r'){
			if(i == length-1)return 1;
			if(buffer[i+1] == '\n' && numberOfDigits){
				*numberend = buffer + i + 2;
				return 0;
			}else{
				return -1;
			}
		}else if(	(buffer[i] >= '0' && buffer[i] <= '9')
					||
					(buffer[i] >= 'A' && buffer[i] <= 'F')
					||
					(buffer[i] >= 'a' && buffer[i] <= 'f')
				){
			numberOfDigits++;
		}else{
			return -1;
		}
	}
	return 1;
}

unsigned pullHex(char* buffer, unsigned numlen){
	unsigned multiplier = 1;
	unsigned result = 0;
	
	for (unsigned i = numlen; i != 0; --i){

		char val = buffer[i-1] - '0';
		if(val > '9'-'0'){ 
			val -= 'A'-'0';
			if(val > 'F'-'A') 
				val -= 'a'-'A';
			val += 10;
		}
		result += val*multiplier;
		multiplier <<= 4; // *16
	}
	
	

	return result;
}

ChunkedTransfer::ChunkedTransfer():state(LAST_PENDING){}

bool ChunkedTransfer::finished(){return this->state == LAST_COMPLETE;}

bool ChunkedTransfer::add(char* buffer, unsigned length){
	while(true){
		if(!length)return true;

		if(this->state == LAST_COMPLETE){			//if our last transfer was finished
			return true;
		}else if(this->state == LAST_INCOMPLETE){	//if our last packet wasn't finished
			if(length > this->leftOver){			//if we have more data than what we need for the chunk
				data.add(buffer, this->leftOver);	//add it to the data buffer
				buffer += this->leftOver;			//push the buffer forward
				length -= this->leftOver;			//shorten the length
				this->leftOver = 0;					//no more left
				this->state = LAST_SEPARATOR_P1;	//ready for separator
			}else if(length < this->leftOver){		//if we don't have enough to fill the chunk
				data.add(buffer, length);			//push what we have onto the buffer
				this->leftOver -= length;			//set our leftOver accordingly
				return true;						//return cuz theres no more data to be processed
			}else{									//if we have exactly the needed amount of data
				data.add(buffer, length);			//push it onto the buffere
				this->leftOver = 0;					//no leftovers
				this->state = LAST_SEPARATOR_P1;	//next step is the \r\n
				return true;						//return since, once again, no more data
			}
		}else if(this->state == LAST_PENDING){
			unsigned oldSize = this->buffer.length;	//save the old size of the buffer
			this->buffer.add(buffer, length);		//add the current packet to the buffer in case there was something there


			char* numbr_end = 0;
			char numbr = isNumber(this->buffer.buffer, this->buffer.length, &numbr_end);

			if(!numbr){								//success
				this->leftOver = pullHex(			//get the
					this->buffer.buffer, 			//length of
					numbr_end-this->buffer.buffer-2	//the chunk
				);
				if(!this->leftOver){				//if chnk size is zero
					this->state = LAST_COMPLETE;	//then we're done
					return true;					//ret
				}else{
					length -= numbr_end - this->buffer.buffer;	//shorten length size of the number
					buffer += numbr_end - this->buffer.buffer;	//push buffer forward size of the number
					length += oldSize;				//what was inside the pending buffer that isn't in ours
					buffer -= oldSize;				//what was inside the pending buffer that isn't in ours
					
					this->buffer.clear();			//clear the buffer so we're ready for an eventual next number
					this->state = LAST_INCOMPLETE;	//set the state so we start reading actual data
				}
			}else if(numbr == 1){					//not enough data
				return true;						//and since we have no more data, just return
			}else{									//invalid
				return false;						//so fail
			}
		}else if(this->state == LAST_SEPARATOR_P1){
			if(*buffer != '\r')return false;		//isn't what it must be, fuck off
			buffer++; length--;						//adjust buffer and length
			this->state = LAST_SEPARATOR_P2;		//onto the next step
		}else if(this->state == LAST_SEPARATOR_P2){
			if(*buffer != '\n')return false;		//isn't what it must be, fuck off
			buffer++; length--;						//adjust buffer and length
			this->state = LAST_PENDING;				//ready for the next one
		}
	}
}


NonChunkedTransfer::NonChunkedTransfer(unsigned length) : length(length){}

bool NonChunkedTransfer::add(char* buffer, unsigned length){
	if(this->data.length == this->length)return false;
	if(this->data.length + length > this->length)return false;
	this->data.add(buffer, length);
	return true;
}

bool NonChunkedTransfer::finished(){
	return this->data.length == this->length;
}

char* NonChunkedTransfer::getBuffer(){
	return this->data.buffer;
}

unsigned NonChunkedTransfer::getSize(){
	return this->data.length;
}