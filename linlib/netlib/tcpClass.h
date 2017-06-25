#ifndef TCPCLASS_H
#define TCPCLASS_H

#include "socketClass.h"
#include "commonlib/commonMutex.h"
#include <map>

#define EPOLL_SIZE 1024

#define TCPCLASSRUNNING 1
#define TCPCLASSNOTRUN  0

class tcpClass
{
public:
	tcpClass();
	~tcpClass();

public:
	int add_tcp_listen(const char * ip,const int port,const gateway_handle_t *handle);

	int run();
	int close_fd(int fd);


private:
	int 						init_epoll();
	int 						go_run();
	int 						init_node(const int fd);
	fd_data_struct_t* 			find_node(const int fd);
	void 						del_node(const int fd);

private:
	commonMutex 						m_lock;
	std::map<int, gateway_handle_t*> 	m_lister_fd_map;
	std::map<int, fd_data_struct_t*> 	m_client_fd_map;
	int 								m_epoll;
	int 								m_online;
	int 								m_status;
};









#endif