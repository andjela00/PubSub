#pragma once
#include <WinSock2.h>


typedef struct data {
	char topic[30];
	char message[500];
}DATA;

typedef struct queue {
	struct node* front;
	struct node* back;
} QUEUE;

typedef struct node {
	struct data data;
	struct node* next;
} NODE;





typedef struct socketForList {
	SOCKET acceptedSocket;
	struct socketForList* next;
}SOCKET_FOR_LIST;




typedef struct subscribers {
	char topic[30];
	socketForList* socketsConnectedToTopic;
	struct subscribers* next;
}SUBSCRIBERS;


