
#include "tcpUdpClass.h"


tcpUdpClass::tcpUdpClass()
{
	m_epoll			= -1;
	m_online		= 0;
	m_status		= TCPCLASSNOTRUN;
}

tcpUdpClass::~tcpUdpClass()
{

}

int tcpUdpClass::add_tcp_listen(const char * ip,const int port,gateway_handle_t *handle)
{

	if(m_epoll == -1){
		m_epoll = socket_epoll_create();
	}
	if ((m_online+1) > EPOLL_SIZE){
		LOG_ERROR("more than EPOLL_SIZE");
		handle->error(-1,"more than EPOLL_SIZE");
		return NET_ERR;
	}
	int fds = socket_help_init_tcp(ip,port);
	if (fds == NET_ERR){
		LOG_ERROR("socket_help_init_tcp fail");
		return NET_ERR;
	}


	if (socket_epoll_add(m_epoll,fds,fds) == NET_ERR){
		LOG_ERROR("socket_epoll_add fail");
		handle->error(-1,"add epoll fail");
		close(fds);
		return NET_ERR;
	}
	++m_online;
	m_lister_fd_map[fds] =  handle;
}

int tcpUdpClass::add_udp_listen(const char * ip,const int port,gateway_udp_handle_t *handle)
{

	if(m_epoll == -1){
		m_epoll = socket_epoll_create();
	}
	if ((m_online+1) > EPOLL_SIZE){
		LOG_ERROR("more than EPOLL_SIZE");
		handle->error(-1,"more than EPOLL_SIZE");
		return NET_ERR;
	}

	int fds = socket_help_init_udp(ip,port);
	if (fds == NET_ERR){
		LOG_ERROR("socket_help_init_udp fail");
		return NET_ERR;
	}
	if (socket_epoll_add(m_epoll,fds,fds) == NET_ERR){
		LOG_ERROR("socket_epoll_add fail");
		handle->error(-1,"add epoll fail");
		close(fds);
		return NET_ERR;
	}
	++m_online;
	m_udp_fd_map[fds] =  handle;
}

int tcpUdpClass::run()
{
	go_run();
}



int tcpUdpClass::go_run()
{
	if(m_epoll == -1){
		return NET_EPOLL_ERR;
	}

	int ready_event_nums;
	int fd;

	ready_event_nums = epoll_wait(m_epoll,m_ready_event,EPOLL_SIZE,50);
	if( ready_event_nums < 0 ){
		switch( errno ){
			case EBADF:
			case EINVAL:
				LOG_ERROR("epoll_wait fail");
				return NET_EPOLL_ERR;
		}
	}
	if(ready_event_nums == 0){
		//设置超时的情况
		LOG_INFO("epoll_wait timeout");
		return NET_OK;
	}
	for (int i = 0; i < ready_event_nums; ++i)
	{
		fd = m_ready_event[i].data.fd;
		if( m_ready_event[i].events & (EPOLLERR|EPOLLHUP) ){
            LOG_ERROR("epoll_find fd:%d have error",fd);
			pclose_fd(fd);
			continue;
		}

		if(is_lister_fd(fd)){
			deal_lister_fd(fd);
		}
		else if(is_udp_fd(fd)){
			if ( m_ready_event[i].events & EPOLLIN){
				deal_udp_recv_fd(fd);
			}
			else if (m_ready_event[i].events & EPOLLOUT){
				deal_udp_send_fd(fd);
			}
			else{

			}	
		}
		else{
			if ( m_ready_event[i].events & EPOLLIN){
				deal_client_recv_fd(fd);
			}
			else if (m_ready_event[i].events & EPOLLOUT){
				deal_client_send_fd(fd);
			}
			else{

			}	
		}
	}
	return NET_OK;
}

int tcpUdpClass::deal_lister_fd(int fd)
{
	socket_accept_tcp(fd,this);
}
void tcpUdpClass::deal_udp_recv_fd(int fd)
{
	socket_accept_udp(fd,this);
}

