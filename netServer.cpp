#include<winsock2.h>
#include<iostream>
#include<string>
#include <chrono>

#include "test.h"
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
/*增加超时机制，如果超过一定的时间，对端client没有发送消息，则关闭socket*/
int netServerSelect(){
	//服务器端已经accept的连接上连续未收到任何数据的最长时间，超过这个时间服务器端将主动关闭对应连接
	const int maxIdleConnectionSeconds = 60;
	//整个服务器端连续未收到任何accept以及已建立连接上未收到任何数据的最长时间
	const int maxServerIdleConsecutiveSeconds = 120; 

	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsdata;
	if (WSAStartup(sockVersion, &wsdata) != 0)
	{
		return 1;
	}

	const int MYPORT = 8888;   // the port users will be connecting to
	const int BACKLOG = 5;     // how many pending connections queue will hold
	const int BUF_SIZE = 200;

	int fd_A[BACKLOG] = {0};    // accepted connection fd
	int connectionRemainTime[BACKLOG] = { maxIdleConnectionSeconds };//maxIdleConnectionSeconds for every connection
	int serverMaxRemainIdleTime = maxServerIdleConsecutiveSeconds;
	int conn_amount;    // current connection amount
	int sock_fd, new_fd;  // listen on sock_fd, new connection on new_fd
	SOCKADDR_IN server_addr;    // server address information
	SOCKADDR_IN client_addr; // connector's address information

	int sin_size;
	int yes = 1;
	char buf[BUF_SIZE];
	int ret;
	int i;

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		perror("socket error");
		return -1;
	}

	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(int)) == -1) {
		perror("setsockopt error");
		return -1;
	}
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;         // host byte order
	server_addr.sin_port = htons(MYPORT);     // short, network byte order
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // automatically fill with my IP
	//memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

	if (bind(sock_fd, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		perror("bind error");
		return -1;
	}

	if (listen(sock_fd, BACKLOG) == SOCKET_ERROR) {
		perror("listen error");
		return -1;
	}

	printf("listen port %d\n", MYPORT);
	fd_set fdsr;
	int maxsock;
	TIMEVAL tv;
	conn_amount = 0;
	sin_size = sizeof(client_addr);
	maxsock = sock_fd;
	chrono::steady_clock::time_point lastTP = chrono::steady_clock::now();
	chrono::steady_clock::time_point currentTP;
	chrono::duration<double> deltaSeconds;
	while (1) {
		// initialize file descriptor set
		FD_ZERO(&fdsr);
		FD_SET(sock_fd, &fdsr);

		// timeout setting
		tv.tv_sec = 10;
		tv.tv_usec = 0;

		// add active connection to fd set
		for (i = 0; i < BACKLOG; i++) {
			if (fd_A[i] != 0) {
				FD_SET(fd_A[i], &fdsr);
			}
		}

		ret = select(0, &fdsr, NULL, NULL, &tv);

		if (ret == SOCKET_ERROR) {
			perror("select error");
			break;
		}
		//select超时场景
		else if (ret == 0) {
			currentTP = chrono::steady_clock::now();
			deltaSeconds = chrono::duration_cast<chrono::duration<double>>(currentTP - lastTP);
			lastTP = currentTP;
			printf("timeout error\n");
			for (i = 0; i < BACKLOG; i++) {
				if (fd_A[i] != 0) {
					connectionRemainTime[i] -= (int)deltaSeconds.count();
					if (connectionRemainTime[i] <= 0) {
						closesocket(fd_A[i]);
						FD_CLR(fd_A[i], &fdsr);
						fd_A[i] = 0;
						//重置连接剩余时间
						connectionRemainTime[i] = maxIdleConnectionSeconds;
						conn_amount--;
						//待增加启动对应监控程序代码
						test_createProcess();
					}
				}
			}
			serverMaxRemainIdleTime -= (int)deltaSeconds.count();
			if (serverMaxRemainIdleTime <= 0) {
				//待增加启动对应监控程序代码，启动所有需要监控的程序
				test_createProcess();
			}
			continue;
		}
		
		currentTP = chrono::steady_clock::now();
		deltaSeconds = chrono::duration_cast<chrono::duration<double>>(currentTP - lastTP);
		lastTP = currentTP;
		// check every fd in the set
		for (i = 0; i < BACKLOG; i++) {
			if (fd_A[i] == 0)
				continue;

			if (FD_ISSET(fd_A[i], &fdsr)) {
				connectionRemainTime[i] = maxIdleConnectionSeconds;
				ret = recv(fd_A[i], buf, sizeof(buf), 0);
				if (ret <= 0) {        // client close
					printf("client[%d] close\n", i);
					closesocket(fd_A[i]);
					FD_CLR(fd_A[i], &fdsr);
					fd_A[i] = 0;
					//重置连接剩余时间
					connectionRemainTime[i] = maxIdleConnectionSeconds;
					conn_amount--;
					//待增加启动对应监控程序代码
					test_createProcess();
				}
				else {        // receive data
					if (ret < BUF_SIZE)
						memset(&buf[ret], '\0', 1);
					//connectionRemainTime[i] = maxIdleConnectionSeconds;
					printf("client[%d] send:%s\n", i, buf);
					const char * expectedMsg = "echo request";
					if (0 == strcmp(buf, expectedMsg)) {
						const char * sendData = "echo response";
						send(fd_A[i], sendData, strlen(sendData), 0);
					}
					
				}
			}
			else {
				connectionRemainTime[i] -= (int)deltaSeconds.count();
				if (connectionRemainTime[i] <= 0) {
					closesocket(fd_A[i]);
					FD_CLR(fd_A[i], &fdsr);
					fd_A[i] = 0;
					//重置连接剩余时间
					connectionRemainTime[i] = maxIdleConnectionSeconds;
					conn_amount--;
					//待增加启动对应监控程序代码
					test_createProcess();
				}
			}
		}

		// check whether a new connection comes
		if (FD_ISSET(sock_fd, &fdsr)) {
			new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &sin_size);
			if (new_fd <= 0) {
				perror("accept");
				continue;
			}

			// add to fd queue
			if (conn_amount < BACKLOG) {
				for (i = 0; i < BACKLOG; i++)
				{
					if (fd_A[i] == 0)
					{
						fd_A[i] = new_fd;
						break;
					}

				}
				conn_amount++;
				printf("new connection client[%d] %s:%d\n", conn_amount,
					inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

				if (new_fd > maxsock)
					maxsock = new_fd;
			}
			else {
				printf("max connections arrive, exit\n");
				send(new_fd, "bye", 4, 0);
				closesocket(new_fd);
				break;
			}
		}
		serverMaxRemainIdleTime = maxServerIdleConsecutiveSeconds;
	}

	closesocket(sock_fd);

	// close other connections
	for (i = 0; i < BACKLOG; i++) {
		if (fd_A[i] != 0) {
			closesocket(fd_A[i]);
		}
	}

	WSACleanup();

}
void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
int netServerSelect2(){
	const int BUF_SIZE = 1024;
	WSADATA wsadata;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAddr, clntAddr;
	TIMEVAL timeout;
	fd_set reads, cp_reads;

	int adr_sz;
	int str_len, i, fd_num;
	char buf[BUF_SIZE];

	
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
		error_handling("WSAStartup error");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(1234);

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		error_handling("bind error");
	if (listen(hServSock, 5) == SOCKET_ERROR)
		error_handling("listen error");

	FD_ZERO(&reads);
	FD_SET(hServSock, &reads);

	while (1)
	{
		cp_reads = reads;
		timeout.tv_sec = 5;

		if ((fd_num = select(0, &cp_reads, 0, 0, &timeout)) == SOCKET_ERROR)
			break;
		if (fd_num == 0) {
			printf("timeout\n");
			continue;
		}

		for (i = 0; i < reads.fd_count; i++)
		{
			if (FD_ISSET(reads.fd_array[i], &cp_reads))
			{
				if (reads.fd_array[i] == hServSock)
				{
					adr_sz = sizeof(clntAddr);
					hClntSock = accept(hServSock, (SOCKADDR*)&servAddr, &adr_sz);
					FD_SET(hClntSock, &reads);
					printf("connected client:%d\n", hClntSock);
				}
				else
				{
					str_len = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
					if (str_len == 0)
					{
						FD_CLR(reads.fd_array[i], &reads);
						closesocket(cp_reads.fd_array[i]);
						printf("closed client:%d\n", cp_reads.fd_array[i]);
					}
					else
						send(reads.fd_array[i], buf, str_len, 0);
				}
			}
		}
	}
	closesocket(hServSock);
	WSACleanup();
	return 0;
}