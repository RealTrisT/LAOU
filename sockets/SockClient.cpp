#include "SockClient.h"

#ifdef _WIN32
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////      W      I      N      D      O      W      S      ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <string>

//#include <ws2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

bool SockClient::Build(const char* addr, unsigned short port){

	//initializing values
	this->connectSocket = INVALID_SOCKET;
	ZeroMemory(&this->hints, sizeof(this->hints));
	this->hints.ai_family = AF_UNSPEC;
	this->hints.ai_socktype = SOCK_STREAM;
	this->hints.ai_protocol = IPPROTO_TCP;

	//saving addr and port
	this->addr = (char*)addr; this->port = port;

	//variable that's gonna hold the error codes for calls
	int iResult = 0;

	//info of our addr
	addrinfo *ptr = NULL;

	//wsaStartup
	iResult = WSAStartup(MAKEWORD(2, 2), &this->wsaData);
	if (iResult != 0) {		
		goto fail;
	}

	//getaddrinfo
	if (!this->updateAddrInfo(&iResult)) {
		goto fail_after_wsastartup;
	}

	//attempt to establish socket and connect to the different interfaces
	for (ptr = this->addrInfo; ptr != NULL; ptr = ptr->ai_next) {
		this->connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (this->connectSocket == INVALID_SOCKET) {
			goto fail_after_getaddrinfo;
		}

		if ((iResult = connect(this->connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen)) == SOCKET_ERROR) {
			closesocket(this->connectSocket);
			this->connectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}freeaddrinfo(this->addrInfo);

	//if we couldn't find a single interface that worked, fail
	if (this->connectSocket == INVALID_SOCKET) {
		goto fail_after_wsastartup;
	}


	return true;

	fail_after_getaddrinfo:
	freeaddrinfo(this->addrInfo);

	fail_after_wsastartup:
	WSACleanup();

	fail:
	return false;
}


bool SockClient::updateAddrInfo(int* result){
	if (!addr)return false;
	int result_local = getaddrinfo(addr, std::to_string(port).c_str(), &hints, &this->addrInfo);
	if (result)*result = result_local;
	return !result_local;
}

bool SockClient::Write(void* buffer, unsigned buffersize, unsigned* bytes_written){
	int iResult = send(this->connectSocket, (char*)buffer, buffersize, 0);
	if(bytes_written)*bytes_written = iResult;
	return (iResult == SOCKET_ERROR)?false:true;
}

bool SockClient::Read(void* buffer, unsigned buffersize, unsigned* bytes_received){
	int iResult = recv(this->connectSocket, (char*)buffer, buffersize, 0);
	if (iResult > 0) {
		if (bytes_received)*bytes_received = iResult;
		return true;
	} else if (!iResult) {
		return false;
	} else {
		return false;
	}
}

void SockClient::Kill(){
	shutdown(this->connectSocket, SD_BOTH);
	closesocket(this->connectSocket);
	WSACleanup();
	return;
}

#elif __linux__
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////      L      I      N      U      X      //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//TODO: CHANGE THIS INTO THE SAME FORMAT AS WINDOWS

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>		//gethostbyname
#include <string.h>		//memcpy

SockClient::SockClient(){
	this->SockState = States::Uninited;
}

SockClient::SockClient(const char* paddr, unsigned short pport) : SockClient(){
	this->addr = (char*)paddr; this->port = pport;
	this->sock = socket(AF_INET, SOCK_STREAM, 0);
	if(this->sock == -1){
		this->SockState = States::Uninited;
		throw SockClientException(SockClientException::ErrCodes::SOCKCREATE_FAILED, errno);
	}this->SockState = States::SockCreated;

	struct hostent* he;
	if(!(he = gethostbyname(paddr))){
		this->SockState = States::Uninited;
		throw SockClientException(SockClientException::ErrCodes::SOCKHOSTGET_FAILED, h_errno);
	}this->SockState = States::HostGot;

	memcpy(&this->server.sin_addr, he->h_addr_list[0], he->h_length);
	this->server.sin_family = AF_INET;
	this->server.sin_port = htons(pport);

	if(connect(this->sock, (sockaddr*)&this->server, sizeof(this->server)) < 0){
		this->SockState = States::Uninited;
		throw SockClientException(SockClientException::ErrCodes::SOCKCONNECT_FAILED, errno);
	}this->SockState = States::SockConnected;
}

SockClient::~SockClient(){
	this->kill();
}

bool SockClient::write(void* buffer, unsigned buffersize){
	if(this->SockState == States::Uninited)throw SockClientException(SockClientException::ErrCodes::SOCKUNINITED, 0);
	if(send(this->sock, buffer, buffersize, 0) < 0)
		throw SockClientException(SockClientException::ErrCodes::SOCKSEND_FAILED, errno);
}
bool SockClient::read(void* buffer, unsigned buffersize, unsigned* bytesReceived){
	if(this->SockState == States::Uninited)throw SockClientException(SockClientException::ErrCodes::SOCKUNINITED, 0);
	int iResult = 0;
	iResult = recv(this->sock, (char*)buffer, buffersize, 0);
	if (iResult > 0) {
		if (bytesReceived)*bytesReceived = iResult;
		return true;
	} else if (!iResult) {
		return false;
	} else {
		throw SockClientException(SockClientException::ErrCodes::SOCKRECV_FAILED, errno);
	}
}

void SockClient::kill(){
	if(this->SockState != States::Uninited)close(this->sock);
}

const char* SockClient::SockClientException::errDescriptors[(unsigned)ErrCodes::ERRAMOUNT] = {
	"Failed to Get Provided Host",
	"Failed to Create Socket",
	"Failed to Connect Socket",
	"Failed to Receive",
	"Failed to Send",
	"Operation Attempt on Uninitiated Socket"
};

#else
	#error Sockets Not Implemented For The Current Platform
#endif