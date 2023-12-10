#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"


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

/* Create */
void test_create(void){
	fprintf(stderr, "*** TEST create ***\n");
	TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void){
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
	queue_destroy(q);
}

void test_queue_2_items(void){
	int data1 = 10;
	int data2 = -3;
	int *ptr1, *ptr2;
	queue_t q = NULL;
	fprintf(stderr, "*** TEST queue_two_items ***\n");
	TEST_ASSERT(queue_length(q) == -1);
	q = queue_create();
	TEST_ASSERT(queue_length(q) == 0);
	queue_enqueue(q, &data1);
	queue_enqueue(q, &data2);
	TEST_ASSERT(queue_length(q) == 2);
	queue_dequeue(q, (void**)&ptr1);
	TEST_ASSERT(queue_length(q) == 1);
	queue_dequeue(q, (void**)&ptr2);
	TEST_ASSERT(ptr1 == &data1);
	TEST_ASSERT(ptr2 == &data2);
	TEST_ASSERT(*ptr1 == data1);
	TEST_ASSERT(*ptr2 == data2);
	TEST_ASSERT(queue_length(q) == 0);
	queue_destroy(q);
}

/* Enqueue/Dequeue for 5 data */
void test_queue_5_items(void){
	int *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_5_items ***\n");
	int data[5] = {1000, 24, 34, 70, 500};
	q = queue_create();
	for (int i = 0; i < 5; i++) {	// adding 5 entries of data
		queue_enqueue(q, &data[i]);
	}

	for (int i = 0; i < 5; i++) {	// cheking all data is recived in right order
		queue_dequeue(q, (void**)&ptr);
		TEST_ASSERT(ptr == &data[i]);
	}
	queue_destroy(q);
}


/* Destroy */
void test_destroy(void){
	int data = 0, *ptr;
	queue_t q = NULL;

	fprintf(stderr, "*** TEST queue_distroy while NULL ***\n");
	int i = queue_destroy(q);
	TEST_ASSERT(i == -1);	// when queue is NULL, should return -1

	q = queue_create();

	fprintf(stderr, "*** TEST queue_distroy when not NULL ***\n");
	queue_enqueue(q, &data);
	i = queue_destroy(q);
	TEST_ASSERT(i == -1);	// when queue has data, should return -1

	fprintf(stderr, "*** TEST queue_distroy after deleting all nodes ***\n");
	queue_dequeue(q, (void**)&ptr);
	i = queue_destroy(q);
	TEST_ASSERT(i == 0);	// when data is empty, should return 0
}

/* Delete when got only one data */
void test_delete_in_one(void){
	int data = 0, wrong_data = 1;
	queue_t q;

	fprintf(stderr, "*** TEST test_delete_in_one ***\n");
	q = queue_create();

	queue_enqueue(q, &data);

	int i = queue_delete(q, &wrong_data);
	TEST_ASSERT(i == -1);	// deleting wrong data should return -1

	i = queue_delete(q, &data);
	TEST_ASSERT(i == 0);	// deleting data should return 1 and delete the node and sould be able to destroy 

	i = queue_destroy(q);
	TEST_ASSERT(i == 0);	// when queue is empty, should return 0
}

/* Delete */
void test_delete(void){
	int *ptr;
	queue_t q;
	int data[5] = {1000, 24, 34, 70, 500};
	int index_after_delete[3] = {1, 3};

	fprintf(stderr, "*** TEST test_delete ***\n");

	q = queue_create();
	for (int i = 0; i < 5; i++) {	// adding 5 entries of data
		queue_enqueue(q, &data[i]);
	}

	// deleting first
	int i = queue_delete(q, &data[0]);
	TEST_ASSERT(i == 0);	// deleting data[0] should return 0

	// length should be 4 now
	TEST_ASSERT(queue_length(q) == 4);

	// deleting middle
	i = queue_delete(q, &data[2]);
	TEST_ASSERT(i == 0);	// deleting data should return 0 and delete the node and sould be able to destroy 
	// length should be 4 now
	TEST_ASSERT(queue_length(q) == 3);

	// deleting last
	i = queue_delete(q, &data[4]);
	TEST_ASSERT(i == 0);	// deleting data should return 0 and delete the node and sould be able to destroy 
	// length should be 4 now
	TEST_ASSERT(queue_length(q) == 2);

	for (int i = 0; i < 2; i++) {	// usign queue_dequeue to check the order of remaining nodes
		queue_dequeue(q, (void**)&ptr);
		TEST_ASSERT(ptr == &data[index_after_delete[i]]);
	}
	i = queue_destroy(q);
	TEST_ASSERT(i == 0);	// when queue is empty, should return 0
}

/* Length for 5 data */
void test_length(void){
	int *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST test_length ***\n");
	int data[5] = {1000, 24, 34, 70, 500};
	q = queue_create();
	for (int i = 0; i < 5; i++) {	// adding 5 entries of data
		queue_enqueue(q, &data[i]);
		TEST_ASSERT(queue_length(q) == i+1);
	}

	for (int i = 0; i < 5; i++) {	// cheking all data is recived in right order
		queue_dequeue(q, (void**)&ptr);
		TEST_ASSERT(queue_length(q) == 4-i);
	}
	queue_destroy(q);
}

// test queue iterate
void iter_func(queue_t queue, void *data){
	*(int*)data += 1;
	// printing only because of error "queue not being used"
	queue_length(queue);
}



/* Iterate */
void test_iterator(void){
	int *ptr;
	queue_t q = NULL;

	// testing when q is NULl
	int i = queue_iterate(q, iter_func);
	TEST_ASSERT(i == -1);

	fprintf(stderr, "*** TEST test_iterator ***\n");
	int data[5] = {1000, 24, 34, 70, 500};

	q = queue_create();
	for (int i = 0; i < 5; i++) {	// adding 5 entries of data
		queue_enqueue(q, &data[i]);
	}

	// testing when q is not empty
	i = queue_iterate(q, iter_func);
	TEST_ASSERT(i == 0);

	//testing that iter_func behaved correctly, incrementing value:
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT((int)*ptr==1001);

	for (int i = 1; i < 5; i++) {	// checking all data is received in right order
		queue_dequeue(q, (void**)&ptr);
		TEST_ASSERT(*ptr == data[i]);
	}

	// testing when q is empty
	i = queue_iterate(q, iter_func);
	TEST_ASSERT(i == 0);
	queue_destroy(q);

}


int main(void)
{
	test_create();
	test_queue_simple();
	test_queue_2_items();
	test_queue_5_items();
	test_destroy();
	test_delete_in_one();
	test_delete();
	test_length();
	test_iterator();

	return 0;
}
