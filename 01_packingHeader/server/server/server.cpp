#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <ws2tcpip.h>
#include "server.hpp"

#define BUFSIZE 512

void	readHeader(char* buf, PHeader* head)
{
	memcpy(head, buf, 5);

//	head->DataSize = *(short*)buf;
//	head->PacketID = *(short*)(buf + 2);
//	head->Type = *(buf + 4);

}

int main() 
{
	int			retval;

	WSADATA		wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET) 
		err_quit(L"socket()");
	
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(ADDR_ANY);
	serverAddr.sin_port = htons(11021);
	retval = bind(listenSocket, (SOCKADDR *)&serverAddr, sizeof(SOCKADDR_IN));
	if (retval == SOCKET_ERROR) err_quit(L"bind()");

	retval = listen(listenSocket, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit(L"listen()");
	SOCKET			clientSocket;
	SOCKADDR_IN		clientAddr;
	int				addrLen = sizeof(SOCKADDR_IN);

	PHeader			pack;
	Response		resPack;
	char			buf[BUFSIZE];
	int32_t				calc;

	while (true)
	{
		std::cout << "Server is listening" << std::endl;
		clientSocket = accept(listenSocket, (SOCKADDR *) &clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET) err_display(L"accept()");
		inet_ntop(AF_INET, &clientAddr, buf, BUFSIZE);
		std::cout << "[connected] " << buf << ":" << ntohs(clientAddr.sin_port) << std::endl;
		
		ZeroMemory(buf, BUFSIZE);
		while (true) {
			retval = recv(clientSocket, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR || retval == 0)
				break;

			std::cout << retval << "bytes received" << std::endl;
			if (retval < 10) {
				err_display(L"invalid data");
				break;
			}

			readHeader(buf, &pack);
			switch (pack.PacketID)
			{
			case 21:
				binaryCalculus(buf, &calc);
				break;
			case 22:
				ternaryCalculus(buf, &calc);
				break;
			default:
				calc = 0;
			}

			ZeroMemory(&resPack, sizeof(resPack));
			resPack.DataSize = sizeof(resPack);
			resPack.PacketID = 31;
			memcpy(&resPack.Data, &calc, sizeof(int));
			retval = send(clientSocket, (char*)&resPack, resPack.DataSize, 0);
			std::cout << retval << "bytes sent" << std::endl;

			
		}
		std::cout << "[deconnected]" << std::endl;
		closesocket(clientSocket);
	}

	closesocket(listenSocket);

	WSACleanup();
	return 0;
}