#include "socketClass.h"






int socketinit(const char *ip,int port)
{
	int fds = socket(AF_INET,SOCK_STREAM,0);
	if(fds == -1){
		return -1;
	}
	struct sockaddr_in serverAddr;
	bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr  =  inet_addr(ip);

	int val=1;
    if (0 != setsockopt(fds,SOL_SOCKET,SO_REUSEADDR,(const void*)&val,sizeof(int))){
		return -1;
    }
    if (set_socket_nonblock(fds) < 0){
    	return -1;
    }

	if(bind(fds,(struct sockaddr *)&serverAddr,sizeof(struct sockaddr)) == -1){
		return -1;
	}
	if (listen(fds,128) == -1)
	{
		return -1;
	}
	return fds;
}

int set_socket_nonblock(int fd)
{
   return fcntl(fd,F_SETFL,fcntl(fd,F_GETFL,0) | O_NONBLOCK);
}
