#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "timeoutlib/timeoutClass.h"


 void* callback(void *arg)
 {
 	void * g = arg;
 	printf("arg:%p\n",g);	

 }

int main(int argc, char const *argv[])
{


	timeoutClass tc;
	tc.add_timer(1,0,callback,(void*)(10),true);
	tc.add_timer(5,5000,callback,(void*)(55000),false);

	bool first = false;
	for(;;){
		tc.run();

		if(first == false){
			tc.add_timer(4,1000000 / 2,callback,(void*)(333),true);
			first = true;
		}
	}
	
	return 0;
}

