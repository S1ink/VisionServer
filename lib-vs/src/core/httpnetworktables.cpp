// #include "httpnetworktables.h"


// // std::string HttpNTables::resourceMapper(std::string&& root, const std::string& requested) {
// // 	if(requested.at(1) == 'n' && requested.at(2) == 't' && requested.at(3) == '/') {	// networktables addr

// // 	}
// // }

// std::string HttpNTables::serializeNtRequest(const std::string& ntreq) {
//     if(isNtPath(ntreq)) {
//         std::string_view table = ntreq;
//         table.remove_prefix(3);
//         if(isNTable(ntreq)) {
//             return serializeTable(nt::NetworkTableInstance::GetDefault().GetTable(table));
//         } else {
//             return serializeEntry(nt::NetworkTableInstance::GetDefault().GetEntry(table));
//         }
//     }
//     return std::string();
// }
// std::string HttpNTables::serializeNtPath(const std::string& path) {
//     return path.back() == '/' ? 
//         serializeTable(nt::NetworkTableInstance::GetDefault().GetTable(path)) : 
//         serializeEntry(nt::NetworkTableInstance::GetDefault().GetEntry(path));
// }
// std::string HttpNTables::serializeTable(std::shared_ptr<nt::NetworkTable> t) {
//     std::ostringstream buffer;
//     std::vector<std::string> tables = t->GetSubTables();
//     std::vector<std::string> keys = t->GetKeys();
//     for(size_t i = 0; i < tables.size(); i++) {
//         buffer << tables[i] << newline;
//     }
//     for(size_t i = 0; i < keys.size(); i++) {
//         buffer << serializeEntry(t->GetEntry(keys[i])) << newline;
//     }
//     return buffer.str();
// }
// std::string HttpNTables::serializeEntry(nt::NetworkTableEntry e) {
//     std::ostringstream buffer;
//     buffer << e.GetName() << " : ";
//     switch(e.GetInfo().type) {
//         case(NT_UNASSIGNED): {
//             buffer << "null\n";
//             break;
//         }
//         case(NT_BOOLEAN): {
//             buffer << e.GetBoolean(false) << newline;
//             break;
//         }
//         case(NT_DOUBLE): {
//             buffer << e.GetDouble(0.0) << newline;
//             break;
//         }
//         case(NT_STRING): {
//             buffer << e.GetString("null") << newline;
//             break;
//         }
//         case(NT_BOOLEAN_ARRAY): {
//             for(bool v : e.GetBooleanArray(wpi::span<const int>())) {
//                 buffer << v << ", ";
//             }
//             buffer << newline;
//             break;
//         }
//         case(NT_DOUBLE_ARRAY): {
//             for(double v : e.GetDoubleArray(wpi::span<const double>())) {
//                 buffer << v << ", ";
//             }
//             buffer << newline;
//             break;
//         }
//         case(NT_STRING_ARRAY): {
//             for(const std::string& v : e.GetStringArray(wpi::span<const std::string>())) {
//                 buffer << v << ", ";
//             }
//             buffer << newline;
//             break;
//         }
//         case(NT_RPC):
//         case(NT_RAW):
//         default: {
//             buffer << e.GetRaw("null") << newline;
//         }
//     }
// }

// void HttpNTables::respond(const int socket, const char* ip, const int readlen, const std::string& input, HttpServer::HttpFormatter* formatter) {
// 	this->request_buff.parse(input);
//     HeaderList headers;
//     std::ifstream reader;
//     std::string body, path;

//     if(isNtPath(this->request_buff.getResource())) {
//         path = extractNtPath(this->request_buff.getResource());
//     } else {
//         path = std::move(this->resourceMapper(that->getRoot(), this->request_buff.getResource()));
//     }

//     formatter->onRequest(socket, ip, readlen, &this->request_buff, this->request_buff.getResource().c_str());

//     (this->response.intHeaders())->reset();

//     headers.add(   //headers that apply to all
//         Segment("Server", "Custom C++ HTTP Server (Raspberry Pi)")
//     );

