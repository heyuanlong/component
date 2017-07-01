#include "socket_recv.h"



int socket_recv_tcp(int fd,fd_data_struct_t* n )
{
	int recv_len;
	void *buf;
	int size;

	while(1){
		buf = n->r_pdata + n->r_size;
		size = (char *)n->r_ptail - (char *)n->r_pdata;
		size = size - (n->r_size);
		recv_len = recv(fd,buf,size,0);
					
		if(recv_len > 0){
			n->r_size += recv_len;
			if((n->r_pdata + n->r_size) == n->r_ptail && n->r_pdata != n->r_porigin){   //对数据的移动
				memcpy(n->r_porigin,n->r_pdata,n->r_size);
				n->r_pdata = n->r_porigin;
			}
			continue;
		}
		else if (recv_len == 0){
			return NET_CLOSED;
		}
		else{
			if (errno == EINTR){
				continue;
			}
			else if(errno == EAGAIN || errno == EWOULDBLOCK){
				return NET_AGAIN;
			}
			else{
				LOG_ERROR("client have error:%d",errno);
				return NET_ERR;
			}
		}
	}
}

