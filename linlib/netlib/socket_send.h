#ifndef SOCKETSENT_H
#define SOCKETSENT_H

#include "errno.h"
#include "socket_base.h"
#include "loglib/log.h"


int socket_send_tcp(int fd,char *buf,int size);

int socket_send_udp_addr(int fd,const  char *buf,const  int size,struct sockaddr_in addr);
int socket_send_udp_ip_port(int fd,const  char *buf,const  int size,const char *ip,const int port);

#endif