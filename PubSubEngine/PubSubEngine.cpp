// PubSubEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <conio.h>
#include "../Common/PubSubEngine.h"
#include <iostream>


int main(void){
	// Socket used for listening for new clients 
	SOCKET listenSocketPublisher = INVALID_SOCKET;
	SOCKET listenSocketSubscriber = INVALID_SOCKET;
	int iResult, IResultSubscriber;



	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	listenSocketPublisher = InitializeListenSocket(PUBLISHER_PORT);
	if (listenSocketPublisher == SOCKET_ERROR || listenSocketPublisher == INVALID_SOCKET) return 1;
	listenSocketSubscriber = InitializeListenSocket(SUBSCRIBER_PORT);
	if (listenSocketSubscriber == SOCKET_ERROR || listenSocketSubscriber == INVALID_SOCKET) return 1;

	// Set listenSocket in listening mode
	iResult = listen(listenSocketPublisher, SOMAXCONN);
	IResultSubscriber = listen(listenSocketSubscriber, SOMAXCONN);

	if (iResult == SOCKET_ERROR || IResultSubscriber == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocketPublisher);
		closesocket(listenSocketSubscriber);
		WSACleanup();
		return 1;
	}

	printf("Server initialized, waiting for clients.\n");

	InitializeQUEUE(&queue);
	CreateMap(map);
	InitAllNecessaryCriticalSection();

	CreateAllSemaphores();

	CreateAllThreads(&listenSocketPublisher, &listenSocketSubscriber);

	if (!t1 || !t2 || !t3 || !t4 || !t5 || !t6 || !t7 || !t8) {

		ReleaseSemaphore(FinishSignal, 6, NULL);
	}

	while (true) {

		if (_kbhit()) {
			char c = _getch();
			if (c == 'X') {
				ReleaseSemaphore(FinishSignal, 6, NULL);
				break;
			}
		}
	}

	if (t1) {
		WaitForSingleObject(t1, INFINITE);
	}
	if (t2) {
		WaitForSingleObject(t2, INFINITE);
	}
	if (t3) {
		WaitForSingleObject(t3, INFINITE);
	}
	if (t4) {
		WaitForSingleObject(t4, INFINITE);
	}
	if (t5) {
		WaitForSingleObject(t5, INFINITE);
	}
	if (t6) {
		WaitForSingleObject(t6, INFINITE);
	}
	if (t7) {
		WaitForSingleObject(t7, INFINITE);
	}
	if (t8) {
		WaitForSingleObject(t8, INFINITE);
	}

	DeleteAllThreadsAndSemaphores();
	DeleteAllNecessaryCriticalSection();

	ClearQueue(&queue);

	CloseAllSocketsFromList(publisherSockets);
	CloseAllSocketsFromList(subscriberSockets);

	DeleteMap(map);

	deleteList(&publisherSockets);
	deleteList(&subscriberSockets);

	closesocket(listenSocketPublisher);
	closesocket(listenSocketSubscriber);
	listenSocketPublisher = INVALID_SOCKET;
	listenSocketSubscriber = INVALID_SOCKET;
	WSACleanup();

	return 0;

}


