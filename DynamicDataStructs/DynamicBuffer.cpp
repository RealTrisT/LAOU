#include "DynamicBuffer.h"

#include <string.h>

DynamicBuffer::DynamicBuffer(){
	this->dependencies = std::vector<char**>();
	this->buffer = new char[1]; 	//just so we don't have
	this->length = 0;				//to check every add/remove
}
DynamicBuffer::~DynamicBuffer(){
	delete[] this->buffer;
}

void DynamicBuffer::addDependency(char** ptr){
	this->dependencies.push_back(ptr);
}

void DynamicBuffer::remDependency(char** ptr){
	for (int i = 0; i < this->dependencies.size(); ++i){
		if(this->dependencies[i] == ptr){
			this->dependencies.erase(this->dependencies.begin() + i); 
			break;
		}
	}
}

void DynamicBuffer::add(char* buffer, unsigned length){
	unsigned oldsize = this->length;						//save old size
	char* oldAddr    = this->buffer;						//save old addr
	this->length     = this->length + length;				//calc new size
	this->buffer     = new char[this->length];				//alloc mem
	memcpy(this->buffer, oldAddr, oldsize);					//copy over old stuff
	memcpy(this->buffer + oldsize, buffer, length);			//copy over new stuff
	delete[] oldAddr;										//erase old memory

	//fix dependant pointers
	for (unsigned i = 0; i < this->dependencies.size(); ++i)
		*this->dependencies[i] += oldAddr - this->buffer;
	return;
}

void DynamicBuffer::removeStart(unsigned amount){
	memcpy(this->buffer, this->buffer + amount, this->length -= amount);
}

void DynamicBuffer::removeEnd(unsigned amount){
	this->length -= amount;	
}

void DynamicBuffer::clear(){
	this->length = 0;
}