//     switch (this->request_buff.getMethod()) {
//     case Method::GET:   //requesting resource
//     {
//         if(isNtPath(this->request_buff.getResource())) {
//             body = serializeNtPath(path);
//             headers.add({
//                 {"Content-Type", "text/plain"},
//                 {"Content-Length", std::to_string(body.length())}
//             });
//             if (that->getVersion() == Version::HTTP_1_0) {   //close connection if http 1.0
//                 headers.add("Connection", "close");
//             }
//             else {
//                 headers.add("Connection", "keep-alive");
//             }
//             this->response.update(Code::OK, headers, body);
//         } else {
//             reader.open(path.c_str(), std::ios::binary);    //attempt to open resource
//             if (reader.is_open()) { //if resource exists
//                 body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
//                 headers.add({
//                     {"Content-Type", safeMime(path.c_str())},
//                     {"Content-Length", std::to_string(body.length())}
//                 });
//                 if (that->getVersion() == Version::HTTP_1_0) {   //close connection if http 1.0
//                     headers.add("Connection", "close");
//                 }
//                 else {
//                     headers.add("Connection", "keep-alive");
//                 }
//                 this->response.update(Code::OK, headers, body);
//             }
//             else {  //send 404
//                 reader.open(this->resourceMapper(that->getRoot(), "/error.html"));
//                 if (reader.is_open()) { //check if error page exists
//                     body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
//                     replace(body, "{{code}}", Codes::getString(Code::NOT_FOUND).c_str());
//                     headers.add(
//                         Segment("Content-Type", "text/html")
//                     );
//                 }
//                 else {  //if not send plain text
//                     body = "{Error page not found} - Error: 404 Not Found";
//                     headers.add(
//                         Segment("Content-Type", "text/plain")
//                     );
//                 }
//                 headers.add({
//                     {"Content-Length", std::to_string(body.length())},
//                     {"Connection", "close"},
//                     });
//                 this->response.update(Code::NOT_FOUND, headers, body);
//             }
//         }
//         break;
//     }
//     break;
//     case Method::HEAD:  //requesting header only
//     {
//         reader.open(path, std::ios::binary);    //attempt to open resource
//         if (reader.is_open()) { //if exists
//             body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
//             headers.add({
//                 {"Content-Type", safeMime(path.c_str())},
//                 {"Content-Length", std::to_string(body.length())}
//                 });
//             this->response.update(Code::OK, headers);
//         }
//         else {  //else send 404
//             reader.open(this->resourceMapper(that->getRoot(), "/error.html"));
//             if (reader.is_open()) { //check error page exists
//                 body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
//                 replace(body, "{{code}}", Codes::getString(Code::NOT_FOUND).c_str());
//                 headers.add(
//                     Segment("Content-Type", "text/html")
//                 );
//             }
//             else {  //if not send text
//                 body = "{Error page not found} - Error: 404 Not Found";
//                 headers.add(
//                     Segment("Content-Type", "text/plain")
//                 );
//             }
//             headers.add({
//                 {"Content-Length", std::to_string(body.length())},
//                 {"Connection", "close"},
//                 });
//             this->response.update(Code::NOT_FOUND, headers);
//         }
//         break;
//     }
//     break;
//     case Method::CONNECT:
//     case Method::DELETE:
//     case Method::OPTIONS:
//     case Method::PATCH:
//     case Method::POST:  // POST is used when the exact url is not known, or a general action is being commanded
//     case Method::PUT:   // PUT is used when the exact url is known, and can modify prexisting objects
//     case Method::TRACE: //methods not supported
//     {
//         reader.open(this->resourceMapper(this->that->getRoot(), "/error.html"));
//         if (reader.is_open()) { //check if page exists
//             body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
//             replace(body, "{{code}}", Codes::getString(Code::METHOD_NOT_ALLOWED).c_str());
//             headers.add(
//                 Segment("Content-Type", "text/html")
//             );
//         }
//         else {  //else send plain text
//             body = "{Error page not found} - Error: 405 Method Not Allowed";
//             headers.add(
//                 Segment("Content-Type", "text/plain")
//             );
//         }
//         headers.add({
//             {"Allow", "GET, HEAD"}, //only GET and HEAD are suppored
//             {"Content-Length", std::to_string(body.length())},
//             {"Connection", "close"},
//             });
//         this->response.update(Code::METHOD_NOT_ALLOWED, headers, body);
//         break;
//     }
//     case Method::ERROR: //method is the first thing that is parsed, so an error means that the request was invalid
//     {
//         reader.open(this->resourceMapper(that->getRoot(), "/error.html"));
//         if (reader.is_open()) { //check if page exists
//             body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
//             replace(body, "{{code}}", Codes::getString(Code::BAD_REQUEST).c_str());
//             headers.add(
//                 Segment("Content-Type", "text/html")
//             );
//         }
//         else {  //else send plain text
//             body = "{Error page not found} - Error: 400 Bad Request";
//             headers.add(
//                 Segment("Content-Type", "text/plain")
//             );
//         }
//         headers.add({
//             {"Content-Length", std::to_string(body.length())},
//             {"Connection", "close"},
//             });
//         this->response.update(Code::BAD_REQUEST, headers, body);
//         break;
//     }
//     default:    //defaults to 501 NOT IMPLEMENTED
//     {
//         reader.open(this->resourceMapper(that->getRoot(), "/error.html"));
//         if (reader.is_open()) { //check if page exists
//             body = std::string((std::istreambuf_iterator<char>(reader)), (std::istreambuf_iterator<char>()));
//             replace(body, "{{code}}", Codes::getString(Code::NOT_IMPLEMENTED).c_str());
//             headers.add(
//                 Segment("Content-Type", "text/html")
//             );
//         }
//         else {  //else send plain text
//             body = "{Error page not found} - Error: 501 Not Implemented";
//         }
//         headers.add({
//             {"Content-Length", std::to_string(body.length())},
//             {"Connection", "close"},
//             });
//         this->response.update(Code::NOT_IMPLEMENTED, headers, body);
//         break;
//     }
//     }

//     std::string ret = this->response.getSerialized();
//     formatter->onResponse(socket, ip, send(socket, ret.c_str(), ret.length(), 0), &(this->response), path.c_str());
//     return;
// }