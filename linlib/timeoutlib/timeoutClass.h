#ifndef TIMEOUTCLASS_H
#define TIMEOUTCLASS_H

#include "minheap-internal.h"
 #include <sys/time.h>

typedef void* (*timer_callback)(void *arg);
typedef struct timerValue{
	void* user_arg;
	bool isloop;
	int s;
	int us;
}timerValue_t;

class timeoutClass
{
public:
	timeoutClass();
	~timeoutClass();

public:
	int run();
	int add_timer(int s,int us,timer_callback call,void *user_arg,bool isloop = false);

	/* data */

private:
	int add_timer_loop(struct hvs_event* p,int s,int us);
	int set_time();
	struct timeval get_add_tv(int s,int us);
	struct timeval get_sub_tv(struct timeval t1,struct timeval t2);
	int go_run();
private:
	min_heap_t hp;
	struct timeval current_tv;
};

#endif