#include "server.h"

#ifdef _WIN32
#pragma push_macro("DELETE")
#pragma push_macro("ERROR")

#undef DELETE
#undef ERROR
#endif

void HttpServer::prepServer() {
    getSock();
    bindSock();
    startListen();
}
bool HttpServer::defLog() {
    return (&(this->logs.getStream()) == &std::cout);
}
void HttpServer::lerror(const char* message) {
    if (this->defLog()) {
        perror(message);
    }
    else {
        ((((this->logs <<= dateStamp()) <= " : Error - ") <= message) < newline);
    }
}
void HttpServer::ex_lerror(const char* message) {
    if (this->defLog()) {
        perror(message);
    }
    else {
        ((((this->logs <<= dateStamp()) <= " : Error - ") <= message) < newline);
    }
    exit(errno);
}

std::string HttpServer::HttpHandler::find(std::string&& root, const std::string& item) {
    std::string path = std::move(root);
    if (item == "/") {
        path.append("/index.html");
    }
    //check for special specifier paths here
    else {
        path.append(item);
    }
    return path;
}
const char* HttpServer::HttpHandler::safeMime(const char* path) {
    const char* mime = getMegaMimeType(path);
    if (mime) {
        return mime;
    }
    return "text/plain";
}

void HttpServer::HttpHandler::respond(const int socket, const char* ip, const int readlen, const std::string& input) {
    Request req(input); //CHECK FOR VALID HTTP
    HeaderList headers;
    std::ifstream reader;
    std::string body, path = this->rfind(that->root, req.getResource());

    this->that->formatter.onRequest(socket, ip, readlen, &req, path.c_str());   //change Request so that we can pass a const in here

    (this->response.intHeaders())->reset();

    headers.add(   //headers that apply to all
        Segment("Server", "Custom C++ HTTP Server (Raspberry Pi)")
    );

    switch (req.getMethod()) {
    case Method::GET:   //requesting resource
    {
        reader.open(path.c_str(), std::ios::binary);    //attempt to open resource
        if (reader.is_open()) { //if resource exists
            body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
            headers.add({
                {"Content-Type", safeMime(path.c_str())},
                {"Content-Length", std::to_string(body.length())}
                });
            if (that->version == Version::HTTP_1_0) {   //close connection if http 1.0
                headers.add("Connection", "close");
            }
            else {
                headers.add("Connection", "keep-alive");
            }
            this->response.update(Code::OK, headers, body);
        }
        else {  //send 404
            reader.open(this->rfind(that->root, "/error.html"));
            if (reader.is_open()) { //check if error page exists
                body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
                replace(body, "{{code}}", Codes::getString(Code::NOT_FOUND).c_str());
                headers.add(
                    Segment("Content-Type", "text/html")
                );
            }
            else {  //if not send plain text
                body = "{Error page not found} - Error: 404 Not Found";
                headers.add(
                    Segment("Content - Type", "text / plain")
                );
            }
            headers.add({
                {"Content-Length", std::to_string(body.length())},
                {"Connection", "close"},
                });
            this->response.update(Code::NOT_FOUND, headers, body);

        }
        break;
    }
    break;
    case Method::HEAD:  //requesting header only
    {
        reader.open(path, std::ios::binary);    //attempt to open resource
        if (reader.is_open()) { //if exists
            body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
            headers.add({
                {"Content-Type", safeMime(path.c_str())},
                {"Content-Length", std::to_string(body.length())}
                });
            this->response.update(Code::OK, headers);
        }
        else {  //else send 404
            reader.open(this->rfind(that->root, "/error.html"));
            if (reader.is_open()) { //check error page exists
                body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
                replace(body, "{{code}}", Codes::getString(Code::NOT_FOUND).c_str());
                headers.add(
                    Segment("Content-Type", "text/html")
                );
            }
            else {  //if not send text
                body = "{Error page not found} - Error: 404 Not Found";
                headers.add(
                    Segment("Content-Type", "text/plain")
                );
            }
            headers.add({
                {"Content-Length", std::to_string(body.length())},
                {"Connection", "close"},
                });
            this->response.update(Code::NOT_FOUND, headers);
        }
        break;
    }
    break;
    case Method::CONNECT:
    case Method::DELETE:
    case Method::OPTIONS:
    case Method::PATCH:
    case Method::POST:
    case Method::PUT:
    case Method::TRACE: //methods not supported
    {
        reader.open(this->rfind(that->root, "/error.html"));
        if (reader.is_open()) { //check if page exists
            body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
            replace(body, "{{code}}", Codes::getString(Code::METHOD_NOT_ALLOWED).c_str());
            headers.add(
                Segment("Content-Type", "text/html")
            );
        }
        else {  //else send plain text
            body = "{Error page not found} - Error: 405 Method Not Allowed";
            headers.add(
                Segment("Content-Type", "text/plain")
            );
        }
        headers.add({
            {"Allow", "GET, HEAD"}, //only GET and HEAD are suppored
            {"Content-Length", std::to_string(body.length())},
            {"Connection", "close"},
            });
        this->response.update(Code::METHOD_NOT_ALLOWED, headers, body);
        break;
    }
    case Method::ERROR: //method is the first thing that is parsed, so an error means that the request was invalid
    {
        reader.open(this->rfind(that->root, "/error.html"));
        if (reader.is_open()) { //check if page exists
            body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
            replace(body, "{{code}}", Codes::getString(Code::BAD_REQUEST).c_str());
            headers.add(
                Segment("Content-Type", "text/html")
            );
        }
        else {  //else send plain text
            body = "{Error page not found} - Error: 400 Bad Request";
            headers.add(
                Segment("Content-Type", "text/plain")
            );
        }
        headers.add({
            {"Content-Length", std::to_string(body.length())},
            {"Connection", "close"},
            });
        this->response.update(Code::BAD_REQUEST, headers, body);
        break;
    }
    default:    //defaults to 501 NOT IMPLEMENTED
    {
        reader.open(this->rfind(that->root, "/error.html"));
        if (reader.is_open()) { //check if page exists
            body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
            replace(body, "{{code}}", Codes::getString(Code::NOT_IMPLEMENTED).c_str());
            headers.add(
                Segment("Content-Type", "text/html")
            );
        }
        else {  //else send plain text
            body = "{Error page not found} - Error: 501 Not Implemented";
        }
        headers.add({
            {"Content-Length", std::to_string(body.length())},
            {"Connection", "close"},
            });
        this->response.update(Code::NOT_IMPLEMENTED, headers, body);
        break;
    }
    }

    std::string ret = this->response.getSerialized();
    this->that->formatter.onResponse(socket, ip, send(socket, ret.c_str(), ret.length(), 0), &(this->response), path.c_str());
    return;
}

