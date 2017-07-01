#ifndef SOCKETBASE_H
#define SOCKETBASE_H

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>



#define EPOLL_SIZE 2048
#define EPOLLTIMEOUT 500
#define MSGMAXSIZE 65535


#define NET_OK 				0
#define NET_AGAIN 			-100
#define NET_ERR				-1
#define NET_ARG				-2
#define NET_EPOLL_ERR		-3
#define NET_CLOSED			-4


typedef struct header
{
	int size;
	int cmd;
}header_t;




typedef struct gateway_handle
{
	void (*open) ();
	void (*connect) (const int fd,const char * addr);
	void (*disconnect) (const int fd);
	void (*error) (const int fd,const char * msg);
	void (*message) (const int fd,const void * msg,const int cmd,const int size);
}gateway_handle_t;

typedef struct gateway_udp_handle
{
	void (*open) ();
	void (*error) (const int fd,const char * msg );
	void (*message) (const int fd,const void * msg,const int size,struct sockaddr_in client_addr,socklen_t client_len);
}gateway_udp_handle_t;




typedef struct fd_data_struct{
	void 							*r_porigin;
	void 							*r_ptail;
    void 							*r_pdata;
	int 							r_size;

	void 							*s_porigin;
	void 							*s_ptail;
    void 							*s_pdata;
	int 							s_size;

	gateway_handle_t				*handle;
}fd_data_struct_t;





#endif