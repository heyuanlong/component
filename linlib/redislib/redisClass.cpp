#include "redisClass.h"



redisClass::redisClass()
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
		redisReply *reply = (redisReply *)redisCommand(m_rc,authstr);
		if (reply->type == REDIS_REPLY_ERROR){
			//printf("Error: %s\n", reply->str);
			freeReplyObject(reply);
			return -1;
		}
		freeReplyObject(reply);
	}
	return 0;
}

redisContext* redisClass::getRedisCT()
{
	return m_rc;
}

int redisClass::ping()
{
	int ret = 0;
	redisReply *reply = (redisReply *)redisCommand(m_rc,"ping");
	if (reply->type == REDIS_REPLY_ERROR){
		//printf("Error: %s\n", reply->str);
		ret = -1;
	}
	freeReplyObject(reply);
	return ret;
}
int redisClass::set(const char *key,const char* value)
{
	int ret = 0;
	redisReply *reply = (redisReply *)redisCommand(m_rc,"set %s %s",key,value);
	if (reply->type == REDIS_REPLY_ERROR){
		//printf("Error: %s\n", reply->str);
		ret = -1;
	}
	freeReplyObject(reply);
	return ret;
}
int redisClass::get(const char *key,char *value)
{
	int ret = 0;
	redisReply *reply = (redisReply *)redisCommand(m_rc,"get %s",key);
	if (reply->type == REDIS_REPLY_ERROR){
		//printf("Error: %s\n", reply->str);
		ret = -1;
	}
	strcpy(value,reply->str);
	freeReplyObject(reply);
	return ret;
}

int redisClass::incr(const char *key,long long *value)
{
	int ret = 0;
	redisReply *reply = (redisReply *)redisCommand(m_rc,"incr %s",key);
	if (reply->type == REDIS_REPLY_ERROR){
		//printf("Error: %s\n", reply->str);
		ret = -1;
	}
	*value = reply->integer;
	freeReplyObject(reply);
	return ret;
}