#pragma once
#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_QUEUE_SIZE 500
#include "Structs.h"
#include <stdio.h>
#include <ws2tcpip.h>
#include "List.h"
#include "Queue.h"
#include "HashMap.h"
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 524
#define SAFE_DELETE_HANDLE(a) if(a){CloseHandle(a);} 
#define PUBLISHER_PORT "27016"
#define SUBSCRIBER_PORT "27017"

extern bool dataReady = true;
QUEUE queue;
CRITICAL_SECTION criticalSectionForQueue, criticalSectionForPublisher, criticalSectionForSubscribers, criticalSectionForListOfSubscribers;
HANDLE FinishSignal, QueueIsFull, QueueIsEmpty, publisherIsWorking, publisherFinished;
socketForList* publisherSockets = NULL;
socketForList* subscriberSockets = NULL;
subscribers* map[map_size];

bool isTopicThere = true;

HANDLE t1, t2, t3, t4, t5, t6, t7, t8;

DWORD thread1ID, thread2ID, thread3ID, thread4ID, thread5ID, thread6ID, thread7ID, thread8ID;


char* TopicToLower(char* topic) {

	for (unsigned int i = 0; i < strlen(topic); i++) {
		topic[i] = tolower(topic[i]);
	}
	return topic;
}

void InitAllNecessaryCriticalSection() {
	InitializeCriticalSection(&criticalSectionForQueue);
	InitializeCriticalSection(&criticalSectionForPublisher);
	InitializeCriticalSection(&criticalSectionForSubscribers);
	InitializeCriticalSection(&criticalSectionForListOfSubscribers);
}

void DeleteAllNecessaryCriticalSection() {
	DeleteCriticalSection(&criticalSectionForQueue);
	DeleteCriticalSection(&criticalSectionForPublisher);
	DeleteCriticalSection(&criticalSectionForSubscribers);
	DeleteCriticalSection(&criticalSectionForListOfSubscribers);
}

bool InitializeWindowsSockets() {
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{

		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

SOCKET InitializeListenSocket(const char* port) {

	SOCKET listenSocket = INVALID_SOCKET;
	// Prepare address information structures
	addrinfo* resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     // 

	// Resolve the server address and port
	int iResult = getaddrinfo(NULL, port, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}


	// Create a SOCKET for connecting to server
						//		IPv4 address famly|stream socket | TCP protocol
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);

		WSACleanup();
		return INVALID_SOCKET;
	}

	// Setup the TCP listening socket - bind port number and local address 
	// to socket
	iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return INVALID_SOCKET;
	}

	// Since we don't need resultingAddress any more, free it
	freeaddrinfo(resultingAddress);
	//stavi u neblokirjauci rezim
	unsigned long mode = 1;
	iResult = ioctlsocket(listenSocket, FIONBIO, &mode);
	if (iResult != NO_ERROR) {
		printf("ioctlsocket failed with error: %ld\n", iResult);
		return INVALID_SOCKET;
	}
	return listenSocket;
}

void CreateAllSemaphores() {

	FinishSignal = CreateSemaphore(0, 0, 6, NULL);
	QueueIsFull = CreateSemaphore(0, 0, MAX_QUEUE_SIZE, NULL);
	QueueIsEmpty = CreateSemaphore(0, MAX_QUEUE_SIZE, MAX_QUEUE_SIZE, NULL);
	publisherIsWorking = CreateSemaphore(0, 0, 1, NULL);
	publisherFinished = CreateSemaphore(0, 0, 1, NULL);
}


void DeleteAllThreadsAndSemaphores() {
	SAFE_DELETE_HANDLE(t1);
	SAFE_DELETE_HANDLE(t2);
	SAFE_DELETE_HANDLE(t3); 
	SAFE_DELETE_HANDLE(t4);
	SAFE_DELETE_HANDLE(t5);
	SAFE_DELETE_HANDLE(t6);
	SAFE_DELETE_HANDLE(t7);
	SAFE_DELETE_HANDLE(t8);
	SAFE_DELETE_HANDLE(FinishSignal);
	SAFE_DELETE_HANDLE(QueueIsEmpty);
	SAFE_DELETE_HANDLE(QueueIsFull);
	SAFE_DELETE_HANDLE(publisherIsWorking);
	SAFE_DELETE_HANDLE(publisherFinished);
}

