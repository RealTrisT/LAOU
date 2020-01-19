#include "SockClient.h"


#ifdef _WIN32
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////      W      I      N      D      O      W      S      ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#elif __linux__
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////      L      I      N      U      X      //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//g++ test.cpp SSLSockClient.cpp SockClient.cpp -I/usr/include/openssl -lssl -lcrypto

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

struct SSLSockClient : public SockClient{

	enum struct States{
		Uninited,
		ctxCreated,
		instanceCreated,
		fdSet,
		ready,
	};

	struct SSLSockClientException {
		friend struct SSLSockClient;
		private:
			enum struct ErrCodes {
				SSLSOCK_SSLUNINITED,
				SSLSOCKCTXINIT_FAILED,
				SSLSOCKSSLINIT_FAILED,
				SSLSOCKFDATTRB_FAILED,
				SSLSOCKHANDSHAKE_FAILED,
				SSLSOCKRECV_FAILED,
				SSLSOCKSEND_FAILED,
				SSLSOCKUNINITED,
				ERRAMOUNT
			};
			ErrCodes errCode;
			unsigned additionalErrcode;
			static const char* errDescriptors[(unsigned)ErrCodes::ERRAMOUNT];

		public:
			SSLSockClientException(ErrCodes rc, unsigned arc) : errCode(rc), additionalErrcode(arc) {}
			const char* Message() { return errDescriptors[(unsigned char)errCode]; }
			//const char* Message = errDescriptors[(unsigned)errCode];
			unsigned ErrorCode() { return additionalErrcode; }
	};

	//TODO: maybe add support for something other than TLS
	SSLSockClient(const char* paddr, unsigned short pport);

	bool read(void* buffer, unsigned buffersize, unsigned* bytesReceived = 0);

	bool write(void* buffer, unsigned buffersize);

	void kill();

	~SSLSockClient();

	static void InitOpenSSL();
	static void TermOpenSSL();

private:
	States state;

	SSL_CTX *sslctx;
	SSL *cSSL;

	static bool openSSLInited;
};

#else
	#error Sockets Not Implemented For The Current Platform
#endif