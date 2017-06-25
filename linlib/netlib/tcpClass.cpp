
#include "tcpClass.h"


tcpClass::tcpClass()
{
	m_epoll			= -1;
	m_online		= 0;
	m_status		= TCPCLASSNOTRUN;
}

tcpClass::~tcpClass()
{

}


int tcpClass::add_tcp_listen(const char * ip,const int port,const gateway_handle_t *handle)
{

	int fds = socketinit(ip,port);
	if (fds < 0){
		return -1;
	}
	m_lock.lock();
	m_lister_fd_map[fds] =  handle;
	if (m_status == TCPCLASSRUNNING){
		//
	}
	m_lock.unlock();
}

int tcpClass::run()
{
	m_status = TCPCLASSRUNNING;
	init_epoll();
	go_run();
}

int tcpClass::init_epoll()
{
	m_epoll = epoll_create(EPOLL_SIZE);
	if(m_epoll < 0){
		return -1;
	}
	struct epoll_event server_epoll_event;
	server_epoll_event.events = EPOLLIN;
	std::map<int, gateway_handle_t*>::iterator i = m_lister_fd_map.begin();

	for ( ;i != m_lister_fd_map.end(); ++i)
	{
		server_epoll_event.data.fd = i->first;
		if (epoll_ctl(g_epoll,EPOLL_CTL_ADD,i->first,&server_epoll_event) == -1){
			i->second->error(-1,"add epoll fail");
			close(i->first);
		}
		else{
			++m_online;
		}
	}
	return 0;
}

int tcpClass::go_run()
{

}
int tcpClass::close_fd(int fd)
{}



int 						tcpClass::init_node(const int fd)
{}
fd_data_struct_t* 			tcpClass::find_node(const int fd)
{}
void 						tcpClass::del_node(const int fd)
{}

