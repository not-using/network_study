#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <ws2tcpip.h>
#include <thread>
#include "server.hpp"

#define BUFSIZE 100

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
	retval = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR_IN));
	if (retval == SOCKET_ERROR) err_quit(L"bind()");

	retval = listen(listenSocket, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit(L"listen()");
	SOCKET			clientSocket;
	SOCKADDR_IN		clientAddr;
	int				addrLen = sizeof(SOCKADDR_IN);

	char			recvBuf[BUFSIZE];
	char			addrBuf[32];
	char			sendBuf[9];
	int32_t			calc;
	int				recvSize;

	while (true)
	{
		std::cout << "Server is listening" << std::endl;
		clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET) {
			err_display(L"accept()");
			break;
		}
		inet_ntop(AF_INET, &clientAddr, addrBuf, 32);
		std::cout << "[connected] " << addrBuf << ":" << ntohs(clientAddr.sin_port) << std::endl;

		ZeroMemory(recvBuf, BUFSIZE);
		while (true) {
			int recvSize = recv(clientSocket, recvBuf, BUFSIZE, 0);
			if (recvSize == SOCKET_ERROR) {
				err_display(L"recv()");
				break;
			}
			else if (recvSize == 0)
				break;
			std::cout << recvSize << "bytes received" << std::endl;
			if (recvSize < 5) {
				err_display(L"invalid data");
				break;
			}
			int		readPos = 0;
			while (recvSize)
			{
				PHeader* pack = (PHeader*)&recvBuf[readPos];
				if (pack->DataSize > recvSize) {
					memcpy(recvBuf, recvBuf + readPos, BUFSIZE - readPos);
					int newRecv = recv(clientSocket, recvBuf + BUFSIZE - readPos, readPos, 0);
					std::cout << newRecv << "bytes received" << std::endl;
					recvSize = BUFSIZE - readPos + newRecv;
					pack = (PHeader*)&recvBuf;
					if (pack->DataSize > recvSize) {
						err_display(L"invalid data");
						break;
					}
					readPos = 0;

				};
				switch (pack->PacketID)
				{
				case 21:
					binaryCalculus(recvBuf + readPos, &calc);
					break;
				case 22:
					ternaryCalculus(recvBuf + readPos, &calc);
					break;
				default:
					calc = 0;
				}

				ZeroMemory(sendBuf, 9);
				Response* resPack = (Response*)sendBuf;

				resPack->DataSize = 9;
				resPack->PacketID = 31;
				memcpy(&resPack->Data, &calc, sizeof(int));
				retval = send(clientSocket, sendBuf, resPack->DataSize, 0);
				std::cout << retval << "bytes sent" << std::endl;

				readPos += pack->DataSize;
				recvSize -= pack->DataSize;
			}

			//delay
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		}
		std::cout << "[deconnected]" << std::endl;
		closesocket(clientSocket);
	}

	closesocket(listenSocket);

	WSACleanup();
	return 0;
}