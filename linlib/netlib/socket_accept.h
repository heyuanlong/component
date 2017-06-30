#ifndef SOCKETACCEPT_H
#define SOCKETACCEPT_H



#include "errno.h"
#include "loglib/log.h"

#include "socket_base.h"
#include "socket_epoll.h"
#include "socket_help.h"

#include "tcpUdpClass.h"


int socket_accept_tcp(int fd, tcpUdpClass	*ptp);
int socket_accept_udp(int fd, tcpUdpClass	*ptp);




#endif