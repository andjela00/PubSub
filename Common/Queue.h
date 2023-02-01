#pragma once
#include<stdio.h>
#include <stdlib.h>
#include "../Common/Structs.h"

void InitializeQUEUE(QUEUE* queue) {
	queue->front = NULL;
	queue->back = NULL;
}

void Enqueue(QUEUE* queue, DATA data) {
	struct node* newOne = (struct node*)malloc(sizeof(struct node));
	newOne->data = data;
	newOne->next = NULL;

	if (queue->back != NULL) {
		queue->back->next = newOne;
	}

	queue->back = newOne;

	if (queue->front == NULL) {
		queue->front = newOne;
	}
}

bool Dequeue(QUEUE* queue, DATA* retVal) {
	if (queue->front == NULL) {
		printf("Struct queue is empty!\n");
		return false;
	}

	NODE* temp = queue->front;
	*retVal = temp->data;

	queue->front = queue->front->next;
	if (queue->front == NULL) {
		queue->back = NULL;
	}

	free(temp);
	return true;
}

void ShowQueue(QUEUE* queue) {
	NODE* current = queue->front;

	while (current != NULL) {
		printf("%s %s\n", current->data.topic, current->data.message);
		current = current->next;
	}

}

void ClearQueue(QUEUE* queue) {
	DATA retVal;
	while (Dequeue(queue, &retVal))
	{
		continue;
	}
}

bool findInQueue(QUEUE queue, char* topic) {
	NODE* current = queue.front;
	while (current != NULL) {
		if (strcmp(current->data.topic, topic) == 0) {
			return true;
		}
		current = current->next;
	}
	return false;
}

