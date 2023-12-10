#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

typedef enum{
	RUNNING,
	READY,
	BLOCKED,
	EXITED
} uthread_state_t;

struct uthread_tcb{
	uthread_ctx_t *context;
	void *stack;
	//void *arg;
	uthread_state_t state;
	int tid;  //thread ID, assigned sequentially
};

struct uthread_tcb *currentContext;
struct uthread_tcb *previousContext;
queue_t waitingTasks;
int maxTid;
bool threadingActive = false;

struct uthread_tcb *uthread_current(void){
	return currentContext;
}

void uthread_yield(void){
	if(!threadingActive){ //if we're the only running thread
		return;
	}
	// running thread will move to ready
	if (currentContext->state == RUNNING){
		currentContext->state = READY;
	}
	preempt_disable();
	// keeping preempt disable in this block becuase if thread stop between
	// freeing data, then uthread_yield will be called again and this time we would get error of accesing freed memory
	struct uthread_tcb *nextContext;
	if (previousContext != NULL && previousContext->state == EXITED){
		uthread_ctx_destroy_stack(previousContext->stack);
		free(previousContext->context);
		free(previousContext);
		previousContext = NULL;
	}

	// if process stopes after queue_dequeue, it will be called again and this time new thread will execute and we will lost one thread
	if (queue_dequeue(waitingTasks, (void**)&nextContext) == -1) {
		;
		return;
	}
	// cannot let the thread end before adding it to the queue and changing previousContext
	nextContext->state = RUNNING;
	if (currentContext->state == READY){
		queue_enqueue(waitingTasks, currentContext);
	}
	previousContext=currentContext;
	currentContext=nextContext;
	uthread_ctx_switch(previousContext->context,currentContext->context);
	preempt_enable();

}

void uthread_exit(void){
	// setting state to EXITED, so uthread_yield will free its memory
	currentContext->state = EXITED;
	// calling uthread_yield to start running next process
	uthread_yield();
}

int uthread_create(uthread_func_t func, void *arg){
	if(!threadingActive){ //if multithreading not active
		return -1;
	}
	// creating new thread
	struct uthread_tcb *newThread = calloc(1,sizeof(struct uthread_tcb));
	newThread->stack = uthread_ctx_alloc_stack();
	newThread->context = calloc(1,sizeof(uthread_ctx_t));
	newThread->state = READY;
	newThread->tid=maxTid++;
	if (newThread == NULL || newThread->stack == NULL || newThread->context == NULL){ //memory allocation failed
		return -1;
	}
	if(uthread_ctx_init(newThread->context, newThread->stack, func, arg) == -1){ //context init failed
		return -1;
	}
	preempt_disable();
	// adding new thread to waiting task
	queue_enqueue(waitingTasks, newThread);
	preempt_enable();
	return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg){
	//initialize new context
	threadingActive = true;
	previousContext = NULL;
	// setting up ideal thread
	struct uthread_tcb *idleThread = calloc(1,sizeof(struct uthread_tcb));
	idleThread->stack = uthread_ctx_alloc_stack();
	idleThread->context = calloc(1,sizeof(uthread_ctx_t));
	idleThread->state = READY;
	idleThread->tid=-1;
	getcontext(idleThread->context);
	currentContext=idleThread;
	maxTid=0;
	waitingTasks = queue_create();

	if (uthread_create(func, arg) == -1) { // failed to create new thread
		return -1;
	}
	//start the preemption, if preempt is false functions in preempt will not execute
	preempt_start(preempt);
	//calling uthread_yield to start running the new thread
	while(queue_length(waitingTasks) > 0){
		uthread_yield();
	}
	preempt_stop();
	threadingActive = false;
	// freeing all data
	queue_destroy(waitingTasks);
	uthread_ctx_destroy_stack(previousContext->stack);
	free(previousContext->context);
	free(previousContext);
	free(idleThread->context);
	free(idleThread->stack);
	free(idleThread);

	return 0;
}

void uthread_block(void){
	// setting stack to blocked and letting next thread run
	currentContext->state = BLOCKED;
	uthread_yield();
}

void uthread_unblock(struct uthread_tcb *uthread){
	// if not blocked, no need to unblock it
	if (uthread->state != BLOCKED){
		return;
	}
	// setting set to ready and put it in queue
	uthread->state = READY;
	queue_enqueue(waitingTasks, uthread);
}

