#ifndef SOCKETHELP_H
#define SOCKETHELP_H




#include "errno.h"
#include "socket_base.h"
#include "loglib/log.h"



int socket_help_init_tcp(const char *ip,const int port);
int socket_help_init_udp(const char *ip,const int port);

int socket_help_set_nonblock(int fd);


#endif