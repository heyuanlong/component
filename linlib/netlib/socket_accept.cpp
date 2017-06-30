#include <socket_accept.h>



int socket_accept_tcp(int fd, tcpUdpClass	*ptp)
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
				return NET_AGAIN;
			}
			return NET_ERR;
		}

		set_socket_nonblock(client);
		if ((ptp->m_online+1) > EPOLL_SIZE){
			LOG_ERROR("more than EPOLL_SIZE");
			(ptp->m_lister_fd_map)[fd]->error(client,"more than EPOLL_SIZE");
			return NET_ERR;
		}

		if (socket_epoll_add(m_epoll,client,client) == NET_ERR){
			LOG_ERROR("socket_epoll_add fail");
			(ptp->m_lister_fd_map)[fd]->error(-1,"add epoll fail");
			close(client);
			return NET_ERR;
		}

		++ptp->m_online;
		ptp->init_client_node(client,(ptp->m_lister_fd_map)[fd]);
		(ptp->m_lister_fd_map)[fd]->connect(client,"");
	}
}


char tmp_udp_buf[MSGMAXSIZE];
int socket_accept_udp(int fd,std::map<int, tcpUdpClass	*ptp)
{
	int  len;
	struct sockaddr_in addr;
  	socklen_t client_len = sizeof(addr);

	while(1){
		len = recvfrom(fd, tmp_udp_buf, MSGMAXSIZE, 0, (struct sockaddr *)&addr, &client_len);
		if (len > 0){
			(ptp->m_udp_fd_map)[fd]->message(fd,tmp_udp_buf,len,addr,client_len);
			continue;
		}
		return NET_OK;
	}
}