#pragma once

struct Sock{
	//returns true if connected, false if not
	virtual bool Build(const char* addr, unsigned short port) = 0;

	//returns true if information was read, false if failed or closed
	virtual bool Read(void* buffer, unsigned buffersize, unsigned* bytes_read = 0) = 0;

	//returns true if information was written, false if failed or closed
	virtual bool Write(void* buffer, unsigned buffersize, unsigned* bytes_written = 0) = 0;

	//kills the socket
	virtual void Kill() = 0;
};