void tcpUdpClass::deal_client_recv_fd(int fd)
{
	fd_data_struct_t* n = find_client_node(fd);
	if (n == NULL){
		return;
	}

	int recv_len;
	void *buf;
	int size;

	loop:

	buf = n->r_pdata + n->r_size;
	size = (char *)n->r_ptail - (char *)n->r_pdata;
	size = size - (n->r_size);
	recv_len = recv(fd,buf,size,0);
				
	if(recv_len > 0){
		n->r_size += recv_len;
		if((n->r_pdata + n->r_size) == n->r_ptail && n->r_pdata != n->r_porigin){   //对数据的移动
			memcpy(n->r_porigin,n->r_pdata,n->r_size);
			n->r_pdata = n->r_porigin;
			//这里要重构，回形缓存满了，导致接受的数据不够多，要下次epoll才有。（当前是LT，问题倒是不大）
		}
		if(n->r_size >= sizeof(header_t)){
			int i_packet_size = ((header_t *)n->r_pdata)->size;

			if(i_packet_size > MSGMAXSIZE  ){
				pclose_fd(fd);
				n->handle->error(fd,"packet size too big");
				return ;
			}

			if(n->r_size >= i_packet_size ){
				n->handle->message(fd,n->r_pdata,((header_t *)n->r_pdata)->cmd,i_packet_size);
				n->r_pdata += i_packet_size;
				n->r_size -= i_packet_size;

				goto loop;
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
			return;
		}
		else{		
			pclose_fd(fd);
			n->handle->error(fd,"client fail");
		}
	}
}

void tcpUdpClass::send_udp_addr(int fd,char *buf,int size,struct sockaddr_in client_addr,socklen_t client_len)
{
	socket_send_udp_addr(fd,buf,size,client_addr);
}
void tcpUdpClass::send_udp_ip_port(int fd,char *buf,int size,const char *ip,const int port)
{
	socket_send_udp_ip_port(fd,buf,size,ip,port);
}
void tcpUdpClass::send_tcp(int fd,char *buf,int size)
{
	if(size <= 0 ){
		return;
	}
	deal_client_send_fd(fd);

	fd_data_struct_t* n = find_client_node(fd);
	if( n == NULL){
		return ;
	}
	if(n->s_size != 0){		//存在缓存
		//加入缓存
		add_to_send_buf(n,buf,size);
		socket_epoll_write(m_epoll,fd,fd,true);
		return ;
	}
	//不存在缓存
	//发送
	//加入缓存
	int ret = socket_send_tcp(fd,buf,size);
	if (ret == NET_ERR){
		pclose_fd(fd);
		return;
	}

	if((size - ret) > 0){
		add_to_send_buf(n,buf + ret,size - ret);
		socket_epoll_write(m_epoll,fd,fd,true);
	}
	return;
}
int tcpUdpClass::add_to_send_buf(fd_data_struct_t* n,char *buf,int size)
{
	if (size <= 0 ){
		return 0;
	}
	if ( (n->s_size + size) > MSGMAXSIZE ){
		//printf("---------------------------------------------------------------------------------5\n");
		return -1;
	}

	if ( (n->s_pdata + n->s_size + size ) > n->s_ptail ){
		memcpy(n->s_porigin,n->s_pdata,n->s_size);
		n->s_pdata = n->s_porigin;
	}

	memcpy(n->s_pdata + n->s_size , buf,size);
	n->s_size += size;

	return 0;
}


void tcpUdpClass::deal_client_send_fd(int fd)
{
	fd_data_struct_t* n = find_client_node(fd);
	if( n == NULL){
		return ;
	}
	if(n->s_size == 0){
		return ;
	}

	int ret = socket_send_tcp(fd,(char *)n->s_pdata,n->s_size);
	if (ret == NET_ERR){
		//printf("--------------------------------------------------------------------------6\n");
		pclose_fd(fd);
	}

	n->s_pdata += ret;
	n->s_size -=ret; 
	if(n->s_size == 0){
		socket_epoll_write(m_epoll,fd,fd,false);
	}
	return;
}	


bool tcpUdpClass::is_lister_fd(int fd)
{
	if (m_lister_fd_map.find(fd) == m_lister_fd_map.end()){
		return false;
	}
	
	return true;
}
bool tcpUdpClass::is_udp_fd(int fd)
{
	if (m_udp_fd_map.find(fd) == m_udp_fd_map.end()){
		return false;
	}
	
	return true;
}
int tcpUdpClass::close_fd(int fd)
{
	epoll_ctl(m_epoll,EPOLL_CTL_DEL,fd,NULL);
	del_client_node(fd);
	close(fd);
	--m_online;

	return 0;
}
void tcpUdpClass::pclose_fd(int fd)
{
	epoll_ctl(m_epoll,EPOLL_CTL_DEL,fd,NULL);
	del_client_node(fd);
	close(fd);
	--m_online;
}



int tcpUdpClass::init_client_node(const int fd,gateway_handle_t* handle)
{
	if (m_client_fd_map.find(fd) != m_client_fd_map.end()){
		del_client_node(fd);
	}

	fd_data_struct_t *n = (fd_data_struct_t*)malloc(sizeof(fd_data_struct_t));
	n->r_porigin = (void*) malloc(MSGMAXSIZE );
	n->r_ptail = n->r_porigin + (MSGMAXSIZE );
	n->r_pdata = n->r_porigin;
	n->r_size = 0;

	n->s_porigin = (void*) malloc(MSGMAXSIZE );
	n->s_ptail = n->s_porigin + (MSGMAXSIZE );
	n->s_pdata = n->s_porigin;
	n->s_size = 0;

	n->handle = handle;
	m_client_fd_map[fd] = n;

	return 0;
}

fd_data_struct_t* tcpUdpClass::find_client_node(const int fd)
{
	if (m_client_fd_map.find(fd) == m_client_fd_map.end()){
		return NULL;
	}
	return m_client_fd_map[fd];
}

void tcpUdpClass::del_client_node(const int fd)
{
	if (m_client_fd_map.find(fd) != m_client_fd_map.end()){
		fd_data_struct_t *n = m_client_fd_map[fd];
		free(n->r_porigin);
		free(n->s_porigin);
		free(n);
		m_client_fd_map.erase(fd);
	}
}

