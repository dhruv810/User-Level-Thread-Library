#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

// gloable variable to save default signal handler
sigset_t old_data;
bool preempt_at_start = false;

// Blocking SIGVTALRM signal
void preempt_disable(void){
	if (!preempt_at_start){	//if preemption disabled, nothing to do
		return;
	}
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &ss, &old_data);	//stop listening for the alarm
}

// Unblocking SIGVTALRM signal
void preempt_enable(void){
	if (!preempt_at_start){	//if preemption disabled, nothing to do
		return;
	}
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &ss, &old_data); //continue listening for alarm
}

void preempt_start(bool preempt){
	preempt_at_start = preempt;	//if preemption disabled, nothing to do
	if (!preempt_at_start){
		return;
	}
	struct sigaction sa;
	struct itimerval timer;
	/* Set up handler for alarm */
	sa.sa_handler = (void (*)(int))uthread_yield;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGVTALRM, &sa, NULL);
	// timer
	// new it_value after each iterarion
	timer.it_interval.tv_sec = 0;	// we dont need tv_sec
									// so 0 is for disabling it
	// 1 sec = 1000000 microseconds
	// devide by HZ to fire an alarm 100 times per second
	timer.it_interval.tv_usec = 1000000 / HZ;

	// initial values
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 1000000 / HZ;
	setitimer(ITIMER_VIRTUAL, &timer, NULL);
}

void preempt_stop(void){
	if (!preempt_at_start){ //if preemption disabled, nothing to do
		return;
	}
	sigprocmask(SIG_SETMASK, &old_data, NULL);	// resetting signal handler 
}

