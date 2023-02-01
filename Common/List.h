#pragma once
#include<stdio.h>
#include<stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h> 
#include "../Common/Structs.h"

bool Add(socketForList** head, SOCKET data) {
	socketForList* new_node;
	new_node = (socketForList*)malloc(sizeof(socketForList));
	if (new_node == NULL) {
		return false;
	}

	new_node->acceptedSocket = data;
	new_node->next = *head;
	*head = new_node;
	return true;
}

bool Remove(socketForList** head, SOCKET sock) {
	socketForList* current = *head;
	socketForList* previous = NULL;

	if (current == NULL) {
		return false;
	}

	while (current->acceptedSocket != sock) {

		//if it is last node
		if (current->next == NULL) {
			return false;
		}
		else {
			//store reference to current link
			previous = current;
			//move to next link
			current = current->next;
		}
	}

	if (current == *head) {
		//change first to point to next link
		*head = (*head)->next;
	}
	else {
		//bypass the current link
		previous->next = current->next;
	}

	free(current);
	current = NULL;
	return true;
}


void deleteList(socketForList** head) {
	socketForList* temp = NULL;
	socketForList* current = *head;

	while (current != NULL) {
		temp = current;
		current = current->next;
		free(temp);
		temp = NULL;
	}
	*head = current;
}

bool FindInList(socketForList** head, SOCKET socket) {

	socketForList* temp = NULL;
	socketForList* current = *head;
	while (current != NULL) {
		if (current->acceptedSocket == socket)
			return true;
		current = current->next;
	}

	return false;
}
void print_list(socketForList* head) {
	if (head == NULL) {
		printf("List is empty!");
		return;
	}

	socketForList* current = head;

	while (current != NULL) {
		printf("%d\n", current->acceptedSocket);
		current = current->next;
	}

	printf("\n");
}

void CloseAllSocketsFromList(socketForList* lista) {
	int iResult;

	while (lista != NULL) {
		iResult = shutdown(lista->acceptedSocket, SD_SEND);
		if (iResult == SOCKET_ERROR)
		{
			printf("method: CloseAllSocketsFromList failed with error: %d\n", WSAGetLastError());
			closesocket(lista->acceptedSocket);
			WSACleanup();

		}
		closesocket(lista->acceptedSocket);
		lista = lista->next;
	}
}

