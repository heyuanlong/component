#ifndef REDISCLASS_H
#define REDISCLASS_H

#include <stdlib.h>
#include <string.h>

#include <hiredis/hiredis.h>


class redisClass
{
public:
	redisClass();
	~redisClass();
public:
	int init(const char *host,const int port ,const char * auth);
	int ping();
	int set(const char *key,const char* value);
	int get(const char *key,char *value);

private:
	redisContext 					*m_rc;
	/* data */
};


#endif