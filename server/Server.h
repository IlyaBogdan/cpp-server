#pragma once
#include <iostream>
#include <sstream>
#include <string>

#define _WIN32_WINNT 0x501

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using std::cerr;

using namespace std;

class Server {
	
public:

	void run() {
		cout << "Server is running on " << this->port << " port" << "\n";
		while (true) {
			this->runSocket();
		}
	}

	int stop() {
		return 0;
	}

	Server(PCSTR port) {
		this->port = port;
	}

private:

	PCSTR port;
	WSADATA wsaData;
	struct addrinfo* addr = NULL;
	int counter = 1;
	int listen_socket;
	int client_socket;

	int runSocket() {
		
		int result = WSAStartup(MAKEWORD(2, 2), &this->wsaData);

		if (result != 0) {
			cerr << "WSAStartup failed: " << result << "\n";
			return result;
		}
		struct addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		result = getaddrinfo("127.0.0.1", this->port, &hints, &this->addr);

		if (result != 0) {
			cerr << "getaddrinfo failed: " << result << "\n";
			WSACleanup();
			return 1;
		}
		this->listen_socket = socket(this->addr->ai_family, this->addr->ai_socktype, this->addr->ai_protocol);
		if (this->listen_socket == INVALID_SOCKET) {
			cerr << "Error at socket: " << WSAGetLastError() << "\n";
			freeaddrinfo(this->addr);
			WSACleanup();
			return 1;
		}
		result = bind(this->listen_socket, this->addr->ai_addr, (int)this->addr->ai_addrlen);
		if (result == SOCKET_ERROR) {
			cerr << "bind failed with error: " << WSAGetLastError() << "\n";
			freeaddrinfo(this->addr);
			closesocket(this->listen_socket);
			WSACleanup();
			return 1;
		}
		if (listen(this->listen_socket, SOMAXCONN) == SOCKET_ERROR) {
			cerr << "listen failed with error: " << WSAGetLastError() << "\n";
			closesocket(this->listen_socket);
			WSACleanup();
			return 1;
		}
		this->client_socket = accept(listen_socket, NULL, NULL);
		if (this->client_socket == INVALID_SOCKET) {
			cerr << "accept failed: " << WSAGetLastError() << "\n";
			closesocket(listen_socket);
			WSACleanup();
			return 1;
		} else {
			this->response();
		}
	}
	int response() {
		int result = WSAStartup(MAKEWORD(2, 2), &this->wsaData);

		const int max_client_buffer_size = 1024;
		char buf[max_client_buffer_size];

		result = recv(this->client_socket, buf, max_client_buffer_size, 0);

		std::stringstream response;
		std::stringstream response_body;

		if (result == SOCKET_ERROR) {
			cerr << "recv failed: " << result << "\n";
			closesocket(this->client_socket);
		}
		else if (result == 0) {
			cerr << "connection closed...\n";
		}
		else if (result > 0) {

			std::cout << result;

			buf[result] = '\0';

			char buf_test[100];
			snprintf(buf_test, sizeof(buf_test), "<em><small>Test C++ Http Server %d</small></em>\n", this->counter);
			response_body << "<title>Test C++ HTTP Server</title>\n"
				<< "<h1>Test page count: </h1>\n"
				<< "<p>This is body of the test page...</p>\n"
				<< "<h2>Request headers</h2>\n"
				<< "<pre>" << buf << "</pre>\n"
				<< buf_test;

			response << "HTTP/1.1 200 OK\r\n"
				<< "Version: HTTP/1.1\r\n"
				<< "Content-Type: text/html; charset=utf-8\r\n"
				<< "Content-Length: " << response_body.str().length()
				<< "\r\n\r\n"
				<< response_body.str();

			result = send(this->client_socket, response.str().c_str(),
				response.str().length(), 0);

			if (result == SOCKET_ERROR) {
				cerr << "send failed: " << WSAGetLastError() << "\n";
			}
			closesocket(this->client_socket);
		}

		closesocket(this->listen_socket);
		freeaddrinfo(this->addr);
		WSACleanup();
		this->counter++;
		return 0;
	}
};

