#include "redisClass.h"



redisClass::redisClass(const char *host,const int port ,const char * auth)
{
	m_rc = NULL;
}
redisClass::~redisClass()
{

}

int redisClass::init(const char *host,const int port ,const char * auth)
{
	struct timeval timeout = {1,500000}; //1.5second
	m_rc = redisConnectWithTimeout(host,port,timeout);
	if ( m_rc== NULL || m_rc->err){
		if(m_rc){
			//printf("Connect error :%s\n",m_rc->errstr );
			redisFree(m_rc);
			m_rc = NULL;
			return -1;
		}else{
			//printf("Connection error: can't allocate redis context\n");
			return -1;
		}
		return -1;
	}
	
	if (strlen(auth) > 0){
		char authstr[120];
		sprintf(authstr,"auth %s",auth);
		redisReply *reply = redisCommand(m_rc,authstr);
		if (reply->type == REDIS_REPLY_ERROR){
			//printf("Error: %s\n", reply->str);
			return -1;
		}
	}
	return 0;
}