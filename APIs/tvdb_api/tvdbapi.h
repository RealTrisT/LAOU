#include <string.h>
#include <stdlib.h>

#include <string>

#include "../../sockets/SSLSockClient.h"
#include "../../http/http.h"
#include "../../json/json.h"

//#define Interval24Hrs 86400

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////CREDENTIALS///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct TVDBcreds{
	TVDBcreds(const char* API_KEY, const char* username, const char* userID){
		unsigned API_KEY_len = strlen(API_KEY)+1;
		unsigned username_len = strlen(username)+1;
		unsigned userID_len = strlen(userID)+1;
		this->apikey = new char[API_KEY_len];
		this->username = new char[username_len];
		this->userID = new char[userID_len];
		memcpy(this->apikey, API_KEY, API_KEY_len);
		memcpy(this->username, username, username_len);
		memcpy(this->userID, userID, userID_len);
	}

	TVDBcreds(){
		this->apikey = 0; this->username = 0; this->userID = 0;
	}

	static bool BuildFromFile(FILE* fd, TVDBcreds* crds){
		unsigned tempLen = 0;

		if(fread(&tempLen, sizeof(unsigned), 1, fd) != 1)return false;
		crds->apikey = new char[tempLen];
		if(fread(crds->apikey, 1, tempLen, fd) != tempLen)goto TVDB_fc_fail_first;

		if(fread(&tempLen, sizeof(unsigned), 1, fd) != 1)goto TVDB_fc_fail_first;
		crds->username = new char[tempLen];
		if(fread(crds->username, 1, tempLen, fd) != tempLen)goto TVDB_fc_fail_second;

		if(fread(&tempLen, sizeof(unsigned), 1, fd) != 1)goto TVDB_fc_fail_second;
		crds->userID = new char[tempLen];
		if(fread(crds->userID, 1, tempLen, fd) != tempLen)goto TVDB_fc_fail_third;

	TVDB_fc_fail_third:	
		delete[] crds->userID;
	TVDB_fc_fail_second:
		delete[] crds->username;
	TVDB_fc_fail_first:
		delete[] crds->apikey;
		return false;
	}

	~TVDBcreds(){
		delete[] this->apikey;
		delete[] this->username;
		delete[] this->userID;
	}

	bool save(FILE* fd){
		unsigned API_KEY_len = strlen(this->apikey);
		unsigned username_len = strlen(this->username);
		unsigned userID_len = strlen(this->userID);
		if(fwrite(&API_KEY_len, sizeof(unsigned), 1, fd) != 1)			return false;
		if(fwrite(this->apikey, 1, API_KEY_len, fd) != API_KEY_len)		return false;

		if(fwrite(&username_len, sizeof(unsigned), 1, fd) != 1)			return false;
		if(fwrite(this->username, 1, username_len, fd) != username_len)	return false;

		if(fwrite(&userID_len, sizeof(unsigned), 1, fd) != 1)			return false;
		if(fwrite(this->userID, 1, userID_len, fd) != userID_len)		return false;

		return true;
	}

	char* apikey;
	char* username;
	char* userID;
};




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////TOKEN//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct TVDBtoken{
	TVDBtoken(char* tkn){
		if(!tkn){this->token = 0;this->issueTime = 0;}
		setTime = time(0);
		unsigned tokensize = strlen(tkn)+1;
		this->token = new char[tokensize];
		memcpy(this->token, tkn, tokensize);
	}

	TVDBtoken(){this->token = 0;}

	static bool BuildFromFile(FILE* fd, TVDBtoken* tkn){
		unsigned totalsize = 0;
		unsigned tokensize = 0;
		if(fread(&totalsize, sizeof(unsigned), 1, fd) != 1)return false;		//if couldn't read the total length
		tokensize = totalsize - sizeof(unsigned) - sizeof(time_t);				//calculate tokensize
		if(totalsize <= sizeof(unsigned) + sizeof(time_t)) return false;		//if theres not enough space for the token
		if(fread(&tkn->issueTime, sizeof(time_t), 1, fd), != 1)return false;	//if couldn't read 
		tkn->token = new char[totalsize - sizeof(unsigned) - sizeof(time_t)];	//allocate size
		if(fread(tkn->token, 1, tokensize, fd) != tokensize)
			{delete[] tkn->token; return false;}
		return true;
	}


	~TVDBtoken(){
		if(this->token)delete[] this->token;
	}

	bool save(FILE* fd){
		if(!token)return false;

		unsigned tokensize = strlen(this->token)+1;
		unsigned totalsize = sizeof(unsigned) + sizeof(time_t) + tokensize;

		unsigned char* buffer = new unsigned char[totalsize];
		
		*(unsigned*)buffer = totalsize;
		*(time_t*)(&((unsigned*)buffer)[1]) = this->issueTime;
		memcpy(&buffer[sizeof(unsigned) + sizeof(time_t)], this->token, tokensize);

		if(fwrite(buffer, 1, totalsize, fd) != totalsize){delete[] buffer; return false;}
		delete[] buffer;
		return true;
	}

	bool getFromLogin(SSLSockClient* s, TVDBcreds* creds){
		try{
			//------------------------------------------------------------------------------------------------------------BUILD QUERY
			std::string json = (std::string)"{\"apikey\":\"" + creds->apikey + "\"";
			if(creds->username && creds->userID)
				json += ", \"username\":\"" + creds->username + "\", \"userkey\":\"" + creds->userID + "\"";
			json += "}";

			std::string query = "POST /login HTTP/1.1\r\n"
								"Host: api.thetvdb.com\r\n"
								"Content-Type: application/json\r\n"
								"Connection: Keep-Alive\r\n"
								"Content-Length: " + std::to_string(json.length()) + 
								"\r\n\r\n" + json;

			//------------------------------------------------------------------------------------------------------------SEND QUERY
			if(!s->write(query.c_str(), query.length()))return false;

			//------------------------------------------------------------------------------------------------------------READ QUERY RESULT
			char buffr[4096]; unsigned amountRead = 0;
			bool readResult = false
			ResponseHttp resp;

			HttpResponse resp = HttpResponse();

			while(readResult = s->read(buffr, 4096, &amountRead)){
				resp.add(buffr, amountRead);
				if(resp.isBad())return false;
				if(resp.isOver())break;
			}if(!resp.isOver())return false;

			//------------------------------------------------------------------------------------------------------------INTERPRET JSON
			std::string jsonstr = std::string(resp.content.getBuffer(), resp.content.getSize());
			JSON::Instance jsn = JSON::Instance(jsonstr.c_str());
			jsonstr = jsonstr["token"].getString();						//repurpose


			//------------------------------------------------------------------------------------------------------------BUILD OURSELVES
			this->issueTime = time(0);
			if(this->token)delete[] this->token;
			size_t strsize = jsonstr.size();
			this->token = new char[strsize+1];
			memcpy(this->token, jsonstr.c_str(), strsize);
			this->token[strsize] = '\0';

			//------------------------------------------------------------------------------------------------------------FIN
		}catch (SSLSockClient::SSLSockClientException e){
			//printf("[%u]%s\n", e.ErrorCode(), e.Message());
			return false;
		}catch (SockClient::SockClientException e){
			//printf("[%u]%s\n", e.ErrorCode(), e.Message());
			return false;
		}catch (const char* e){
			//printf("failed to access json member\n");
			return false;
		}
	}

	bool isValid(unsigned maxInterval){return (time(0) - this->issueTime) >= maxInterval;}
	char* token;
	time_t issueTime;
};





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////SESSION//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct TVDBsession{
	static bool Create(TVDBsession* tvs, TVDBtoken* tkn, TVDBcreds* creds){
		try{
			tvs->sock = new SSLSockClient("api.thetvdb.com", 443);
		}catch (SSLSockClient::SSLSockClientException e){
			printf("[%u]%s\n", e.ErrorCode(), e.Message());	//TODO chang dis
			return false;
		}catch (SockClient::SockClientException e){
			printf("[%u]%s\n", e.ErrorCode(), e.Message());	//TODO: chang dis
			return false;
		}

		if(!tkn->isValid(86400) && !tkn->getFromLogin(tvs->sock, creds)){
			delete tvs->sock;
			return false;
		}

		tvs->token = tkn;
		tvs->credentials = creds;
		return true;
	}

	bool GetReq(char* loc, std::string* result){
		std::string req = (std::string)"GET " + loc + " HTTP/1.1\r\n"
						  "Authorization: Bearer " + token->token + "\r\n"
						  "Host: api.thetvdb.com\r\n"
						  "Accept-Encoding: chunked"
						  "\r\n"

		//------------------------------------------------------------------------------------------------------------SEND QUERY
		if(!s->write(query.c_str(), query.length()))return false;

		//------------------------------------------------------------------------------------------------------------READ QUERY RESULT
		char buffr[4096]; unsigned amountRead = 0;
		bool readResult = false
		ResponseHttp resp;

		HttpResponse resp = HttpResponse();

		while(readResult = s->read(buffr, 4096, &amountRead)){
			resp.add(buffr, amountRead);
			if(resp.isBad())return false;
			if(resp.isOver())break;
		}if(!resp.isOver())return false;

		//------------------------------------------------------------------------------------------------------------INTERPRET JSON
		*result = std::string(resp.content.getBuffer(), resp.content.getSize());
	}

	bool PostReq(char* loc, char* data, unsigned datalen){

	}

	SSLSockClient* sock;
	TVDBtoken* token;
	TVDBcreds* credentials;
};