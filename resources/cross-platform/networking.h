#pragma once

#include <iostream>
#include <stdio.h>
#include <vector>
#include <string.h>

#ifdef __unix__
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#define CLOSE_SOCK(sock) (close(sock))
#endif
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define CLOSE_SOCK(sock) (closesocket(sock))
#endif

#ifdef _WIN32 
typedef SOCKET SOCK_T;
#define __SOCK_ERROR == INVALID_SOCKET
#else
typedef int SOCK_T;
#define __SOCK_ERROR < 0
#endif

class BaseServer {
private:
	const char* ip, * port;
	bool reuse;
	int family, socktype, flags, connections;
protected:
	//friend class http::HttpServer;
	addrinfo* list;
	SOCK_T sockmain;
public:
#ifdef _WIN32
	static int initDll();
	static void exitDll();
#endif

	BaseServer(
		const char* ip, const char* port, int connections = 5, bool reusable = true,
		int family = AF_INET, int socktype = SOCK_STREAM, int flags = AI_PASSIVE
	);
	~BaseServer();

	void getServerAddr();
	void getSock();
	void bindSock();
	void startListen();

	//general
	static void getAddrs(const char* ip, const char* port, addrinfo* hints, addrinfo** list);
	static void getAddrs(addrinfo** list, const char* ip, const char* port, int family = AF_UNSPEC, int socktype = SOCK_STREAM, int flags = AI_PASSIVE);
	static void getAddr4(addrinfo** list, const char* ip, const char* port, int socktype = SOCK_STREAM, int flags = AI_PASSIVE);
	static void getAddr6(addrinfo** list, const char* ip, const char* port, int socktype = SOCK_STREAM, int flags = AI_PASSIVE);
	static void getServerAddr(addrinfo** list, const char* port, int family = AF_INET, int socktype = SOCK_STREAM);
	static void getServerAddr(addrinfo** list);

	static void addrListVec(addrinfo* list, std::vector<addrinfo>& vec);

	static SOCK_T getSock(addrinfo* addr, bool reuse = true);
	static SOCK_T getSock(const addrinfo& addr, bool reuse = true);
	static SOCK_T bindNewSock(addrinfo* addr, bool reuse = true);
	static void bindSock(SOCK_T sock, addrinfo* addr);

	static void startListen(SOCK_T sock, int connections);
};

//network utilities
void getSockIp(SOCK_T socket, char* ip);