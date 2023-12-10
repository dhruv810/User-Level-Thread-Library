#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#include "queue.h"

struct queue_node {
	void *data;
	struct queue_node *next;
};

struct queue {
	struct queue_node *head;
	struct queue_node *tail;
	int length;
};

queue_t queue_create(void){
	struct queue *newQueue = malloc(sizeof(struct queue));
	newQueue->head=NULL;
	newQueue->tail=NULL;
	newQueue->length = 0;
	return newQueue;
}

int queue_destroy(queue_t queue){
	if(queue == NULL || queue->head != NULL){ //cannot destroy non-empty or nonexistent queue
		return -1;
	}
	free(queue);
	return 0;
}

int queue_enqueue(queue_t queue, void *data){
	if(queue == NULL || data==NULL){
		return -1;
	}
	struct queue_node *newNode = malloc(sizeof(struct queue_node));
	if (newNode == NULL){
		return -1;
	}
	newNode->data = data;
	if(queue->length == 0){
		queue->tail = newNode;
		queue->head = newNode;
	} else {
		queue->tail->next = newNode;
		queue->tail = newNode;
	}
	queue->tail->next = NULL;
	queue->length++;
	return 0;
}

int queue_dequeue(queue_t queue, void **data){
	if(queue == NULL || queue->head == NULL || queue->head->data == NULL){
		return -1;
	}
	struct queue_node *thisNode = queue->head;
	*data = thisNode->data;
	queue->head = thisNode->next;
	free(thisNode);
	if (queue->head == NULL){
		queue->tail = NULL; //if queue is empty, don't leave old tail pointer dangling.
	}
	queue->length--;
	return 0;
}

int queue_delete(queue_t queue, void *data){
	if(queue == NULL || data == NULL){
		return -1;
	}
	struct queue_node *thisNode = queue->head;
	struct queue_node *lastNode = NULL;
	while(thisNode != NULL){
		if (thisNode->data == data){
			if (queue->head == thisNode){
				queue->head = queue->head->next;
			} else {
				if (lastNode != NULL){
					lastNode->next=thisNode->next;
				}
			}
			if (queue->tail == thisNode){
				queue->tail=NULL;
			}
			queue->length--;
			free(thisNode);
			return 0;
		}
		lastNode = thisNode;
		thisNode = thisNode->next;
	}
	return -1;
}

int queue_iterate(queue_t queue, queue_func_t func){
	if(queue == NULL || func == NULL){
		return -1;
	}
	struct queue_node *thisNode = queue->head;
	while(thisNode != NULL){
		func(queue, thisNode->data);
		thisNode = thisNode->next;
	}
	return 0;
}

int queue_length(queue_t queue){
	if(queue == NULL){
		return -1;
	}
	return queue->length;
}

