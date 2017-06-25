#ifndef SOCKETCLASS_H
#define SOCKETCLASS_H






typedef struct gateway_handle
{
	void (*open) ();
	void (*connect) (const int fd,const char * addr);
	void (*disconnect) (const int fd);
	void (*error) (const int fd,const char * msg);
	void (*message) (const int fd,const void * msg,const int size);
}gateway_handle_t;

typedef struct fd_data_struct{
	void 							*porigin;
	void 							*ptail;
    void 							*pdata;
	int 							size;
}fd_data_struct_t;


int 			socketinit(const char *ip,int port);
int 			set_socket_nonblock(int fd);

#endif
