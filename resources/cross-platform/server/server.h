#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <atomic>

#include "http.h"
#include "mimetype.h"
#include "../networking.h"
#include "../output.h"
#include "../program.h"
#include "../resources.h"

#ifdef _WIN32
#pragma push_macro("DELETE")
#pragma push_macro("ERROR")

#undef DELETE
#undef ERROR
#endif

class HttpServer : public BaseServer {
protected:
	std::atomic<bool> l_online;	//backup for if external control not specified

	const char* root;
	Version version;
	std::atomic<bool>* online;
	olstream logs;

	void prepServer();
	inline bool defLog();
	void lerror(const char* message);
	void ex_lerror(const char* message);

	class HttpHandler {
	private:
		HttpServer* that;	//surrounding instance
	protected:
		std::string(*rfind)(std::string&&, const std::string&);
		Response response;
		//Request request;
	public:
		static std::string find(std::string&& root, const std::string& item);
		static const char* safeMime(const char* path);

		HttpHandler(HttpServer* outer, std::string(*finder)(std::string&&, const std::string&) = find) :
			that(outer), rfind(finder), response(that->version)/*, request(that->version)*/ {}

		virtual void respond(const int socket, const char* ip, const int readlen, const std::string& input);	//server
		//void request(const std::string& input, std::ostream& output, bool init = false);	//client
	};

public:

	class Formatter {
		template<class format_t> friend class HttpServer2;
	protected:
		HttpServer* that;
		olstream* getStream();
		Version* getVer();

		Formatter() : that(nullptr) {}
		virtual void update(HttpServer* outer);
	public:	//fix visiblity issues with default constructor/add guards for nullptr (that)
		Formatter(HttpServer* outer) : that(outer) {}

		virtual void onServerStart();
		virtual void onConnect(int fd, const char* ip);
		virtual void onRequest(int fd, const char* ip, int readlen, const Request* req = nullptr, const char* resource = nullptr);
		virtual void onResponse(int fd, const char* ip, int sent, const Response* resp = nullptr, const char* resource = nullptr);
		virtual void onDisconnect(int fd, const char* ip);
		virtual void onServerEnd();
	};

	HttpServer(
		const olstream& logger,
		const char* root = progdir.getDir(),
		std::string(*rmapper)(std::string&&, const std::string&) = HttpHandler::find,
		std::atomic<bool>* control = nullptr,
		Version version = Version::HTTP_1_1,
		int max_clients = 5
	);
	HttpServer(
		olstream&& logger = &std::cout,
		const char* root = progdir.getDir(),
		std::string(*rmapper)(std::string&&, const std::string&) = HttpHandler::find,
		std::atomic<bool>* control = nullptr,
		Version version = Version::HTTP_1_1,
		int max_clients = 5
	);

	void setLog(const std::ios::openmode modes);
	void setLog(const char* file);
	void setLog(const char* file, const std::ios::openmode modes);
	void setLog(std::ostream* stream);
	void setLog(const olstream& target);
	void setLog(olstream&& target);

	void stop();
	void estop();

	virtual void serve();
	virtual void s_serve1_0();
	virtual void s_serve1_1();
	virtual void serve_test();
private:
	HttpHandler handler;
	Formatter formatter;
};

//*****************************************************************************************

template<class format_t = HttpServer::Formatter>
class HttpServer2 : public HttpServer {
protected:

	class HttpHandler2 : public HttpServer::HttpHandler {
	protected:
		HttpServer2<format_t>* d_that;	//surrounding derived instance
	public:
		HttpHandler2(HttpServer2<format_t>* outer, std::string(*finder)(std::string&&, const std::string&) = find) :
			HttpHandler((HttpServer*)outer, finder), d_that(outer) {}

		void respond(const int socket, const char* ip, const int readlen, const std::string& input) override;
	};

public:
	HttpServer2(
		const olstream& logger,
		const char* root = progdir.getDir(),
		std::string(*rmapper)(std::string&&, const std::string&) = HttpHandler::find,
		std::atomic<bool>* control = nullptr,
		Version version = Version::HTTP_1_1,
		int max_clients = 5
	) :
		HttpServer(logger, root, rmapper, control, version, max_clients),
		t_handler(this, rmapper), t_formatter((HttpServer*)this)
	{
		static_assert(std::is_base_of<HttpServer::Formatter, format_t>::value, "Custom log formatter is not derived from HttpServer::Formatter and will not work corectly");
	}
	HttpServer2(
		olstream&& logger = &std::cout,
		const char* root = progdir.getDir(),
		std::string(*rmapper)(std::string&&, const std::string&) = HttpHandler::find,
		std::atomic<bool>* control = nullptr,
		Version version = Version::HTTP_1_1,
		int max_clients = 5
	) :
		HttpServer(logger, root, rmapper, control, version, max_clients),
		t_handler(this, rmapper), t_formatter((HttpServer*)this)
	{
		static_assert(std::is_base_of<HttpServer::Formatter, format_t>::value, "Custom log formatter is not derived from HttpServer::Formatter and will not work corectly");
	}

	//void serve() override;
	void s_serve1_0() override;
	void s_serve1_1() override;
	void serve_test() override;
private:
	HttpHandler2 t_handler;
	format_t t_formatter;
};

