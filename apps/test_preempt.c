#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "uthread.h"

#define loops_to_wait 4000000 //may want to adjust for faster or slower systems


#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)

int thread_two_finished = 0;

void signal_error(){
	TEST_ASSERT("ERROR, RECIVED SIGVTALARM" && 1==2);
}

void test_signal_cleanup(){
	//verify the alarm timer isn't still running
	fprintf(stderr, "*** TEST signal cleanup ***\n");
	struct sigaction sa;
	/* Set up handler for alarm */
	sa.sa_handler = (void (*)(int))signal_error;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGVTALRM, &sa, NULL);
	for(int i=0; i<5; i++){
		for(volatile long j=0; j < loops_to_wait*100; j++){
			;
		}
		printf("iterating\n");
	}
	TEST_ASSERT("signal cleanup test successful" || 1==1);
}

void thread_two(){
	int thread_two_counter = 0;
	for(int i=0; i < 20; i++){
		printf("Thread 2\n");
		thread_two_counter++;
		uthread_yield();
	}
	thread_two_finished = 1;
	printf("Thread 2 finished (ran %d times)\n",thread_two_counter);
	TEST_ASSERT(thread_two_counter == 20);
}

void thread_one(){
	uthread_create(thread_two, NULL);
	int thread_one_counter = 0;
	while(1){
		//busy loop to waste time until preempted
		for(volatile long j=0; j < loops_to_wait; j++){
			if (thread_two_finished){
				break;
			}
		}
		if (thread_two_finished){
			break;
		}
		printf("Thread 1\n");
		thread_one_counter++;
	}
	printf("Thread 1 finished (ran %d times)\n",thread_one_counter);
	TEST_ASSERT(thread_one_counter >= 5 && thread_one_counter <= 1000);
}

int main(void){
	uthread_run(true, thread_one, NULL);
	test_signal_cleanup();
	return 0;
}
