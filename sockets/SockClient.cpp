#include "SockClient.h"

#ifdef _WIN32
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////      W      I      N      D      O      W      S      ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <string>

#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

SockClient::SockClient(){
	this->SockState = States::Uninited;				this->connectSocket = INVALID_SOCKET;
	ZeroMemory(&this->hints, sizeof(this->hints));	this->hints.ai_family = AF_UNSPEC;
	this->hints.ai_socktype = SOCK_STREAM;			this->hints.ai_protocol = IPPROTO_TCP;
}

SockClient::SockClient(const char* paddr, unsigned short pport) : SockClient(){
	this->addr = (char*)paddr; this->port = pport;

	int iResult = 0;
	addrinfo *ptr = NULL;

	iResult = WSAStartup(MAKEWORD(2, 2), &this->wsaData);
	if (iResult != 0) {
		throw SockClientException(SockClientException::ErrCodes::WSASTARTUP_FAILED, iResult);
	}this->SockState = States::WSAInited;

	if (!updateAddrInfo(&iResult)) {
		this->SockState = States::Uninited;
		WSACleanup();
		throw SockClientException(SockClientException::ErrCodes::GETADDRINFO_FAILED, iResult);
	}this->SockState = States::AddrinfoGotten;

	for (ptr = this->addrInfo; ptr != NULL; ptr = ptr->ai_next) {
		this->connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (this->connectSocket == INVALID_SOCKET) {
			this->SockState = States::Uninited;
			freeaddrinfo(this->addrInfo);
			WSACleanup();
			throw SockClientException(SockClientException::ErrCodes::SOCKCREATE_FAILED, WSAGetLastError());
		}

		if (SOCKET_ERROR == (iResult = connect(this->connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen))) {
			closesocket(this->connectSocket);
			this->connectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}freeaddrinfo(this->addrInfo);

	if (this->connectSocket == INVALID_SOCKET) {
		this->SockState = States::Uninited;
		WSACleanup();
		throw SockClientException(SockClientException::ErrCodes::SOCKCONNECT_FAILED, WSAGetLastError());
	}this->SockState = States::SockConnected;
}

SockClient::~SockClient(){
	if (this->SockState != States::Uninited)this->kill();
}

bool SockClient::updateAddrInfo(int* result){
	int result_local = 0;
	if (!addr) { return false; }
	result_local = getaddrinfo(addr, std::to_string(port).c_str(), &hints, &this->addrInfo);
	if (result)*result = result_local;
	return !result_local;
}

bool SockClient::write(void * buffer, unsigned buffersize){
	int iResult = 0;
	if (this->connectSocket == INVALID_SOCKET)throw SockClientException(SockClientException::ErrCodes::SOCKUNINITED, 0);
	iResult = send(this->connectSocket, (char*)buffer, buffersize, 0);
	if (iResult == SOCKET_ERROR) {
		throw SockClientException(SockClientException::ErrCodes::SOCKSEND_FAILED, WSAGetLastError());
	}
	return true;
}

bool SockClient::read(void* buffer, unsigned buffersize, unsigned* bytesReceived){
	int iResult = 0;
	if(this->connectSocket == INVALID_SOCKET)throw SockClientException(SockClientException::ErrCodes::SOCKUNINITED, 0);
	iResult = recv(this->connectSocket, (char*)buffer, buffersize, 0);
	if (iResult > 0) {
		if (bytesReceived)*bytesReceived = iResult;
		return true;
	} else if (!iResult) {
		return false;
	} else {
		throw SockClientException(SockClientException::ErrCodes::SOCKRECV_FAILED, WSAGetLastError());
	}
}

void SockClient::kill(){
	if (this->SockState == States::SockConnected)shutdown(this->connectSocket, SD_BOTH);
	this->SockState = States::Uninited;
	closesocket(this->connectSocket);
	WSACleanup();
	return;
}

const char* SockClient::SockClientException::errDescriptors[(unsigned)ErrCodes::ERRAMOUNT] = {
	"Failed to Get Address Info",
	"Failed to Create Socket",
	"Failed to Connect Socket",
	"Failed to Receive",
	"Failed to Send",
	"Operation Attempt on Uninitiated Socket"
};

#elif __linux__
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////      L      I      N      U      X      //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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