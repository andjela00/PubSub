#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <WinSock2.h>
#include "List.h"
#define _CRT_SECURE_NO_WARNINGS
#define map_size 16

void CreateMap(subscribers** map) {
	for (int i = 0; i < map_size; i++) {
		map[i] = NULL;

	}
}

unsigned int HashFunction(const char* topic) {

	int length = strlen(topic);
	unsigned int hash_value = 0;
	for (int i = 0; i < length; i++) {

		hash_value += topic[i];
		hash_value = (hash_value * topic[i]) % map_size;
	}

	return hash_value;

}

bool AddToMap(subscribers** map, subscribers* subscriber) {
	if (subscriber == NULL)
		return false;
	int index = HashFunction(subscriber->topic);

	subscriber->next = map[index];
	map[index] = subscriber;

	return true;
}

subscribers* CreateSubscriber(const char* topic) {

	subscribers* newSubscriber = (subscribers*)malloc(sizeof(subscribers));
	if (newSubscriber == NULL)
		return NULL;

	strcpy_s(newSubscriber->topic, topic);
	newSubscriber->socketsConnectedToTopic = NULL;
	return newSubscriber;
}

subscribers* FindSubscriberInMap(subscribers** map, const char* topic) {
	int index = HashFunction(topic);
	subscribers* temp = map[index];
	while (temp != NULL && strcmp(temp->topic, topic) != 0) {
		temp = temp->next;
	}
	return temp;
}

void DeleteSubscriber(subscribers** map, SOCKET socket) {

	for (int i = 0; i < map_size; i++) {
		if (map[i] == NULL)
			continue;
		else {
			subscribers* temp = map[i];
			while (temp != NULL)
				if (Remove(&temp->socketsConnectedToTopic, socket)) {
					printf("Deleting subscriber..\n");
				}
			temp = temp->next;
		}
	}
}


bool DeleteFromMap(subscribers** map, char* topic) {
	int index = HashFunction(topic);
	subscribers* temp = map[index];
	subscribers* prev = NULL;
	while (temp != NULL && strcmp(temp->topic, topic) != 0) {
		prev = temp;
		temp = temp->next;
	}
	if (temp == NULL) {
		return false;
	}
	if (prev == NULL) {
		map[index] = temp->next;
	}
	else {
		prev->next = temp->next;
	}

	deleteList(&temp->socketsConnectedToTopic);
	free(temp);
	temp = NULL;
	return true;
}

void DeleteMap(subscribers** map) {
	for (int i = 0; i < map_size; i++) {
		if (map[i] == NULL) continue;
		else {
			subscribers* temp = map[i];
			while (map[i] != NULL) {
				DeleteFromMap(map, map[i]->topic);
			}
		}
	}
}

void printMap(subscribers** map) {
	printf("====================HashMap=============================\n");
	for (int i = 0; i < map_size; i++) {
		if (map[i] == NULL) {
			printf("%i \t EMPTY\n", i);
		}
		else {
			printf("%i\t", i);
			subscribers* temp = map[i];
			while (temp != NULL)
			{
				printf("-----------TOPIC : %s-------------\n\tList of subscribers that are connected to topic:\n  ", temp->topic);
				socketForList* connectedSockets = temp->socketsConnectedToTopic;
				while (connectedSockets != NULL) {
					printf("\t\t\t- %d -\n ", connectedSockets->acceptedSocket);
					connectedSockets = connectedSockets->next;
				}
				printf("\tEND OF LIST FOR TOPIC : %s\n", temp->topic);
				printf("\t\n_______________________________________________\n");
				temp = temp->next;
			}
			printf("\n");
		}
	}
	printf("========================================================\n");
}