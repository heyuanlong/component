#ifndef POLL_SOCKET_EPOLL__H
#define POLL_SOCKET_EPOLL__H

#include <netdb.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>


#include "socket_base.h"
#include "loglib/log.h"


static bool  socket_epoll_invalid(int efd) {
	return efd == -1;
}

static int socket_epoll_create() {
	return epoll_create(EPOLL_SIZE);
}

static void socket_epoll_release(int efd) {
	close(efd);
}

static int  socket_epoll_add(int efd, int sock, int fd) {
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = fd;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, sock, &ev) == -1) {
		LOG_ERROR("socket_epoll_add error:%d",errno);
		return NET_ERR;
	}
	return NET_OK;
}

static int  socket_epoll_del(int efd, int sock) {
	if (epoll_ctl(efd, EPOLL_CTL_DEL, sock , NULL) == -1) {
		LOG_ERROR("socket_epoll_del error:%d",errno);
		return NET_ERR;
	}
	return NET_OK;
}

static int  socket_epoll_write(int efd, int sock, int fd, bool enable) {
	struct epoll_event ev;
	ev.events = EPOLLIN | (enable ? EPOLLOUT : 0);
	ev.data.fd = fd;
	if (epoll_ctl(efd, EPOLL_CTL_MOD, sock, &ev) == -1) {
		LOG_ERROR("socket_epoll_write error:%d",errno);
		return NET_ERR;
	}
	return NET_OK;
}

static int  socket_epoll_wait(int efd, struct epoll_event *e, int max,int timeout) {

	while(1){
		int n = epoll_wait(efd , e, max, timeout);
		if (n < 0){
			if(errno == EINTR){
				continue; 
			}
			return NET_ERR;
		}
		return n;
	}
}

#endif