olstream* HttpServer::Formatter::getStream() {
    return &(this->that->logs);
}
Version* HttpServer::Formatter::getVer() {
    return &(this->that->version);
}

void HttpServer::Formatter::update(HttpServer* outer) {
    this->that = outer;
}

void HttpServer::Formatter::onServerStart() {
    ((((this->that->logs <<= dateStamp()) <= " : Webserver resources initialized - beginning to serve [") <= Versions::getString(this->that->version)) < "]\n\n");
}
void HttpServer::Formatter::onConnect(int fd, const char* ip) {
    ((((this->that->logs <<= dateStamp()) <= " : Got connection from [") <= ip) < "]\n");
}
void HttpServer::Formatter::onRequest(int fd, const char* ip, int readlen, const Request* req, const char* resource) {
    ((((((this->that->logs <<= dateStamp()) <= " : Got request from [") <= ip) <= "] - length: ") <= readlen) < newline);
}
void HttpServer::Formatter::onResponse(int fd, const char* ip, int sent, const Response* resp, const char* resource) {
    ((((((((this->that->logs <<= dateStamp()) <= " : Sent {") <= resource) <= "} to [") <= ip) <= "] - length: ") <= sent) < newline);
}
void HttpServer::Formatter::onDisconnect(int fd, const char* ip) {
    ((((this->that->logs <<= dateStamp()) <= " : Ended connection with [") <= ip) < "]\n\n");
}
void HttpServer::Formatter::onServerEnd() {
    this->that->logs << withTime("Webserver stopped");
}

HttpServer::HttpServer(
    const olstream& logger,
    const char* root,
    std::string(*rmapper)(std::string&&, const std::string&),
    std::atomic<bool>* control,
    Version version,
    int max_clients
) : BaseServer(NULL, "http", max_clients), root(root), version(version), handler(this, rmapper), formatter(this) {
    if (control != nullptr) {
        this->online = control;
    }
    else {
        this->online = &(this->l_online);
        this->l_online = false;
    }
    this->logs = logger;
    this->logs.setModes(std::ios::app);
    getServerAddr();
}
HttpServer::HttpServer(
    olstream&& logger,
    const char* root,
    std::string(*rmapper)(std::string&&, const std::string&),
    std::atomic<bool>* control,
    Version version,
    int max_clients
) : BaseServer(NULL, "http", max_clients), root(root), version(version), handler(this, rmapper), formatter(this) {
    if (control != nullptr) {
        this->online = control;
    }
    else {
        this->online = &(this->l_online);
        this->l_online = false;
    }
    this->logs = std::move(logger);
    this->logs.setModes(std::ios::app);
    getServerAddr();
}

void HttpServer::setLog(const std::ios::openmode modes) {
    this->logs.setModes(modes);
}
void HttpServer::setLog(const char* file) {
    this->logs.setStream(file);
}
void HttpServer::setLog(const char* file, const std::ios::openmode modes) {
    this->logs.setStream(file, modes);
}
void HttpServer::setLog(std::ostream* stream) {
    this->logs.setStream(stream);
}
void HttpServer::setLog(const olstream& target) {
    this->logs = target;
}
void HttpServer::setLog(olstream&& target) {
    this->logs = std::move(target);
}

void HttpServer::stop() {   //will not change external vars, but still alerts running serve functions to stop
    this->l_online = false;
    this->online = &l_online;
}
void HttpServer::estop() {
    CLOSE_SOCK(this->sockmain);
    this->logs << withTime("Server socket closed out of loop (emergency stop)\n");
}

