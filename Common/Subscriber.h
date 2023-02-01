#pragma once
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define DEFAULT_BUFLEN 524
#define DEFAULT_PORT 27017
#define SAFE_DELETE_HANDLE(a) if(a){CloseHandle(a);} 
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "Structs.h"
#include <string.h>

bool InitializeWindowsSockets();
bool Connect();
void Subscribe(void* topic);
char* TopicToLower(char* topic);
char** separate_string(char* str, char separator, int* parts_count);
HANDLE endSignal, endOfThread;
CRITICAL_SECTION criticalSectionForInput;
DWORD thread1ID;

SOCKET connectSocket = INVALID_SOCKET;

bool Connect() {
	// create a socket
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return false;
	}

	// create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(DEFAULT_PORT);
	// connect to server specified in serverAddress and socket connectSocket
	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}
	return true;
}

void Subscribe(void* topic) {

	char* message = (char*)topic;

	int iResult = send(connectSocket, message, strlen(message), 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
	}


}

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}



char** separate_string(char* str, char separator, int* parts_count) {
	char** parts = (char**)malloc(sizeof(char*) * strlen(str));
	char* current_part = (char*)malloc(sizeof(char) * strlen(str));
	int current_part_len = 0;

	int parts_index = 0;
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == separator) {
			current_part[current_part_len] = '\0';
			parts[parts_index] = current_part;

			current_part = (char*)malloc(sizeof(char) * strlen(str));
			current_part_len = 0;
			parts_index++;
		}
		else {
			current_part[current_part_len] = str[i];
			current_part_len++;
		}
	}
	current_part[current_part_len] = '\0';
	parts[parts_index] = current_part;

	*parts_count = parts_index + 1;
	return parts;
}

void HeaderForEnteringTopic() {
	printf("Format of the topic is signal.type.num.\n");
	printf("For signal you can choose STATUS or ANALOG.\n\t~If you choose STATUS it must be FUSE or BREAKER.\n\t~If you choose ANALOG it must be SEC_A or SEC_V.\n");
	printf("\t~INSTEAD OF TYPE YOU CAN ENTER '*' FOR BOTH TYPES OF SIGNAL\n");
	printf("NUM is the number of device.\n");
	printf("Please enter your topic: \n");
}


char* TopicToLower(char* topic) {

	for (unsigned int i = 0; i < strlen(topic); i++) {
		topic[i] = tolower(topic[i]);
	}
	return topic;
}

DWORD WINAPI FunkcijaThread1(LPVOID param) {
	SOCKET connectSocket = *(SOCKET*)param;
	while (WaitForSingleObject(endOfThread, 500) == WAIT_TIMEOUT) {

		if (_kbhit()) {
			EnterCriticalSection(&criticalSectionForInput);
			char ch = _getch();
			LeaveCriticalSection(&criticalSectionForInput);
			if (ch == 's' || ch == 'S') {

				bool resultForsignal = true;
				bool resultForType1 = true;
				bool resultForType2 = true;
				bool resultForNum = true;
				char topic[100];
				char topicToLower[100];
				int parts_count = 0;
				char signal[100];
				char num[100];

				HeaderForEnteringTopic();
				fgets(topic, sizeof(topic), stdin);
				strcpy(topicToLower, TopicToLower(topic));
				topicToLower[strcspn(topicToLower, "\n")] = 0;


				EnterCriticalSection(&criticalSectionForInput);
				char** parts = separate_string(topicToLower, '.', &parts_count);
				if (parts_count != 3) {
					printf("You have to enter exactly 3 parts of topic\n");
				}
				else {
					for (int i = 0; i < parts_count; i++) {
						if (strcmp(parts[0], "status") != 0 && strcmp(parts[0], "analog") != 0) {

							resultForsignal = false;
							break;
						}
						else {
							strcpy(signal, parts[0]);


						}

						if (strcmp(signal, "status") == 0 && strcmp(parts[1], "fuse") != 0 && strcmp(parts[1], "breaker") != 0 && strcmp(parts[1], "*") != 0) {

							resultForType1 = false;
							break;
						}
						else if (strcmp(signal, "analog") == 0 && strcmp(parts[1], "sec_a") != 0 && strcmp(parts[1], "sec_v") != 0 && strcmp(parts[1], "*") != 0) {

							resultForType2 = false;
							break;
						}

						strcpy(num, parts[2]);
						int length = strlen(num);
						if (length == 1) {
							if (!isdigit(num[0]))
							{
								resultForNum = false;
							}
						}
						else {
							for (i = 0; i < length; i++)
								if (!isdigit(num[i]))
								{
									resultForNum = false;

								}

						}

					}
				}
				if (resultForsignal == false) {
					printf("SIGNAL must be STATUS or ANALOG\n");
				}
				else if (resultForType1 == false) {
					printf("For signal STATUS type must be FUSE or BREAKER\n");
					
				}
				else if (resultForType2 == false) {
					printf("For signal ANALOG type must be SEC_A or SEC_V\n");
					
				}
				else if (resultForNum == false) {
					printf("For NUM you must enter a NUMBER\n");
				}
				else {
					Subscribe((void*)topicToLower);
				}

				
				
			}
			LeaveCriticalSection(&criticalSectionForInput);

		}
	}
	closesocket(connectSocket);
	WSACleanup();
	return 0;
}





