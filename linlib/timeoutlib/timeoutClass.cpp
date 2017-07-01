#include "timeoutClass.h"
#include "stdio.h"


timeoutClass::timeoutClass()
{
	min_heap_ctor(&hp);
}
timeoutClass::~timeoutClass()
{

}

int timeoutClass::run()
{
	set_time();
	return go_run();
}
int timeoutClass::add_timer(int s,int us,timer_callback call,void *user_arg,bool isloop)
{
	if( us >= 1000000 || us < 0 ){
		us = 1000000 - 1;
	}
	if (s < 0 ){
		s = 1;
	}

	set_time();
	
	struct hvs_event* p = (struct hvs_event*)malloc(sizeof(struct hvs_event));
	timerValue_t *ptv =  (timerValue_t *)malloc(sizeof(timerValue_t));
	ptv->user_arg = user_arg;
	ptv->isloop = isloop;
	ptv->s = s;
	ptv->us = us;

	min_heap_elem_init(p);
	p->arg = (void*)ptv;
	p->callback = call;
	p->ev_timeout=get_add_tv(s,us);

	if (min_heap_push(&hp,p) < 0 ){
		return -1;
	}
	return 0;
}
int timeoutClass::add_timer_loop(struct hvs_event* p,int s,int us)
{
	min_heap_elem_init(p);
	p->ev_timeout=get_add_tv(s,us);

	if (min_heap_push(&hp,p) < 0 ){
		return -1;
	}
	return 0;
}

int timeoutClass::set_time()
{
    gettimeofday(&current_tv, 0);
}

int timeoutClass::go_run()
{
	struct hvs_event* 	p_tmp_evt;
	struct timeval 		sub_tv;
	timerValue_t 		*ptv;

	while((p_tmp_evt = min_heap_top(&hp)) != 0){
		sub_tv = get_sub_tv(current_tv,p_tmp_evt->ev_timeout);

		if (sub_tv.tv_sec > 0 || (sub_tv.tv_sec == 0 && sub_tv.tv_usec > 0) )
		{
			min_heap_pop(&hp);
			ptv = (timerValue_t*)p_tmp_evt->arg;
			p_tmp_evt->callback(ptv->user_arg);

			if (ptv->isloop){
				add_timer_loop(p_tmp_evt,ptv->s,ptv->us);
			}
			else{
				free(p_tmp_evt->arg);
				free(p_tmp_evt);
			}
			continue;
		}
		break;
	}
}

struct timeval timeoutClass::get_add_tv(int s,int us)
{
	struct timeval vvp;

	vvp.tv_sec=s + current_tv.tv_sec;
	vvp.tv_usec=us + current_tv.tv_usec;
	if(vvp.tv_usec >= 1000000){
		++vvp.tv_sec;
		vvp.tv_usec -= 1000000;
	}
	return vvp;
}
struct timeval timeoutClass::get_sub_tv(struct timeval t1,struct timeval t2)
{
	struct timeval vvp;

	vvp.tv_sec = t1.tv_sec - t2.tv_sec;
	vvp.tv_usec = t1.tv_usec - t2.tv_usec;
	if (vvp.tv_usec < 0) {
		vvp.tv_sec--;
		vvp.tv_usec += 1000000;
	}

	return vvp;
}