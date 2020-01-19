#include "SSLSockClient.h"



#ifdef _WIN32
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////      W      I      N      D      O      W      S      ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



bool openSSLInited = false; 

void InitOpenSSL(){
	if(!openSSLInited){
		SSL_load_error_strings();
		SSL_library_init();
		OpenSSL_add_all_algorithms();
		openSSLInited = true;
	}
}

void TermOpenSSL(){
	if(openSSLInited){
		ERR_free_strings();
		EVP_cleanup();
		openSSLInited = false;
	}
}





bool SSLSockClient::Build(const char* addr, unsigned short port){
	if(!this->SockClient::Build(addr, port))return false;

	if(!openSSLInited)goto fail;

	this->sslctx = SSL_CTX_new(TLS_client_method());
	if(!this->sslctx)goto fail;

	this->cSSL = SSL_new(this->sslctx);
	if(!this->cSSL){	
		goto fail_after_context;
	}

	if(!SSL_set_fd(this->cSSL, this->connectSocket)){
		goto fail_after_sslnew;
	}

	if(SSL_connect(this->cSSL) <= 0){
		goto fail_after_sslnew;
	}

	return true;

	//---------------------------------------------
	
	fail_after_sslnew:
	SSL_free(this->cSSL);

	fail_after_context:
	SSL_CTX_free(this->sslctx);

	fail:
	this->SockClient::Kill();
	return false;
}

bool SSLSockClient::Read(void* buffer, unsigned buffersize, unsigned* bytes_read){
	int result = SSL_read(this->cSSL, (char *)buffer, buffersize);
	if(result > 0){
		if(bytes_read)*bytes_read = result;
		return true;
	}else if(!result){
		return false;
	}else{
		return false;
	}
}

bool SSLSockClient::Write(void* buffer, unsigned buffersize, unsigned* bytes_written){
	int result = SSL_write(this->cSSL, buffer, buffersize);
	if(result > 0){
		if(bytes_written)*bytes_written = result;
		return true;
	}else if(!result){
		return false;
	}else{
		return false;
	}
}

void SSLSockClient::Kill(){
	SSL_shutdown(this->cSSL);
	SSL_free(this->cSSL);
	SSL_CTX_free(this->sslctx);
	this->SockClient::Kill();
}


#elif __linux__
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////      L      I      N      U      X      //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SSLSockClient::openSSLInited = false;

SSLSockClient::SSLSockClient(const char* paddr, unsigned short pport) : SockClient(paddr, pport){
	if(!SSLSockClient::openSSLInited)throw SSLSockClientException(SSLSockClientException::ErrCodes::SSLSOCK_SSLUNINITED, 0);
	this->sslctx = SSL_CTX_new(TLS_client_method());
	if(!this->sslctx){
		this->state = States::Uninited;
		this->SockClient::kill();
		throw SSLSockClientException(SSLSockClientException::ErrCodes::SSLSOCKCTXINIT_FAILED, ERR_peek_last_error());
	}this->state = States::ctxCreated;

	this->cSSL = SSL_new(this->sslctx);
	if(!this->cSSL){
		SSL_CTX_free(this->sslctx);
		this->state = States::Uninited;
		this->SockClient::kill();
		throw SSLSockClientException(SSLSockClientException::ErrCodes::SSLSOCKSSLINIT_FAILED, ERR_peek_last_error());
	}this->state = States::instanceCreated;

	int setfd_result = 0;
	if(!(setfd_result = SSL_set_fd(cSSL, this->sock))){
		SSL_CTX_free(this->sslctx);
		SSL_free(this->cSSL);
		this->state = States::Uninited;
		this->SockClient::kill();
		throw SSLSockClientException(SSLSockClientException::ErrCodes::SSLSOCKFDATTRB_FAILED, setfd_result);
	}this->state = States::fdSet;

	int ssl_err = SSL_connect(cSSL);
	if(ssl_err <= 0){
		SSL_free(this->cSSL);
		SSL_CTX_free(this->sslctx);
		this->state = States::Uninited;
		this->SockClient::kill();
		throw SSLSockClientException(SSLSockClientException::ErrCodes::SSLSOCKHANDSHAKE_FAILED, SSL_get_error(this->cSSL, ssl_err));
	}this->state = States::ready;
}

bool SSLSockClient::read(void* buffer, unsigned buffersize, unsigned* bytesReceived){
	if(this->state == States::Uninited)throw SSLSockClientException(SSLSockClientException::ErrCodes::SSLSOCKUNINITED, 0);
	int result = SSL_read(cSSL, (char *)buffer, buffersize);
	if(result > 0){
		if(bytesReceived)*bytesReceived = result;
		return true;
	}else if(!result){
		return false;
	}else{
		throw SSLSockClientException(SSLSockClientException::ErrCodes::SSLSOCKRECV_FAILED, result);
	}
}

bool SSLSockClient::write(void* buffer, unsigned buffersize){
	if(this->state == States::Uninited)throw SSLSockClientException(SSLSockClientException::ErrCodes::SSLSOCKUNINITED, 0);
	int result = SSL_write(this->cSSL, buffer, buffersize);
	if(!result || result != buffersize){
		return false;
	}else if(result > 0){
		return true;
	}else{
		throw SSLSockClientException(SSLSockClientException::ErrCodes::SSLSOCKSEND_FAILED, result);
	}
}

void SSLSockClient::kill(){
	if(this->state == States::Uninited)throw SSLSockClientException(SSLSockClientException::ErrCodes::SSLSOCKUNINITED, 0);
	SSL_shutdown(this->cSSL);
	SSL_free(this->cSSL);
	SSL_CTX_free(this->sslctx);
	this->state = States::Uninited;
	this->SockClient::kill();
}

SSLSockClient::~SSLSockClient(){
	if(this->state != States::Uninited)this->kill();
}

void SSLSockClient::InitOpenSSL(){
	if(!SSLSockClient::openSSLInited){
		SSL_load_error_strings();
		SSL_library_init();
		OpenSSL_add_all_algorithms();
		SSLSockClient::openSSLInited = true;
	}
}

void SSLSockClient::TermOpenSSL(){
	if(SSLSockClient::openSSLInited){
		ERR_free_strings();
		EVP_cleanup();
		SSLSockClient::openSSLInited = false;
	}
}

const char* SSLSockClient::SSLSockClientException::errDescriptors[(unsigned)ErrCodes::ERRAMOUNT] = {
	"Must Init Openssl First With SSLSockClient::InitOpenSSL",
	"Failed to Init the Context",
	"Failed to Init the SSL Instance",
	"Failed to Attribute the Socket's File Descriptor to the SSL Instance",
	"Failed to Handshake",
	"Failed to Receive",
	"Failed to Send",
	"Operation Attempt on Uninitiated Socket"
};

#else
	#error Sockets Not Implemented For The Current Platform
#endif