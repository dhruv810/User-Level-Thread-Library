# Project 2: User-Level Thread Library

## Summary

This program, User-Level Thread Library, emulates a basic user-level thread
library for Linux and other POSIX compatible systems. Specifically, it
implements a complete interface for applications to create and run independent
threads concurrently. It also includes an interrupt-based scheduler and thread
synchronization API. It also features a round-robin style thread execution order
with optional preemption to prevent starvation.

## Stages

### Stage 1: queue API

For stage one we implemented a FIFO queue, which uses a linked list to store
arbitrarily long lists of arbitrary data (using void pointers to point to
whatever data structures are required to be stored). To satisfy the requirement
that all operations (except iterate and delete) are O(1), we used a distinct
struct for tracking the reference to the head and tail of the queue, along with
the current size of the queue. Each individual queue node, meanwhile, simply had
a data item and a pointer to the next node.  We also implemented extensive
testing for the queue in queue_tester, verifying that the queue implementation
was functional, including all the expected error states.

### Stage 2: uthread API

The uthread threading API was the most complex part of the project, as it
involved tracking the state of each thread, backing up and restoration of their
registers and their stack. To do this, we used a TCB (Thread Control Block)
struct, which contained all the required information, along with sequentially
assigned numerical thread ID, which was primarily used to simplify debugging and
analysis of the threading behavior, but could also be useful for the future
addition of expanded thread-management features akin to pkill. Fundamentally,
the uthread API is pretty simple, the biggest challenge was understanding the
underlying implementation details of the context switching (as provided by
context.c), and how best to utilize those and keep track of the state of each of
the tasks.

### Stage 3: semaphore API

For the third stage of the project, we implemented a semaphore API to control
access to common resources shared by multiple threads. We used an internal count
to keep track of available resources which initialized to a certain number when
semaphore was created. We used a queue ("waitlist") to track the threads
which were waiting to get access to resources. When a resource becomes newly
available it should be allocated to the first waiting thread, in order to
avoid the corner case where a new thread requests the resource before the thread
that was waiting, which could cause starvation of the waiting thread, we decided
to add each thread to the waitlist before allowing it to proceed. This way,
when the resource is available, the first thread in the waitlist will get
access to the resource before any other threads can take control of it.

### Stage 4: preemption

The final stage of the project, the preemption, was easy to implement but
required a deep understanding of context provided in the information page. The
uthread_yield function changes global variables so making sure that the current
thread does not get interrupted during execution of this function is very
important. That is why we disable preempting before any changes are made to
global variables. This includes freeing memory of completed threads, starting
to execute the next thread in the waitlist, and adding the current thread to
the waitlist if it is not completed. We enabled preempt at the end of the
uthread_yield function. By doing this, we prevent the possibility of losing
pointers to a thread, freeing already freed memory, not adding current running
processes to the waitlist or other similar conditions. We also disabled
preemption during sem_up. If a thread gets interrupted before unblocking the
current thread, the second thread can call unblock before the first one. With
that logic, the first thread will be added to the waitlist after the second
thread, which can cause delay in execution of the first thread. To implement
this, we used the settimer function to send SIGVTALRM signal 100 times per
second, and used sigaction to execute uthread_yield every time SIGVTALRM is
sent. After the implementation, we wrote preempt_test.c, which loops through a
busy loop until preemption interrupts it, as described in more detail in the
testing section below.

### Error Handling

Compared to project 1 (SShell), error handling for this project was relatively
simple, as we didn't need to handle arbitrary string input from users, but
instead simply needed to detect invalid requests, such as when users of the API
attempt to insert items onto a nonexistent queue or semaphore, or destroy a
queue or semaphore that is nonempty. For those cases, we simply returned either
NULL or -1, depending on the return type (int or pointer, respectively).

### Testing

To test our code, we used the test files provided in the starter code. We
created and programmed more test cases in queue_tester to thoroughly test
the queue API and verify all failure modes were handled correctly. Similarly,
for uthread_tester, we added test cases that verified the order of execution
between threads was as expected, that local variables in different contexts
didn't interfere with each other, and that passing arguments to new threads also
behaved as intended. For testing semaphores, we used some global counter flags
to ensure that code execution was behaving in the expected order, properly
blocking threads when they were waiting on unavailable resources. We also
specifically added a test case to focus on the corner case described in the
assignment, to ensure the threads all stayed blocked appropriately and were
unblocked in the right order.

For testing preemption, we needed a slightly more creative approach, because
preemption leads to nondeterministic behavior (as it depends on timing, and the
code could run at a different rate on different systems, or the same system when
under different levels of load). Thus, in preempt_tester.c, we run two threads,
one of which is a busy loop that simply iterates an inner loop 4 million times
before printing and updating a counter. This thread never calls uthread_yield,
thus it can only be interrupted by preemption. When it is interrupted, the other
thread increments its counter, prints, and yields, returning control to the
uncooperative thread. After the cooperative thread finishes its 20th iteration,
it sets a global flag that terminates the uncooperative thread. The
uncooperative thread's counter is then checked to be within a range of 5 to 1000
iterations, to add flexibility when on significantly slower or faster hardware.
We also added tests to verify the SIVGTALRM signal is not continuing to be sent
after the preemption ends.

### Sources Referenced

[Examine and Change Blocked Signals](https://www.ibm.com/docs/en/i/7.3?topic=ssw_ibm_i_73/apis/sigpmsk.html)

[setitimer() — Set value of an interval timer](https://www.ibm.com/docs/en/zos/2.4.0?topic=functions-setitimer-set-value-interval-timer)

[sigprocmask, sigsetmask, or sigblock](https://www.ibm.com/docs/en/aix/7.2?topic=s-sigprocmask-sigsetmask-sigblock-subroutine#sigprocmask__row-d3e56360)

Copyright 2022, Patrick Leiser, Dhruv Fadadu