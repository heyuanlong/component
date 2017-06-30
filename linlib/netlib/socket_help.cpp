#include <socket_help.h>

int socket_help_init_tcp(const char *ip,const int port)
{
	int fds = socket(AF_INET,SOCK_STREAM,0);
	if(fds == -1){
		LOG_ERROR("socket_help_init_tcp error:%d",errno);
		return NET_ERR;
	}
	int val=1;
    if (0 != setsockopt(fds,SOL_SOCKET,SO_REUSEADDR,(const void*)&val,sizeof(int))){
		LOG_ERROR("setsockopt tcp SO_REUSEADDR error:%d",errno);
		return NET_ERR;
    }

    socket_help_set_nonblock(fds);

	struct sockaddr_in serverAddr;
	bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr  =  inet_addr(ip);
	if(bind(fds,(struct sockaddr *)&serverAddr,sizeof(struct sockaddr)) == -1){
		LOG_ERROR("bind tcp error:%d",errno);
		return NET_ERR;
	}
	if (listen(fds,128) == -1)
	{
		LOG_ERROR("listen error:%d",errno);
		return NET_ERR;
	}
	return fds;
}

int socket_help_init_udp(const char *ip,const int port)
{
	int fds = socket(AF_INET,SOCK_DGRAM,0);
	if(fds == -1){
		LOG_ERROR("socket_help_init_udp error:%d",errno);
		return NET_ERR;
	}
	int val=1;
    if (0 != setsockopt(fds,SOL_SOCKET,SO_REUSEADDR,(const void*)&val,sizeof(int))){
		LOG_ERROR("setsockopt udp SO_REUSEADDR error:%d",errno);
		return NET_ERR;
    }
    socket_help_set_nonblock(fds);

	struct sockaddr_in serverAddr;
	bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr  =  inet_addr(ip);//htonl(INADDR_ANY);

	if(bind(fds,(struct sockaddr *)&serverAddr,sizeof(struct sockaddr)) == -1){
		LOG_ERROR("bind udp error:%d",errno);
		return NET_ERR;
	}
	return fds;
}



int socket_help_set_nonblock(int fd)
{
   return fcntl(fd,F_SETFL,fcntl(fd,F_GETFL,0) | O_NONBLOCK);
}
