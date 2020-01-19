#pragma once


#ifdef _WIN32
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////      W      I      N      D      O      W      S      ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//WHEN ON WINDOWS, COMPILE WITH "-lws2_32"
	#define WIN32_LEAN_AND_MEAN
	//#define _WIN32_WINNT _WIN32_WINNT_NT4			// Windows NT 4.0  
	//#define _WIN32_WINNT _WIN32_WINNT_WIN2K		// Windows 2000  
	//#define _WIN32_WINNT _WIN32_WINNT_WINXP		// Windows XP  
	//#define _WIN32_WINNT _WIN32_WINNT_WS03		// Windows Server 2003  
	//#define _WIN32_WINNT _WIN32_WINNT_WIN6		// Windows Vista  
	//#define _WIN32_WINNT _WIN32_WINNT_VISTA		// Windows Vista  
	//#define _WIN32_WINNT _WIN32_WINNT_WS08		// Windows Server 2008  
	//#define _WIN32_WINNT _WIN32_WINNT_LONGHORN	// Windows Vista  
	//#define _WIN32_WINNT _WIN32_WINNT_WIN7		// Windows 7  
	//#define _WIN32_WINNT _WIN32_WINNT_WIN8		// Windows 8  
	//#define _WIN32_WINNT _WIN32_WINNT_WINBLUE		// Windows 8.1  
	//#define _WIN32_WINNT _WIN32_WINNT_WINTHRESHOLD// Windows 10  
	  #define _WIN32_WINNT _WIN32_WINNT_WIN10		// Windows 10
	#include <ws2tcpip.h>


	struct SockClient {
		enum struct States {
			Uninited,
			WSAInited,
			AddrinfoGotten,
			SockConnected
		};
		struct SockClientException {
			friend struct SockClient;
			private:
				enum struct ErrCodes {
					WSASTARTUP_FAILED,
					GETADDRINFO_FAILED,
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

	private:
		char* addr;
		unsigned short port;

		addrinfo hints;
		WSADATA wsaData;
		States SockState;
		addrinfo* addrInfo;
		SOCKET connectSocket;

		bool updateAddrInfo(int* result = 0);
	public:
		bool write(void* buffer, unsigned buffersize);
		bool read(void* buffer, unsigned buffersize, unsigned* bytesReceived = 0);
		void kill();
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