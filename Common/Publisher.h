
#pragma once
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define DEFAULT_BUFLEN 524
#define DEFAULT_PORT 27016

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "Structs.h"


char* TopicToLower(char* topic);
char** separate_string(char* str, char separator, int* parts_count);
bool InitializeWindowsSockets();
bool Connect();

SOCKET connectSocket = INVALID_SOCKET;

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
		return false;
	}
	

	return true;
}



int Publish(void* topic, void* message) {
	// message to send
	DATA result;
	strcpy_s(result.topic, (char*)topic);
	strcpy_s(result.message, (char*)message);

	// Send an prepared message with null terminator included
	int iResult = send(connectSocket, (char*)&result, (int)sizeof(DATA), 0);


	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return -1;
	}
	return iResult;
}

char* TopicToLower(char* topic) {

	for (unsigned int i = 0; i < strlen(topic); i++) {
		topic[i] = tolower(topic[i]);
	}
	return topic;
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
	printf("For signal you can choose STATUS or ANALOG.\n\t If you choose STATUS it must be FUSE or BREAKER.\n\tIf you choose ANALOG it must be SEC_A or SEC_V.\n");
	printf("\t INSTEAD OF TYPE YOU CAN ENTER '*' FOR BOTH TYPES OF SIGNAL\n");
	printf("NUM is the number of device.\n");
	printf("Please enter your topic: \n");
}