#ifndef TCPUDPCLASS_H
#define TCPUDPCLASS_H



#include "socketClass.h"
#include "socketEpoll.h"


#include <stdlib.h>
#include <pthread.h>
#include <map>
#include <error.h>
#include <stdio.h>

#define EPOLL_SIZE 1024
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
	void 		send_udp(int fd,char *buf,int size,struct sockaddr_in client_addr,socklen_t client_len);
	void 		send_tcp(int fd,char *buf,int size);
	int 		run();
	int 		close_fd(int fd);


private:
	int 						init_epoll();
	int 						go_run();

	bool 						is_lister_fd(int fd);
	bool 						is_udp_fd(int fd);
	void 						deal_lister_fd(int fd);
	void 						deal_udp_recv_fd(int fd);
	void 						deal_client_recv_fd(int fd);
	void 						deal_udp_send_fd(int fd);
	void 						deal_client_send_fd(int fd);
	int 						add_to_send_buf(fd_data_struct_t* n,char *buf,int size);
	int 						net_send(int fd,char *buf,int size);

	void						pclose_fd(int fd);

	int 						init_client_node(const int fd,gateway_handle_t* handle);
	fd_data_struct_t* 			find_client_node(const int fd);
	void 						del_client_node(const int fd);

private:
	std::map<int, gateway_handle_t*> 		m_lister_fd_map;
	std::map<int, fd_data_struct_t*> 		m_client_fd_map;
	std::map<int, gateway_udp_handle_t*> 	m_udp_fd_map;
	
	int 									m_status;

private:
	int 								m_epoll;
	int 								m_online;
	struct epoll_event 					m_ready_event[EPOLL_SIZE];
	struct epoll_event 					m_temp_event;

	char 								*m_udp_buf[MSGMAXSIZE];
};





#endif