#include<winsock2.h>
#include<iostream>
#include<string>
#include<chrono>
#include<thread>

#include "netClient.h"


using namespace std;
#pragma comment(lib, "ws2_32.lib")

int netClientTestWetherServerAlive()
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 1;
	}

	
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET){
		cout << "Socket error" << endl;
		return 1;
	}
	sockaddr_in sock_in;
	sock_in.sin_family = AF_INET;
	sock_in.sin_port = htons(8888);
	sock_in.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (connect(clientSocket, (sockaddr*)&sock_in, sizeof(sock_in)) == SOCKET_ERROR){
		cout << "Connect error" << endl;
		return 1;
	}

	const char * msg = "echo request";
	const char * expectedMsg = "echo response";

	const int nTotalConsectiveRetryTime = 3;
	int nAlreadyConsectiveRetryTime = 0;
	
	while (nAlreadyConsectiveRetryTime<nTotalConsectiveRetryTime) {
		nAlreadyConsectiveRetryTime++;
		send(clientSocket, msg, strlen(msg), 0);
		cout << "client send:" << msg << endl;

		char revdata[100];
		int num = recv(clientSocket, revdata, 100, 0);
		if (num > 0){
			revdata[num] = '\0';
		
			if (0 == strcmp(revdata, expectedMsg)) {
				cout << "client recv:" << revdata << endl;
				nAlreadyConsectiveRetryTime = 0;
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				continue;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	closesocket(clientSocket);

	

	WSACleanup();

	return 0;
}

