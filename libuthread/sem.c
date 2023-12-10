#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "private.h"

struct semaphore {
	size_t count;			//number of resources available
	struct queue* waitlist;	//items waiting for resources
};

sem_t sem_create(size_t count){
	struct semaphore* new_sem = malloc(sizeof(struct semaphore));
	if (new_sem == NULL)	// if cant allocate memory return -1
		return NULL;

	// create empty queue for waiting tasks
	new_sem->waitlist = queue_create();
	new_sem->count = count;

	return new_sem;
}

int sem_destroy(sem_t sem){
	// if sem is not NULL and waitlist is empty then free sem's memory
	if (sem == NULL || queue_destroy(sem->waitlist) == -1)
		return -1;

	free(sem);
	return 0;
}

int sem_down(sem_t sem){
	if (sem == NULL){
		return -1;
	}
	// putting thread on waitlist
	struct uthread_tcb* thread = uthread_current();
	queue_enqueue(sem->waitlist, thread);
	if (sem->count == 0){
		// put in block state here
		uthread_block();
	}
	else {
		// every thread will be added to the waitlist before it gets access to the resource, to avoid starvation
		sem->count--;
		sem_up(sem);
	}
	return 0;
}

int sem_up(sem_t sem){
	if (sem == NULL){
		return -1;
	}
	if (queue_length(sem->waitlist) > 0) {
		// take thread out of waitlist now that resource is available
		preempt_disable();
		struct uthread_tcb* thread;
		if (queue_dequeue(sem->waitlist, (void**) &thread) == -1){
			preempt_enable(); //reenable preemption before exiting
			return -1;
		}
		uthread_unblock(thread);
		preempt_enable();
	}
	else
		sem->count++;
	return 0;
}

