#include <stdio.h>
#include <string.h>
#include "SSLSockClient.h"
#include "../http/http.h"
#include "../json/json.h"

const char* req1 =
	"POST /login HTTP/1.1\r\n"
	"Host: api.thetvdb.com\r\n"
	"Content-Type: application/json\r\n"
	"User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:47.0) Gecko/20100101 Firefox/47.0\r\n"
	"Content-Length: 89\r\n"
	"\r\n"
	"{\"apikey\":\"CDCZTRC8SB7V3ZT9\",\"username\":\"century69errors7c\",\"userkey\":\"XJUYUNO5917N47L1\"}"
;

int main(int argc, char const *argv[]){
	char buffer[40000] = {0};
	SSLSockClient::InitOpenSSL();

	try{
		SSLSockClient s = SSLSockClient("api.thetvdb.com", 443);
		s.write((char*)req1, strlen(req1));
		s.read(buffer, 40000);

		ResponseHttp reqInfo;
		if(HttpParseResponse(buffer, &reqInfo)){
			JSON::Element* parsed = JSON::parse(reqInfo.contentStart);
			puts((*parsed)["token"].getString().c_str());
		}else{puts("merda no parse de http");}

	}catch (SSLSockClient::SSLSockClientException e){
		printf("[%u]%s\n", e.ErrorCode(), e.Message());
	}catch (SockClient::SockClientException e){
		printf("[%u]%s\n", e.ErrorCode(), e.Message());
	}

	SSLSockClient::TermOpenSSL();
	return 0;
}