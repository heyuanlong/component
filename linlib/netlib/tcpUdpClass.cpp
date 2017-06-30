
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
		m_epoll = sp_create();
	}
	if ((m_online+1) > EPOLL_SIZE){
		handle->error(-1,"more than EPOLL_SIZE");
		return -1;
	}
	int fds = socketinit(ip,port);
	if (fds < 0){
		return -1;
	}


	if (sp_add(m_epoll,fds,fds) == -1){
		handle->error(-1,"add epoll fail");
		close(fds);
		return -1;
	}
	++m_online;
	m_lister_fd_map[fds] =  handle;
}

int tcpUdpClass::add_udp_listen(const char * ip,const int port,gateway_udp_handle_t *handle)
{

	if(m_epoll == -1){
		m_epoll = sp_create();
	}
	if ((m_online+1) > EPOLL_SIZE){
		handle->error(-1,"more than EPOLL_SIZE");
		return -1;
	}

	int fds = socketinit_udp(ip,port);
	if (fds < 0){
		return -1;
	}
	if (sp_add(m_epoll,fds,fds) == -1){
		handle->error(-1,"add epoll fail");
		close(fds);
		return -1;
	}
	++m_online;
	m_udp_fd_map[fds] =  handle;
}

int tcpUdpClass::run()
{
	go_run();
}

int tcpUdpClass::init_epoll()
{
	return 0;
}

int tcpUdpClass::go_run()
{
	if(m_epoll == -1){
		return -1;
	}

	int ready_event_nums;
	int fd;

	ready_event_nums = epoll_wait(m_epoll,m_ready_event,EPOLL_SIZE,50);
	if( ready_event_nums < 0 ){
		switch( errno ){
			case EBADF:
			case EINVAL:
				//ko_log_error( "event process error for ep is wrong" );
				return -1;
		}
	}
	if(ready_event_nums == 0){
		//设置超时的情况
		printf("--------------------设置超时的情况------------------------\n");
		return 0;
	}
	for (int i = 0; i < ready_event_nums; ++i)
	{
		fd = m_ready_event[i].data.fd;
		if( m_ready_event[i].events & (EPOLLERR|EPOLLHUP) ){
            //ko_log_print_debug("epoll event error, revents:%d", revents);
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
	return 0;
}

int tcpUdpClass::deal_lister_fd(int fd)
{
	struct sockaddr_in addr_in;
	int size;

	while( 1 ){
		int client = accept(fd,( struct sockaddr* )&addr_in, ( socklen_t* ) &size );
		if (client == -1){
			if(errno == EINTR){
				continue;
			}
			if(errno == EAGAIN || errno == EWOULDBLOCK){
				return 0;
			}
			return -1;
		}

		set_socket_nonblock(client);
		if ((m_online+1) > EPOLL_SIZE){
			m_lister_fd_map[fd]->error(client,"more than EPOLL_SIZE");
			return -1;
		}

		if (sp_add(m_epoll,client,client) == -1){
			m_lister_fd_map[fd]->error(-1,"add epoll fail");
			close(client);
			return -1;
		}

		++m_online;
		init_client_node(client,m_lister_fd_map[fd]);
		m_lister_fd_map[fd]->connect(client,"");
	}
}
void tcpUdpClass::deal_udp_recv_fd(int fd)
{
	int  len;
	struct sockaddr_in client_addr;
  	socklen_t client_len = sizeof(client_addr);

	while(1){
		len = recvfrom(fd, m_udp_buf, MSGMAXSIZE, 0, (struct sockaddr *)&client_addr, &client_len);
		if (len > 0){
			m_udp_fd_map[fd]->message(fd,m_udp_buf,len,client_addr,client_len);
			continue;
		}
		return;
	}
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

void tcpUdpClass::send_udp(int fd,char *buf,int size,struct sockaddr_in client_addr,socklen_t client_len)
{
	if(size <= 0 ){
		return;
	}
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
		return ;
	}
	//不存在缓存
	//发送
	//加入缓存
	int ret = net_send(fd,buf,size);
	if (ret == -1){
		pclose_fd(fd);
		//printf("--------------------------------------------------------------------------4\n");
		return;
	}

	add_to_send_buf(n,buf + ret,size - ret);
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

void tcpUdpClass::deal_udp_send_fd(int fd)
{

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

	int ret = net_send(fd,(char *)n->s_pdata,n->s_size);
	if (ret == -1){
		//printf("--------------------------------------------------------------------------6\n");
		pclose_fd(fd);
	}

	n->s_pdata += ret;
	n->s_size -=ret; 
	return;
}	
int tcpUdpClass::net_send(int fd,char *buf,int size)
{
	int flag = 0;
	int send_len = 0;
	while(size > send_len){
		flag = send(fd,buf + send_len,size - send_len,0);
		if( flag < 0 ){
			switch( errno ){
				case EINTR:
					continue;
				case EAGAIN:
					return send_len;
			}
			return -1;
		}
/*		if(flag == 0){ 					//什么情况下会出现
			return 0;
		}
*/
		send_len += flag;
	}
	return send_len;
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

