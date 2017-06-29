#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "redislib/redisClass.h"

int main(int argc, char const *argv[])
{


	redisClass rc;
	int ret;
	long long incr;
	rc.init("127.0.0.1",6379,"FDFDfdi4k25e@sf");

	ret = rc.ping();
	printf("ping:%d\n",ret);

	ret = rc.set("key11","value11");
	printf("set:%d\n",ret);
	
	char value[50];
	ret = rc.get("key11",value);
	printf("set:%d,%s\n",ret,value);

	ret = rc.incr("incr11",&incr);
	printf("incr:%d,%lld\n",ret,incr);	
	ret = rc.incr("incr11",&incr);
	printf("incr:%d,%lld\n",ret,incr);	

	return 0;
}

