#ifndef SOCKETRECV_H
#define SOCKETRECV_H





#include "errno.h"
#include "socket_base.h"
#include "loglib/log.h"

int socket_recv_tcp(int fd,fd_data_struct_t* n );


#endif