template<class format_t>
void HttpServer2<format_t>::HttpHandler2::respond(const int socket, const char* ip, const int readlen, const std::string& input) {
	Request req(input); //CHECK FOR VALID HTTP
	HeaderList headers;
	std::ifstream reader;
	std::string body, path = this->rfind(d_that->root, req.getResource());

	this->d_that->t_formatter.onRequest(socket, ip, readlen, &req, path.c_str());   //change Request so that we can pass a const in here and still have all the info needed

	(this->response.intHeaders())->reset();

	headers.add(   //headers that apply to all
		Segment( "Server", "Custom C++ HTTP Server (Raspberry Pi)" )
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
			if (this->d_that->version == Version::HTTP_1_0) {   //close connection if http 1.0
				headers.add("Connection", "close");
			}
			else {
				headers.add("Connection", "keep-alive");
			}
			this->response.update(Code::OK, headers, body);
		}
		else {  //send 404
			reader.open(this->rfind(d_that->root, "/error.html"));
			if (reader.is_open()) { //check if error page exists
				body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
				replace(body, "{{code}}", Codes::getString(Code::NOT_FOUND).c_str());
				headers.add(
					Segment( "Content-Type", "text/html" )
				);
			}
			else {  //if not send plain text
				body = "{Error page not found} - Error: 404 Not Found";
				headers.add(
					Segment( "Content-Type", "text/plain" )
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
			reader.open(this->rfind(d_that->root, "/error.html"));
			if (reader.is_open()) { //check error page exists
				body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
				replace(body, "{{code}}", Codes::getString(Code::NOT_FOUND).c_str());
				headers.add(
					Segment( "Content-Type", "text/html" )
				);
			}
			else {  //if not send text
				body = "{Error page not found} - Error: 404 Not Found";
				headers.add(
					Segment( "Content-Type", "text/plain" )
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
		reader.open(this->rfind(d_that->root, "/error.html"));
		if (reader.is_open()) { //check if page exists
			body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
			replace(body, "{{code}}", Codes::getString(Code::METHOD_NOT_ALLOWED).c_str());
			headers.add(
				Segment( "Content-Type", "text/html" )
			);
		}
		else {  //else send plain text
			body = "{Error page not found} - Error: 405 Method Not Allowed";
			headers.add(
				Segment( "Content-Type", "text/plain" )
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
		reader.open(this->rfind(d_that->root, "/error.html"));
		if (reader.is_open()) { //check if page exists
			body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
			replace(body, "{{code}}", Codes::getString(Code::BAD_REQUEST).c_str());
			headers.add(
				Segment( "Content-Type", "text/html" )
			);
		}
		else {  //else send plain text
			body = "{Error page not found} - Error: 400 Bad Request";
			headers.add(
				Segment( "Content-Type", "text/plain" )
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
		reader.open(this->rfind(d_that->root, "/error.html"));
		if (reader.is_open()) { //check if page exists
			body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
			replace(body, "{{code}}", Codes::getString(Code::NOT_IMPLEMENTED).c_str());
			headers.add(
				Segment( "Content-Type", "text/html" )
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
	this->d_that->t_formatter.onResponse(socket, ip, send(socket, ret.c_str(), ret.length(), 0), &(this->response), path.c_str());
	return;
}

template<class format_t>
void HttpServer2<format_t>::s_serve1_0() {
	this->version = Version::HTTP_1_0;  //since this is a public function, the HTTP version could be different that that which was initialized with
	prepServer();
	this->t_formatter.onServerStart();

	int nsock, readlen;
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
		this->t_formatter.onConnect(nsock, ipbuff);
		if ((readlen = recv(this->sockmain, buffer, sizeof(buffer), 0)) < 0) {
			this->lerror("Error recieving data");
		}
		else {
			this->t_handler.respond(nsock, ipbuff, readlen, std::string(buffer));
			this->t_formatter.onDisconnect(nsock, ipbuff);
			memset(&buffer, 0, sizeof(buffer));
		}
		CLOSE_SOCK(nsock);
	}
	CLOSE_SOCK(this->sockmain);
	this->t_formatter.onServerEnd();
}

template<class format_t>
void HttpServer2<format_t>::s_serve1_1() {
	this->version = Version::HTTP_1_1;
	prepServer();
	this->t_formatter.onServerStart();

	int nsock, readlen, rbuff, maxfd = this->sockmain;
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
					this->t_formatter.onConnect(nsock, ipbuff);
				}
				else {  //else a client fd is ready
					if ((readlen = recv(i, buffer, sizeof(buffer), 0)) <= 0) {  //read from client fd
						if (readlen == 0) {
							getSockIp(i, ipbuff);
							this->t_formatter.onDisconnect(i, ipbuff);
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
						this->t_handler.respond(i, ipbuff, readlen, std::string(buffer));
					}
					memset(&buffer, 0, sizeof(buffer));
				}
			}
		}
	}
	CLOSE_SOCK(this->sockmain);
	this->t_formatter.onServerEnd();
}

template<class format_t>
void HttpServer2<format_t>::serve_test() {
	this->logs << "Test server does nothing atm\n";
}

#ifdef _WIN32
#pragma pop_macro("ERROR")
#pragma pop_macro("DELETE")
#endif