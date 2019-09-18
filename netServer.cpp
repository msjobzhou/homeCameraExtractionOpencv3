#include<winsock2.h>
#include<iostream>
#include<string>
using namespace std;
#pragma comment(lib,"ws2_32.lib")

int netServer(){

	//初始化DLL
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsdata;
	if (WSAStartup(sockVersion, &wsdata) != 0)
	{
		return 1;
	}

	//创建套接字
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
	{
		cout << "Socket error" << endl;
		return 1;
	}
	int nNetTimeout = 1100;//1秒，
	//设置接收超时，注意这个设置一定要在bind和accept等操作之前，且此参数设置仅适用于阻塞模式
	setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));


	//绑定套接字
	sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(8888);
	sockAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	if (bind(serverSocket, (sockaddr*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR){
		cout << "Bind error" << endl;
		return 1;
	}

	//开始监听
	if (listen(serverSocket, 10) == SOCKET_ERROR){
		cout << "Listen error" << endl;
		return 1;
	}

	SOCKET clientSocket;
	sockaddr_in client_sin;
	char msg[100];//存储传送的消息
	int len = sizeof(client_sin);
	while (true){
		printf("等待连接...\n");
		clientSocket = accept(serverSocket, (sockaddr*)&client_sin, &len);
		if (clientSocket == INVALID_SOCKET){
			cout << "Accept error" << endl;
			WSACleanup();
			continue;
		}
		printf("接受到一个连接：%s \r\n", inet_ntoa(client_sin.sin_addr));
		const int nTotalConsectiveRecvFailTime = 60;
		int nAlreadyConsectiveRecvFailTime = 0;
		while (nAlreadyConsectiveRecvFailTime < nTotalConsectiveRecvFailTime) {
			
			int retVal = recv(clientSocket, msg, 100, 0);
			if (retVal == SOCKET_ERROR)
			{
				int err = WSAGetLastError();
				if (err == WSAETIMEDOUT) {
					printf("recv failed due to timeout\n");
					nAlreadyConsectiveRecvFailTime++;
					continue;
				}
				else if (err == WSAENETDOWN) {
					closesocket(clientSocket);
					printf("clientSocket already closed\n");
					break;
				}
			}

			msg[retVal] = '\0';
			const char * expectedMsg = "echo request";
			cout << "server recv: " << msg << endl;
			if (0 == strcmp(msg, expectedMsg)) {
				nAlreadyConsectiveRecvFailTime = 0;
			}
			const char * sendData = "echo response";
			send(clientSocket, sendData, strlen(sendData), 0);
		}
		closesocket(clientSocket);
	}
	printf("server process exit\n");
	closesocket(serverSocket);
	WSACleanup();



	return 0;
}

