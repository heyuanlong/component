#ifndef SOCKETCLASS_H
#define SOCKETCLASS_H


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

#define MSGMAXSIZE 65535
typedef struct header
{
	int size;
	int cmd;
}header_t;



typedef struct msg_string
{
	header_t 	head;
	char	 	data[MSGMAXSIZE];
}msg_string_t;


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


int 			socketinit(const char *ip,int port);
int 			socketinit_udp(const char *ip,int port);
int 			set_socket_nonblock(int fd);

#endif