void HttpServer::serve() {  //start server in another thread?
    switch (this->version) {
    case Version::HTTP_1_0:
        this->s_serve1_0();
        break;
    case Version::HTTP_1_1:
        this->s_serve1_1();
        break;
    case Version::HTTP_2_0: //fall back to 1.1 but also print warning
        this->s_serve1_1();
        this->logs << "HTTP version (2.0) not supported - utilized fallback to HTTP/1.1\n";
        break;
    default:
        this->logs << "HTTP version not supported - failed to start server\n";
    }
}

void HttpServer::s_serve1_0() {
    this->version = Version::HTTP_1_0;  //since this is a public function, the HTTP version could be different that that which was initialized with
    prepServer();
    this->formatter.onServerStart();

    SOCK_T nsock; 
    int readlen;
    char buffer[10000];
    char ipbuff[INET6_ADDRSTRLEN];
    sockaddr_storage naddr;
    socklen_t naddrlen = sizeof(naddr);
    timeval tbuff, checkup = { 1, 0 };
    fd_set master, fdbuff;
    FD_ZERO(&master);
    FD_ZERO(&fdbuff);
    FD_SET(this->sockmain, &master);

    this->l_online = true;   //server is (will be) online

    while (*(this->online)) {
        tbuff = checkup;
        fdbuff = master;
        if (select((this->sockmain + 1), &fdbuff, NULL, NULL, &tbuff) == -1) {    //wait for activity and perform checkup every timeval
            this->lerror("Select error");
            continue;
            //filter error and exit if needed here
        }
        if ((nsock = accept(this->sockmain, (sockaddr*)&naddr, &naddrlen)) < 0) {
            this->lerror("Error accepting connection");
            continue;
        }
        getSockIp(nsock, ipbuff);
        this->formatter.onConnect(nsock, ipbuff);
        if ((readlen = recv(this->sockmain, buffer, sizeof(buffer), 0)) < 0) {
            this->lerror("Error recieving data");
        }
        else {
            this->handler.respond(nsock, ipbuff, readlen, std::string(buffer));
            this->formatter.onDisconnect(nsock, ipbuff);
            memset(&buffer, 0, sizeof(buffer));
        }
        CLOSE_SOCK(nsock);
    }
    CLOSE_SOCK(this->sockmain);
    this->formatter.onServerEnd();

}
void HttpServer::s_serve1_1() {
    this->version = Version::HTTP_1_1;
    prepServer();
    this->formatter.onServerStart();

    SOCK_T nsock;
    int readlen, rbuff, maxfd = this->sockmain;
    char buffer[10000], ipbuff[INET6_ADDRSTRLEN];
    sockaddr_storage naddr;
    socklen_t naddrlen = sizeof(naddr);
    timeval tbuff, checkup = { 1, 0 };
    fd_set master, fdbuff;
    FD_ZERO(&master);
    FD_ZERO(&fdbuff);
    FD_SET(this->sockmain, &master);

    this->l_online = true;   //server is (will be) online

    while (*(this->online)) {
        tbuff = checkup;
        fdbuff = master;
        if ((rbuff = select((maxfd + 1), &fdbuff, NULL, NULL, &tbuff)) <= 0) {  //wait up to 1 second for activiy, else continue (check for stop message)
            if (rbuff == -1) {
                this->lerror("Select error");
            }
            continue;
        }
        for (int i = 0; i <= maxfd; i++) {
            if (FD_ISSET(i, &fdbuff)) {
                if (i == this->sockmain) {  //if our fd is ready
                    if ((nsock = accept(i, (sockaddr*)&naddr, &naddrlen)) < 0) {    //accept new connection
                        this->lerror("Error accepting connection");
                        continue;
                    }
                    FD_SET(nsock, &master); //add connection fd to master
                    if (nsock > maxfd) {    //update the highest fd
                        maxfd = nsock;
                    }
                    getSockIp(nsock, ipbuff);
                    this->formatter.onConnect(nsock, ipbuff);
                }
                else {  //else a client fd is ready
                    if ((readlen = recv(i, buffer, sizeof(buffer), 0)) <= 0) {  //read from client fd
                        if (readlen == 0) {
                            getSockIp(i, ipbuff);
                            this->formatter.onDisconnect(i, ipbuff);
                        }
                        else {
                            this->lerror("Error recieving data");
                        }
                        CLOSE_SOCK(i);
                        FD_CLR(i, &master);
                        if (i >= maxfd) {
                            maxfd -= 1;
                        }
                    }
                    else {  //respond
                        getSockIp(i, ipbuff);
                        this->handler.respond(i, ipbuff, readlen, std::string(buffer));
                    }
                    memset(&buffer, 0, sizeof(buffer));
                }
            }
        }
    }
    CLOSE_SOCK(this->sockmain);
    this->formatter.onServerEnd();
}
void HttpServer::serve_test() {
    this->logs << "Override this function to run a test HTTP server\n";
}

#ifdef _WIN32
#pragma pop_macro("ERROR")
#pragma pop_macro("DELETE")
#endif