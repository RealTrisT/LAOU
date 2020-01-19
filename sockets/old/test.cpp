#include <cstring>
#include <cstdio>

#include "SockClient.h"

#define CRLF "\r\n"

int main(int argc, char const *argv[]){
	try{
		SockClient yeetos = SockClient("info.cern.ch", 80);
	

		const char* request =
			"GET / HTTP/1.1" CRLF
			"Host: info.cern.ch" CRLF
			"Connection: keep-alive" CRLF
			"Cache-Control: max-age=0" CRLF
			"Upgrade-Insecure-Requests: 1" CRLF
			"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Safari/537.36" CRLF
			"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9" CRLF
			"Referer: https://www.google.com/" CRLF
			"Accept-Encoding: gzip, deflate" CRLF
			"Accept-Language: en-US,en;q=0.9,pt;q=0.8,es;q=0.7,la;q=0.6" CRLF
			//"If-None-Match: \"40521bd2-286-4f1aadb3105c0\"" CRLF
			//"If-Modified-Since: Wed, 05 Feb 2014 16:00:31 GMT" CRLF
			"" CRLF
			"" CRLF
		;
		unsigned request_size = strlen(request);
		yeetos.write((char*)request, request_size);

		char read[10'000];
		unsigned read_size = 0;
		yeetos.read(read, 10'000, &read_size);
		for (unsigned i = 0; i < read_size; ++i){
			/*if(read[i] == '\r'){
				printf("CR");
				continue;
			}else if(read[i] == '\n'){
				printf("LF");
				continue;
			}*/
			putchar(read[i]);
		}

	}catch(SockClient::SockClientException e){
		printf("exception: %s [0x%X]\n", e.Message(), e.ErrorCode());
	}
	return 0;
}