DWORD WINAPI FunkcijaThread1(LPVOID param) {

	SOCKET listenSocketPublisher = *(SOCKET*)param;
	SOCKET acceptedSocketPublisher = INVALID_SOCKET;
	unsigned long mode = 1;
	int lastIndex = 0;

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	fd_set readfds;
	timeval timeVal;
	timeVal.tv_sec = 5;
	timeVal.tv_usec = 0;


	

	while (WaitForSingleObject(FinishSignal, 500) == WAIT_TIMEOUT) {
		FD_ZERO(&readfds);
		FD_SET(listenSocketPublisher, &readfds);
		int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (selectResult == SOCKET_ERROR) {

			printf("select failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocketPublisher);
			WSACleanup();
			return 1;

		}
		else if (selectResult == 0) {
			//timeVal has expired without any action
			continue;

		}
		else {
			sockaddr_in clientAddr;
			int clinetAddrSize = (sizeof(struct sockaddr_in));

			acceptedSocketPublisher = accept(listenSocketPublisher, (struct sockaddr*)&clientAddr, &clinetAddrSize);

			if (acceptedSocketPublisher == INVALID_SOCKET) {

				if (WSAGetLastError() == WSAECONNRESET) {
					printf("accept failed, because timeout for client request has expired.\n");
				}
				else
				{
					printf("accept failed with error: %d\n", WSAGetLastError());
				}

			}
			else {
				//client is succesfully accepted, we need to put socket into a nonblocking mode
				if (ioctlsocket(acceptedSocketPublisher, FIONBIO, &mode) != 0) {
					printf("ioctlsocket failed with error.");
					continue;
				}
				else {
					//we need to add socket into a list of accepted sockets
					EnterCriticalSection(&criticalSectionForPublisher);
					Add(&publisherSockets, acceptedSocketPublisher);
					LeaveCriticalSection(&criticalSectionForPublisher);
					ReleaseSemaphore(publisherIsWorking, 1, NULL); 
					lastIndex++;
					printf("New Publisher request accpeted (%d). Client address: %s : %d\n", lastIndex, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
				}
			}

		}
	}

	ReleaseSemaphore(publisherFinished, 1, NULL);
	// cleanup
	closesocket(listenSocketPublisher);

	WSACleanup();

	return 0;
}

DWORD WINAPI FunkcijaThread2(LPVOID param) {

	int iResult;

	char recvbuf[DEFAULT_BUFLEN];

	fd_set readfds;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	DATA* result;
	socketForList* current = NULL;
	bool finish = false;
	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	const int numOfSemaphore = 2;
	HANDLE semaphores[numOfSemaphore] = { publisherIsWorking, publisherFinished };

	while (WaitForSingleObject(FinishSignal, 500) == WAIT_TIMEOUT) {

		current = publisherSockets; 
		while (current == NULL) {
			
			if (WaitForMultipleObjects(numOfSemaphore, semaphores, FALSE, INFINITE) == WAIT_OBJECT_0 + 1) {
				finish = true;
				break;
			}
			current = publisherSockets;
		}

		if (finish) 
			break;

		FD_ZERO(&readfds);

		EnterCriticalSection(&criticalSectionForPublisher);
		while (current != NULL)
		{
			FD_SET(current->acceptedSocket, &readfds);
			current = current->next;
		}
		LeaveCriticalSection(&criticalSectionForPublisher);


		int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (selectResult == SOCKET_ERROR) {

			printf("select failed with error: %d\n", WSAGetLastError());
			
		}
		else if (selectResult == 0) {
			
			continue;
		}
		else {
			EnterCriticalSection(&criticalSectionForPublisher);
			current = publisherSockets; 
			while (current != NULL)
			{
				if (FD_ISSET(current->acceptedSocket, &readfds)) {


					// Receive data until the client shuts down the connection
					iResult = recv(current->acceptedSocket, recvbuf, (int)(sizeof(DATA)), 0);
					if (iResult > 0)
					{
						result = (DATA*)recvbuf;
						
						WaitForSingleObject(QueueIsEmpty, INFINITE);
						EnterCriticalSection(&criticalSectionForQueue);

						Enqueue(&queue, *result);
						
						

						LeaveCriticalSection(&criticalSectionForQueue);
						current = current->next; 
						ReleaseSemaphore(QueueIsFull, 1, NULL);
					}
					else if (iResult == 0)
					{
						// connection was closed gracefully
						printf("Connection with publisher closed.\n");
						closesocket(current->acceptedSocket);

						socketForList* socketForRemove = current;
						current = current->next;  
						Remove(&publisherSockets, socketForRemove->acceptedSocket);

					}
					else
					{
						// there was an error during recv
						
						printf("publisher recv failed with error: %d\n", WSAGetLastError());
						closesocket(current->acceptedSocket);

						socketForList* socketForRemove = current;
						current = current->next;  
						Remove(&publisherSockets, socketForRemove->acceptedSocket);


					}

				}
				else current = current->next;

			}
			LeaveCriticalSection(&criticalSectionForPublisher);
		}
		

	}

	WSACleanup();
	return 0;
}

DWORD WINAPI FunkcijaThread3(LPVOID param) {

	SOCKET listenSocketSubscriber = *(SOCKET*)param;
	SOCKET acceptedSocketSubscriber = INVALID_SOCKET;

	short lastIndexSub = 0;
	

	fd_set readfds;
	timeval timeVal;
	timeVal.tv_sec = 5;
	timeVal.tv_usec = 0;
	unsigned long mode = 1;

	int IResultSubscriber;
	char recvbuf[DEFAULT_BUFLEN];

	socketForList* currentSocket = NULL;

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	

	while (WaitForSingleObject(FinishSignal, 1000) == WAIT_TIMEOUT) {
		currentSocket = subscriberSockets;

		FD_ZERO(&readfds);

		FD_SET(listenSocketSubscriber, &readfds);
		while (currentSocket != NULL)
		{
			FD_SET(currentSocket->acceptedSocket, &readfds);
			currentSocket = currentSocket->next;
		}


		int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (selectResult == SOCKET_ERROR) {

			printf("select function failed with error: %d\n", WSAGetLastError());

		}
		else if (selectResult == 0) {
			//timeVal has expired and no action has happened
						
			continue;

		}
		else {
			if (FD_ISSET(listenSocketSubscriber, &readfds)) {
				sockaddr_in clientAddr;
				int clinetAddrSize = (sizeof(struct sockaddr_in));

				acceptedSocketSubscriber = accept(listenSocketSubscriber, (struct sockaddr*)&clientAddr, &clinetAddrSize);

				if (acceptedSocketSubscriber == INVALID_SOCKET) {

					if (WSAGetLastError() == WSAECONNRESET) {
						printf("accept failed, because timeout for client request has expired.\n");
					}

				}
				else {
					//client is succesfully accepted, we need to put socket in non blocking mode
					if (ioctlsocket(acceptedSocketSubscriber, FIONBIO, &mode) != 0) {
						printf("ioctlsocket failed with error.");

					}
					else {
						//add accepted socket into a list
						Add(&subscriberSockets, acceptedSocketSubscriber);

						lastIndexSub++;
						printf("New Subscriber request accpeted (%d). Client address: %s : %d\n", lastIndexSub, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
					}
				}
			}
			else {
				
				currentSocket = subscriberSockets;
				while (currentSocket != NULL)
				{
					if (FD_ISSET(currentSocket->acceptedSocket, &readfds)) {

						// Receive data until the client shuts down the connection
						IResultSubscriber = recv(currentSocket->acceptedSocket, recvbuf, DEFAULT_BUFLEN, 0);
						if (IResultSubscriber > 0)
						{
							recvbuf[IResultSubscriber] = '\0';

							strcpy_s(recvbuf, TopicToLower(recvbuf));
							//printf("Message received from subscriber: %s.\n", recvbuf);


							subscribers* temp = FindSubscriberInMap(map, recvbuf);
							if (temp == NULL) {
								subscribers* newSubscriber = CreateSubscriber(recvbuf);
								if (newSubscriber == NULL)
									printf("Failed creating new topic and subscriber!\n");
								else {
									EnterCriticalSection(&criticalSectionForSubscribers);
									if (Add(&newSubscriber->socketsConnectedToTopic, currentSocket->acceptedSocket)) {

										printf("Subscriber has been added in a list for topic!\n");
										if (AddToMap(map, newSubscriber))
											printf("Successful added to map!\n");
										else  
											printf("Adding into the map has failed with an error...\n");
									}
									else 
										printf("Subscriber has not been added into list...");
									LeaveCriticalSection(&criticalSectionForSubscribers);
								}
							}
							else {
								EnterCriticalSection(&criticalSectionForSubscribers);
								if (!FindInList(&temp->socketsConnectedToTopic, currentSocket->acceptedSocket)) {

									if (Add(&temp->socketsConnectedToTopic, currentSocket->acceptedSocket)) 
										printf("Subscriber has been added into list!\n");
									else 
										printf("Subscriber has not been added into list...\n");
								}
								else 
									printf("Subscriber has already been subscribed to that topic!\n");
								LeaveCriticalSection(&criticalSectionForSubscribers);
							}

							//printMap(map);
							currentSocket = currentSocket->next; 
						}
						else if (IResultSubscriber == 0)
						{
							// connection was closed gracefully
							printf("Connection with subscriber closed.\n");
							closesocket(currentSocket->acceptedSocket);


							EnterCriticalSection(&criticalSectionForSubscribers);
							DeleteSubscriber(map, currentSocket->acceptedSocket);
							LeaveCriticalSection(&criticalSectionForSubscribers);
							lastIndexSub--;

							socketForList* socketForRemove = currentSocket;
							currentSocket = currentSocket->next; 

							
							Remove(&subscriberSockets, socketForRemove->acceptedSocket);
						
							

						}
						else
						{
							// there was an error during recv
							printf("subscriber recv failed with error: %d\n", WSAGetLastError());
							closesocket(currentSocket->acceptedSocket);

							EnterCriticalSection(&criticalSectionForSubscribers);
							DeleteSubscriber(map, currentSocket->acceptedSocket); 
							LeaveCriticalSection(&criticalSectionForSubscribers);
							lastIndexSub--;

							socketForList* socketForRemove = currentSocket;
							currentSocket = currentSocket->next;
							
							Remove(&subscriberSockets, socketForRemove->acceptedSocket);
							


						}

					}
					else currentSocket = currentSocket->next;

				}
				

			}
		}
	}


	closesocket(listenSocketSubscriber);
	WSACleanup();
	return 0;
}

DWORD WINAPI FunkcijaThreadPool(LPVOID param) {

	int iResult;
	const int numOfSemaphores = 2;

	HANDLE semaphores[numOfSemaphores] = { FinishSignal,QueueIsFull };
	int id = (int)param;
	while (WaitForMultipleObjects(numOfSemaphores, semaphores, FALSE, INFINITE) == WAIT_OBJECT_0 + 1) {
		
		socketForList* listOfSubscribers = NULL;
		DATA result;

		EnterCriticalSection(&criticalSectionForQueue);
		printf("\tID: %d\n", id);
		if (Dequeue(&queue, &result)) {
			printf("====Removed from queue=====\n");
			printf("Topic: %s \n", result.topic);
			printf("Message: %s\n\n", result.message);
			printf("===========================\n");
		}
		else {
			printf("Unsuccessful removing from queue\n");
		}
		LeaveCriticalSection(&criticalSectionForQueue);

		EnterCriticalSection(&criticalSectionForSubscribers);
		subscribers* temp = FindSubscriberInMap(map, result.topic);
		printMap(map);
		LeaveCriticalSection(&criticalSectionForSubscribers);

		

		if (temp != NULL) {

			EnterCriticalSection(&criticalSectionForSubscribers);

			listOfSubscribers = temp->socketsConnectedToTopic;

			while (listOfSubscribers != NULL) {

				iResult = send(listOfSubscribers->acceptedSocket, (char*)&result, (int)(sizeof(DATA)), 0);
				
				listOfSubscribers = listOfSubscribers->next;

			}
			LeaveCriticalSection(&criticalSectionForSubscribers);
		}

		ReleaseSemaphore(QueueIsEmpty, 1, NULL);
	}

	return 0;
}


void CreateAllThreads(SOCKET* listenSocketPublisher, SOCKET* listenSocketSubscriber) {
	t1 = CreateThread(NULL, 0, &FunkcijaThread1, listenSocketPublisher, 0, &thread1ID);
	t2 = CreateThread(NULL, 0, &FunkcijaThread2, NULL, 0, &thread2ID);
	t3 = CreateThread(NULL, 0, &FunkcijaThread3, listenSocketSubscriber, 0, &thread3ID);

	//THREAD POOL
	t4 = CreateThread(NULL, 0, &FunkcijaThreadPool, (LPVOID)0, 0, &thread4ID);
	t5 = CreateThread(NULL, 0, &FunkcijaThreadPool, (LPVOID)0, 0, &thread5ID);
	t6 = CreateThread(NULL, 0, &FunkcijaThreadPool, (LPVOID)0, 0, &thread6ID);
	t7 = CreateThread(NULL, 0, &FunkcijaThreadPool, (LPVOID)0, 0, &thread7ID);
	t8 = CreateThread(NULL, 0, &FunkcijaThreadPool, (LPVOID)0, 0, &thread8ID);
	
}

