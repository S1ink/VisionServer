// #include <string>

// #include <networktables/NetworkTableInstance.h>
// #include "networktables/NetworkTable.h"

// #include "tools/src/server/server.h"
// #include "tools/src/resources.h"


// class HttpNTables : public HttpServer::HttpHandler {
// public:
// 	HttpNTables() = delete;
// 	HttpNTables(HttpServer* s) : HttpHandler(s) {}
// 	~HttpNTables() = default;

// 	void respond(const int socket, const char* ip, const int readlen, const std::string& input, HttpServer::HttpFormatter* formatter) override;

// protected:
// 	//std::string resourceMapper(std::string&& root, const std::string& requested) override;
// 	inline static bool isNtPath(const std::string& req) { return req.at(1) == 'n' && req.at(2) == 't' && req.at(3) == '/'; }
// 	inline static std::string extractNtPath(const std::string& req) { return req.substr(3); }
// 	inline static bool isNTable(const std::string& req) { return isNtPath(req) && req.back() == '/'; }
// 	inline static bool isNtEntry(const std::string& req) { return isNtPath(req) && req.back() != '/'; }
// 	static std::string serializeNtRequest(const std::string& ntreq);
// 	static std::string serializeNtPath(const std::string& path);
// 	static std::string serializeTable(std::shared_ptr<nt::NetworkTable> t);
// 	static std::string serializeEntry(nt::NetworkTableEntry e);
// 	//static void applyNt(const std::string& ntreq);


// };