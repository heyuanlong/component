#include "stdio.h"
#include "stdlib.h"
#include "netlib/tcpClass.h"
#include "netlib/socketClass.h"

#include "base.h"

void test_open()
{
	printf("test_open\n");
}
void test_connect(const int fd,const char * addr)
{
	printf("test_connect fd:%d,addr:%s\n",fd, addr);
}
void test_disconnect (const int fd)
{
	printf("test_disconnect fd:%d\n", fd);
}
void test_error(const int fd,const char * msg)
{
	printf("test_error fd:%d,msg:%s\n",fd, msg);
}
void test_message(const int fd,const void * msg,const int size)
{
	msg_t *que = (msg_t*)(msg);
	printf(" fd:%d,MSG:useid:%d,size:%d",fd,que->userid,size);
	write(STDOUT_FILENO,que->data,que->dataSize);
	printf("\n");

}


int main(int argc, char const *argv[])
{
	gateway_handle_t handle;
	handle.open = test_open;
	handle.connect = test_connect;
	handle.disconnect = test_disconnect;
	handle.error = test_error;
	handle.message = test_message;

	tcpClass tp;
	tp.add_tcp_listen("0.0.0.0",6001,&handle);
	tp.run();

	return 0;
}

