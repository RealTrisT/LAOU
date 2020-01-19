#pragma once

#include "Sock.h"

#ifdef _WIN32
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////      W      I      N      D      O      W      S      ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//WHEN ON WINDOWS, COMPILE WITH "-lws2_32"
	#define WIN32_LEAN_AND_MEAN
	
	//////////////////////////////////
	#undef _WIN32_WINNT 			//
	#define _WIN32_WINNT 0x501		// idk why tf this works. windows bullshit. shrug
	#include <ws2tcpip.h>			//
	//////////////////////////////////


	struct SockClient : Sock {
		bool Build(const char* addr, unsigned short port);

		bool Write(void* buffer, unsigned buffersize, unsigned* bytes_written = 0);

		bool Read(void* buffer, unsigned buffersize, unsigned* bytes_read = 0);

		void Kill();


	protected:
		char* addr;
		unsigned short port;

		addrinfo hints;
		WSADATA wsaData;
		//States SockState;
		addrinfo* addrInfo;
		SOCKET connectSocket;

		bool updateAddrInfo(int* result = 0);
	};

	

#elif __linux__
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////      L      I      N      U      X      //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <sys/socket.h>
#include <arpa/inet.h>

	struct SockClient {
		enum struct States {
			Uninited,
			SockCreated,
			HostGot,
			SockConnected,
		};
		struct SockClientException {
			friend struct SockClient;
			private:
				enum struct ErrCodes {
					SOCKHOSTGET_FAILED,
					SOCKCREATE_FAILED,
					SOCKCONNECT_FAILED,
					SOCKRECV_FAILED,
					SOCKSEND_FAILED,
					SOCKUNINITED,
					ERRAMOUNT
				};
				ErrCodes errCode;
				unsigned additionalErrcode;
				static const char* errDescriptors[(unsigned)ErrCodes::ERRAMOUNT];

			public:
				SockClientException(ErrCodes rc, unsigned arc) : errCode(rc), additionalErrcode(arc) {}
				const char* Message() { return errDescriptors[(unsigned char)errCode]; }
				//const char* Message = errDescriptors[(unsigned)errCode];
				unsigned ErrorCode() { return additionalErrcode; }
		};
		SockClient();
		SockClient(const char* paddr, unsigned short pport);
		~SockClient();

		char* addr;
		unsigned short port;
	protected: //                       TODO: sock is protected
	public:
		int sock;
	private:
		sockaddr_in server;
		States SockState;
	public:
		bool write(void* buffer, unsigned buffersize);
		bool read(void* buffer, unsigned buffersize, unsigned* bytesReceived = 0);
		void kill();
	};

#else
	#error Sockets Not Implemented For The Current Platform
#endif