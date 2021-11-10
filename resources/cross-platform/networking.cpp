#include "networking.h"

#ifdef _WIN32
void BaseServer::exitDll() {
    WSACleanup();
}
int BaseServer::initDll() {
    WSADATA wsa_data;
    atexit(exitDll);
    return WSAStartup(MAKEWORD(2, 2), &wsa_data);
}
#endif

BaseServer::BaseServer(const char* ip, const char* port, int connections, bool reusable, int family, int socktype, int flags)
    : ip(ip), port(port), reuse(reusable), family(family), socktype(socktype), flags(flags), connections(connections)
{
#ifdef _WIN32
    initDll();
#endif
}
BaseServer::~BaseServer() {
    freeaddrinfo(list);
}

void BaseServer::getServerAddr() {
    int status;
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = socktype;
    hints.ai_flags = flags;

    if ((status = getaddrinfo(ip, port, &hints, &this->list)) != 0) {
        fprintf(stderr, "Error getting address: %ws\n", gai_strerror(status));
        //this->~BaseServer();
    }
}
void BaseServer::getSock() {
    if ((sockmain = socket(list->ai_family, list->ai_socktype, list->ai_protocol))__SOCK_ERROR) {
        perror("Error creating socket");
        //this->~BaseServer();
    }
    if (reuse) {
        int optval = 1; //whatever we want to set the sockopt to, in this case, "true"
#ifdef _WIN32
        if (setsockopt(sockmain, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*) &optval, sizeof(optval)) < 0) {
#else
        if (setsockopt(sockmain, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) < 0) {
#endif
            perror("Error setting socket reusable");
            //this->~BaseServer();
        }
    }
}
void BaseServer::bindSock() {
    if (bind(sockmain, list->ai_addr, list->ai_addrlen) < 0) {
        perror("Error binding socket");
        //this->~BaseServer();
    }
}
void BaseServer::startListen() {
    if (listen(sockmain, connections) < 0) {
        perror("Error initing listen");
        //this->~BaseServer();
    }
}



//static members

void BaseServer::getAddrs(const char* ip, const char* port, addrinfo* hints, addrinfo** list) {
    int addr;
    if ((addr = getaddrinfo(ip, port, hints, list)) != 0) {
        fprintf(stderr, "Error getting address: %ws\n", gai_strerror(addr));
    }
}
void BaseServer::getAddrs(addrinfo** list, const char* ip, const char* port, int family, int socktype, int flags) {
    int addr;
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = socktype;
    hints.ai_flags = flags;

    if ((addr = getaddrinfo(ip, port, &hints, list)) != 0) {
        fprintf(stderr, "Error getting address: %ws\n", gai_strerror(addr));
    }
}
void BaseServer::getAddr4(addrinfo** list, const char* ip, const char* port, int socktype, int flags) {
    int addr;
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = socktype;
    hints.ai_flags = flags;

    if ((addr = getaddrinfo(ip, port, &hints, list)) != 0) {
        fprintf(stderr, "Error getting address: %ws\n", gai_strerror(addr));
    }
}
void BaseServer::getAddr6(addrinfo** list, const char* ip, const char* port, int socktype, int flags) {
    int addr;
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = socktype;
    hints.ai_flags = flags;

    if ((addr = getaddrinfo(ip, port, &hints, list)) != 0) {
        fprintf(stderr, "Error getting address: %ws\n", gai_strerror(addr));
    }
}
void BaseServer::getServerAddr(addrinfo** list, const char* port, int family, int socktype) {
    int addr;
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = socktype;
    hints.ai_flags = AI_PASSIVE;

    if ((addr = getaddrinfo(NULL, "80", &hints, list)) != 0) {
        fprintf(stderr, "Error getting address: %ws\n", gai_strerror(addr));
    }
}
void BaseServer::getServerAddr(addrinfo** list) {
    int addr;
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((addr = getaddrinfo(NULL, "80", &hints, list)) != 0) {
        fprintf(stderr, "Error getting address: %ws\n", gai_strerror(addr));
    }
}

void BaseServer::addrListVec(addrinfo* list, std::vector<addrinfo>& vec) {
    for (addrinfo* a = list; a != NULL; a = a->ai_next) {
        vec.push_back(*a);
    }
    freeaddrinfo(list);
}

SOCK_T BaseServer::getSock(addrinfo* addr, bool reuse) {
    SOCK_T sock;
    if ((sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol))__SOCK_ERROR) {
        perror("Error creating socket");
    }
    if (reuse) {
        int optval = 1;
#ifdef _WIN32
        if (setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&optval, sizeof(optval)) < 0) {
#else
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) < 0) {
#endif
            perror("Error setting socket reusable");
        }
    }
    return sock;
}
SOCK_T BaseServer::getSock(const addrinfo& addr, bool reuse) {
    SOCK_T sock;
    if ((sock = socket(addr.ai_family, addr.ai_socktype, addr.ai_protocol))__SOCK_ERROR) {
        perror("Error creating socket");
    }
    if (reuse) {
        int optval = 1;
#ifdef _WIN32
        if (setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&optval, sizeof(optval)) < 0) {
#else
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) < 0) {
#endif
            perror("Error setting socket reusable");
        }
    }
    return sock;
}
SOCK_T BaseServer::bindNewSock(addrinfo* addr, bool reuse) {
    SOCK_T sock;
    if ((sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol))__SOCK_ERROR) {
        perror("Error creating socket");
    }
    if (reuse) {
        int optval = 1;
#ifdef _WIN32
        if (setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&optval, sizeof(optval)) < 0) {
#else
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) < 0) {
#endif
            perror("Error setting socket reusable");
        }
    }
    if (bind(sock, addr->ai_addr, addr->ai_addrlen) < 0) {  //supress C4267
        perror("Error binding socket");
    }
    return sock;
}
void BaseServer::bindSock(SOCK_T sock, addrinfo* addr) {
    if (bind(sock, addr->ai_addr, addr->ai_addrlen) < 0) {  //supress C4267
        perror("Error binding socket");
    }
}

void BaseServer::startListen(SOCK_T sock, int connections) {
    if (listen(sock, connections) < 0) {
        perror("Error initing listen");
    }
}



//general networking

void getSockIp(SOCK_T socket, char* ip) {
    sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);
    getpeername(socket, (sockaddr*)&addr, &addrlen);
    if (addr.ss_family == AF_INET) {
        inet_ntop(AF_INET, &(((sockaddr_in*)&addr)->sin_addr), ip, INET_ADDRSTRLEN);
    }
    else {
        inet_ntop(AF_INET6, &(((sockaddr_in6*)&addr)->sin6_addr), ip, INET6_ADDRSTRLEN);
    }
}