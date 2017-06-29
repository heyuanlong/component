#ifndef __BASE__
#define __BASE__



#define MSG  	1003


typedef struct head_s head_t;
struct head_s
{
	unsigned short size;
	int cmd;
};


typedef struct msg_s msg_t;
struct msg_s
{
	head_t head;
	int userid;
	int dataSize;
	char data[0];
};




#endif