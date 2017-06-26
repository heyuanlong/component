
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


int tcpClass::add_tcp_listen(const char * ip,const int port,gateway_handle_t *handle)
{

	int fds = socketinit(ip,port);
	if (fds < 0){
		return -1;
	}

	autoLock al(m_lock);
	if (m_status == TCPCLASSRUNNING){
		struct epoll_event server_epoll_event;
		server_epoll_event.events = EPOLLIN;
		server_epoll_event.data.fd = fds;
		if (epoll_ctl(m_epoll,EPOLL_CTL_ADD,fds,&server_epoll_event) == -1){
			handle->error(-1,"add epoll fail");
			return -1;
		}
	}
	m_lister_fd_map[fds] =  handle;

}

int tcpClass::run()
{
	m_status = TCPCLASSRUNNING;
	init_epoll();
	go_run();
}

int tcpClass::init_epoll()
{
	autoLock al(m_lock);
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
		if (epoll_ctl(m_epoll,EPOLL_CTL_ADD,i->first,&server_epoll_event) == -1){
			i->second->error(-1,"add epoll fail");
			m_lister_fd_map.erase(i->first);
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
	int ready_event_nums;
	int fd;
	for (;;)
	{
		m_lock.lock();
		ready_event_nums = epoll_wait(m_epoll,m_ready_event,EPOLL_SIZE,-1);
		for (int i = 0; i < ready_event_nums; ++i)
		{
			fd = m_ready_event[i].data.fd;
			if (!(m_ready_event[i].events & EPOLLIN)){
				continue;
			}

			if(is_lister_fd(fd)){
				deal_lister_fd(fd);
			}
			else{
				deal_client_fd(fd);
			}
		}
		m_lock.unlock();
	}
}

void tcpClass::deal_lister_fd(int fd)
{
	int client = accept(fd,NULL,NULL);
	if (client == -1){
		//
		return;
	}
	if (set_socket_nonblock(client) < 0){
    	//
		return;
    }
	
	if ((m_online+1) > EPOLL_SIZE){
		m_lister_fd_map[fd]->error(client,"more than EPOLL_SIZE");
		return;
	}

	m_temp_event.events = EPOLLIN;
	m_temp_event.data.fd = client;
	if (epoll_ctl(m_epoll,EPOLL_CTL_ADD,client,&m_temp_event) == -1){
		m_lister_fd_map[fd]->error(client,"epoll add client fail");
		return ;
	}
	++m_online;
	init_client_node(client,m_lister_fd_map[fd]);
	m_lister_fd_map[fd]->connect(client,"");
}
void tcpClass::deal_client_fd(int fd)
{
	fd_data_struct_t* n = find_client_node(fd);
	if (n == NULL){
		return;
	}

	int recv_len;
	void *buf = n->pdata + n->size;
	int size = (char *)n->ptail - (char *)n->pdata;
	size = size - (n->size);
	loop:
	recv_len = recv(fd,buf,size,0);
				
	if(recv_len > 0){
		n->size += recv_len;
		if((n->pdata + n->size) == n->ptail && n->pdata != n->porigin){   //
			memcpy(n->porigin,n->pdata,n->size);
			n->pdata = n->porigin;
		}
		if(n->size >= HEADERSIZE){
			unsigned short *s_packet_size = (unsigned short *)n->pdata;
			int i_packet_size = (int)(*s_packet_size);
			if(i_packet_size > MSGMAXSIZE  ){
				pclose_fd(fd);
				n->handle->error(fd,"packet size too big");
			}
			if(n->size >= i_packet_size ){
				n->handle->message(fd,n->pdata,i_packet_size);
				n->pdata += i_packet_size;
				n->size -= i_packet_size;
			}
		}
	}else if (recv_len == 0){
		pclose_fd(fd);
		n->handle->disconnect(fd);
	}else{
		if (errno == EINTR){
			goto loop;
		}
		else if(errno == EAGAIN || errno == EWOULDBLOCK){
			//边缘模式应该不会走到这里
		}
		else{		
			pclose_fd(fd);
			n->handle->error(fd,"client fail");
		}
	}
}

bool tcpClass::is_lister_fd(int fd)
{
	if (m_lister_fd_map.find(fd) == m_lister_fd_map.end()){
		return false;
	}
	
	return true;
}

int tcpClass::close_fd(int fd)
{
	autoLock al(m_lock);
	epoll_ctl(m_epoll,EPOLL_CTL_DEL,fd,NULL);
	del_client_node(fd);
	close(fd);
	--m_online;

	return 0;
}
void tcpClass::pclose_fd(int fd)
{
	epoll_ctl(m_epoll,EPOLL_CTL_DEL,fd,NULL);
	del_client_node(fd);
	close(fd);
	--m_online;
}



int tcpClass::init_client_node(const int fd,gateway_handle_t* handle)
{
	if (m_client_fd_map.find(fd) != m_client_fd_map.end()){
		del_client_node(fd);
	}

	fd_data_struct_t *n = (fd_data_struct_t*)malloc(sizeof(fd_data_struct_t));
	n->porigin = (void*) malloc(MSGMAXSIZE );
	n->ptail = n->porigin + (MSGMAXSIZE );
	n->pdata = n->porigin;
	n->size = 0;
	n->handle = handle;
	m_client_fd_map[fd] = n;

	return 0;
}

fd_data_struct_t* tcpClass::find_client_node(const int fd)
{
	if (m_client_fd_map.find(fd) == m_client_fd_map.end()){
		return NULL;
	}
	return m_client_fd_map[fd];
}

void tcpClass::del_client_node(const int fd)
{
	if (m_client_fd_map.find(fd) != m_client_fd_map.end()){
		fd_data_struct_t *n = m_client_fd_map[fd];
		free(n->porigin);
		free(n);
		m_client_fd_map.erase(fd);
	}
}

