#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "netlib/tcpUdpClass.h"
#include "netlib/socket_base.h"


#include "base.h"



tcpUdpClass tp;


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
void test_message(const int fd,const void * msg, const int cmd,const int size)
{
	msg_t *que = (msg_t*)(msg);
	printf(" fd:%d,MSG:useid:%d,size:%d",fd,que->userid,size);
	write(STDOUT_FILENO,que->data,que->dataSize);
	printf("\n");

	for (int i = 0; i < 200; ++i)
	{
		//tp.send_tcp(fd,"111111111111111111111111",sizeof("111111111111111111111111"));
	}
	
}


void test_udp_open()
{
	printf("test_open\n");
}
void test_udp_error(const int fd,const char * msg)
{
	printf("test_udp_error fd:%d,msg:%s\n",fd, msg);
}
void test_udp_message(const int fd,const void * msg,const int size,struct sockaddr_in client_addr,socklen_t client_len)
{
	char recvbuf[MSGMAXSIZE + 1];
	memcpy(recvbuf,msg,size);
	recvbuf[size]='\0';
	printf("socket %d recv from : %s : %d message: %s ，%d bytes\n",fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), recvbuf, size);

	tp.send_udp_addr(fd,(const char *)msg, size, client_addr, client_len);
}


int main(int argc, char const *argv[])
{
	gateway_handle_t handle;
	handle.open = test_open;
	handle.connect = test_connect;
	handle.disconnect = test_disconnect;
	handle.error = test_error;
	handle.message = test_message;

	gateway_udp_handle_t udphandle;
	udphandle.open = test_udp_open;
	udphandle.error = test_udp_error;
	udphandle.message = test_udp_message;



	
	tp.add_tcp_listen("0.0.0.0",6001,&handle);
	tp.add_tcp_listen("0.0.0.0",6002,&handle);
	
	tp.add_udp_listen("0.0.0.0",6003,&udphandle);
	tp.add_udp_listen("0.0.0.0",6004,&udphandle);

	for (;;)
	{
		tp.run();
	}
	

	return 0;
}

