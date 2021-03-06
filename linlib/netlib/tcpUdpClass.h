#ifndef TCPUDPCLASS_H
#define TCPUDPCLASS_H



#include "socket_base.h"
#include "socket_help.h"
#include "socket_epoll.h"
#include "socket_recv.h"
#include "socket_send.h"
#include "socket_accept.h"


#include <stdlib.h>
#include <pthread.h>
#include <map>
#include <error.h>
#include <stdio.h>


#define HEADERSIZE 2

#define TCPCLASSRUNNING 1
#define TCPCLASSNOTRUN  0

class tcpUdpClass
{
public:
	tcpUdpClass();
	~tcpUdpClass();

public:
	int 		add_tcp_listen(const char * ip,const int port,gateway_handle_t *handle);
	int 		add_udp_listen(const char * ip,const int port,gateway_udp_handle_t *handle);

	void 		send_udp_addr(int fd,const char *buf,const int size,struct sockaddr_in client_addr,socklen_t client_len);
	void 		send_udp_ip_port(int fd,const char *buf,const int size,const char *ip,const int port);

	int 		send_tcp(int fd,char *buf,int size);
	int 		run();

	int 		close_fd(int fd);



public:
	int 						go_run();

	bool 						is_lister_fd(int fd);
	bool 						is_udp_fd(int fd);

	int 						deal_lister_fd(int fd);
	void 						deal_udp_recv_fd(int fd);
	void 						deal_client_recv_fd(int fd);
	int 						deal_client_send_fd(int fd);

	int 						add_to_send_buf(fd_data_struct_t* n,char *buf,int size);
	void						pclose_fd(int fd);

	int 						init_client_node(const int fd,gateway_handle_t* handle);
	fd_data_struct_t* 			find_client_node(const int fd);
	void 						del_client_node(const int fd);

public:
	std::map<int, gateway_handle_t*> 		m_lister_fd_map;
	std::map<int, fd_data_struct_t*> 		m_client_fd_map;
	std::map<int, gateway_udp_handle_t*> 	m_udp_fd_map;

public:
	int 								m_epoll;
	int 								m_online;
	struct epoll_event 					m_ready_event[EPOLL_SIZE];
	
};





#endif