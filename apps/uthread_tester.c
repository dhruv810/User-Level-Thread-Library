/*
 * Thread creation and yielding test
 *
 * Tests the creation of multiples threads and the fact that a parent thread
 * should get returned to before its child is executed.
 * Also verifies that local variables don't interfere with other contexts
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "uthread.h"

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

int args_for_thread_1[] = {1, 2, 3};
char args_for_thread_2[] = {'a', 'b', 'c'};
float args_for_thread_3[] = {3.23, 4.53, 8.55};
int thread_execution_stage = 0;
int threadNum4Through6 = 4;

void thread4through6(void *arg){
	int *thisThreadNum = (int *)arg;
	TEST_ASSERT(++thread_execution_stage == *thisThreadNum+5);
}

void thread3(void *arg){
	float *f = (float *)arg;
	int local_var_thread = 3;
	TEST_ASSERT(++thread_execution_stage == 6);
	TEST_ASSERT(local_var_thread == 3);
	uthread_yield();
	TEST_ASSERT(++thread_execution_stage == 8);
	TEST_ASSERT(local_var_thread == 3);
	TEST_ASSERT(f[0] == args_for_thread_3[0] && f[1] == args_for_thread_3[1] && f[2] == args_for_thread_3[2]);
	uthread_create(thread4through6, &threadNum4Through6); //thread 4
	uthread_yield(); //starts thread 4
	threadNum4Through6 = 5;
	uthread_create(thread4through6, &threadNum4Through6); //thread 5
	uthread_yield();
	threadNum4Through6 = 6;
	uthread_create(thread4through6, &threadNum4Through6); //thread 6
	TEST_ASSERT(++thread_execution_stage == 11);
	threadNum4Through6++;
}

void thread2(void *arg){
	char *c = (char *)arg;
	int local_var_thread = 2;
	TEST_ASSERT(++thread_execution_stage == 3);
	uthread_create(thread3, (void *)args_for_thread_3);
	TEST_ASSERT(++thread_execution_stage == 4);
	TEST_ASSERT(local_var_thread == 2);
	uthread_yield();
	TEST_ASSERT(++thread_execution_stage == 7);
	TEST_ASSERT(local_var_thread == 2);
	TEST_ASSERT(c[0] == args_for_thread_2[0] && c[1] == args_for_thread_2[1] && c[2] == args_for_thread_2[2]);
}

void thread1(void *arg){
	int *i = (int *)arg;
	int local_var_thread = 1;
	TEST_ASSERT(++thread_execution_stage == 1);
	uthread_create(thread2, (void *)args_for_thread_2);
	TEST_ASSERT(++thread_execution_stage == 2);
	uthread_yield();
	TEST_ASSERT(local_var_thread == 1);
	TEST_ASSERT(++thread_execution_stage == 5);
	TEST_ASSERT(i[0] == args_for_thread_1[0] && i[1] == args_for_thread_1[1] && i[2] == args_for_thread_1[2]);
	TEST_ASSERT(local_var_thread == 1);
	uthread_yield();
}

int main(void){
	TEST_ASSERT(thread_execution_stage == 0);
	uthread_run(false, thread1, (void *)args_for_thread_1);
	TEST_ASSERT(++thread_execution_stage == 13);
	return 0;
}
