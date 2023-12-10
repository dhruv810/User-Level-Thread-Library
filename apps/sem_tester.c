#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
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

void test_create(void){
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(sem_create(1) != NULL);
}

void test_destroy(void){
	fprintf(stderr, "*** TEST destroy ***\n");
	sem_t s = sem_create(1);
	TEST_ASSERT(sem_destroy(NULL) == -1);
	TEST_ASSERT(sem_destroy(s) == 0);
}

void test_null_arg(void){
	fprintf(stderr, "*** TEST null_arg ***\n");
	TEST_ASSERT(sem_destroy(NULL) == -1);
	TEST_ASSERT(sem_down(NULL) == -1);
	TEST_ASSERT(sem_up(NULL) == -1);
}

int blocked_test_tracker = 0;
int corner_case_tracker  = 0;
sem_t child_down_sem;
sem_t corner_case_sem;

void corner_case_thread_B(){
	TEST_ASSERT(++corner_case_tracker == 2);
	sem_up(corner_case_sem);
}

void corner_case_thread_C(){
	sem_down(corner_case_sem);
	TEST_ASSERT(++corner_case_tracker == 3 || corner_case_tracker == 5);
	uthread_yield();  //would resume execution of test_corner_case, if not for semaphore blocking it
	TEST_ASSERT(++corner_case_tracker == 4 || corner_case_tracker == 6);
	sem_up(corner_case_sem);
}

void test_corner_case(){
	fprintf(stderr, "*** TEST corner_case ***\n");
	uthread_create(corner_case_thread_B,NULL);
	uthread_create(corner_case_thread_C,NULL);
	corner_case_sem = sem_create(0);
	TEST_ASSERT(++corner_case_tracker == 1);
	sem_down(corner_case_sem);
	//the or handles either valid order of operations
	TEST_ASSERT(++corner_case_tracker == 5 || corner_case_tracker==3);
	uthread_yield();  //would resume execution of corner_case_thread_C, if not for semaphore blocking it
	TEST_ASSERT(++corner_case_tracker == 6 || corner_case_tracker==4);
	sem_up(corner_case_sem);
}

void thread_child_down(){
	TEST_ASSERT(++blocked_test_tracker == 2);
	TEST_ASSERT(sem_down(child_down_sem) == 0);
	TEST_ASSERT(++blocked_test_tracker == 3);
	sem_down(child_down_sem);
	TEST_ASSERT(++blocked_test_tracker == 6);
}

void test_blocked_lifecycle(){
	fprintf(stderr, "*** TEST blocked_lifecycle ***\n");
	child_down_sem = sem_create(1);
	TEST_ASSERT(++blocked_test_tracker == 1);
	uthread_create(thread_child_down,NULL);
	uthread_yield();
	TEST_ASSERT(++blocked_test_tracker == 4);
	TEST_ASSERT(sem_destroy(child_down_sem) == -1);
	uthread_yield(); //cannot yield to blocked process, so continues
	TEST_ASSERT(++blocked_test_tracker == 5);
	TEST_ASSERT(sem_up(child_down_sem) == 0);
	TEST_ASSERT(sem_destroy(child_down_sem) == 0);
	test_corner_case();
}


int main(void){
	test_create();
	test_destroy();
	test_null_arg();
	uthread_run(false,test_blocked_lifecycle, NULL);
	return 0;
}



