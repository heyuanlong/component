#include "socket_send.h"

int socket_send_tcp(int fd,char *buf,int size)
{
	if(size <= 0){
		return 0;
	}

	int flag = 0;
	int send_len = 0;
	while(size > send_len){
		flag = send(fd,buf + send_len,size - send_len,0);
		if( flag < 0 ){
			switch( errno ){
				case EINTR:
					continue;
				case EAGAIN:
					return send_len;
			}
			LOG_ERROR("socket_send_tcp error:%d",errno);
			return NET_ERR;
		}
		send_len += flag;
	}
	return send_len;
}

int socket_send_udp_addr(int fd, char *buf, int size,struct sockaddr_in addr)
{
	if(size <= 0 ){
		return NET_ARG;
	}
	int ret;
	ret = sendto(fd,buf,size,0,(struct sockaddr*)addr, sizeof(struct sockaddr_in));
	if(ret < 0){
		LOG_ERROR("socket_send_udp_addr to error:%d, addr:%s, port:%d, size:%d, fd:%d\n", errno, inet_ntoa(addr->sin_addr), addr->sin_port, size, fd);
	}
	return ret;
}
int socket_send_udp_ip_port(int fd, char *buf, int size,const char *ip,const int port)
{
	if(size <= 0 ){
		return NET_ARG;
	}
	struct sockaddr_in    addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr  =  inet_addr(ip);
	int ret;
	ret = sendto(fd,buf,size,0,(struct sockaddr*)addr, sizeof(struct sockaddr_in));
	if(ret < 0){
		LOG_ERROR("socket_send_udp_ip_port to error:%d, addr:%s, port:%d, size:%d, fd:%d\n", errno, inet_ntoa(addr->sin_addr), addr->sin_port, size, fd);
	}
	return ret;
}

