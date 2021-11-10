#include "http.h"

const std::unordered_map<std::string, Version> Versions::typemap = {
        {"HTTP/1.0", Version::HTTP_1_0},
        {"HTTP/1.1", Version::HTTP_1_1},
        {"HTTP/2.0", Version::HTTP_2_0},
};
const std::unordered_map<Version, std::string> Versions::stringmap = {
    {Version::HTTP_1_0, "HTTP/1.0"},
    {Version::HTTP_1_1, "HTTP/1.1"},
    {Version::HTTP_2_0, "HTTP/2.0"},
};
Version Versions::getType(const std::string& str) {
    auto search = typemap.find(str);
    if (search != typemap.end()) {
        return search->second;
    }
    return Version::ERROR;
}
std::string Versions::getString(Version version) {
    auto search = stringmap.find(version);
    if (search != stringmap.end()) {
        return search->second;
    }
    return std::string();
}
const std::unordered_map<std::string, Method> Methods::typemap = {
    {"GET", Method::GET},
    {"HEAD", Method::HEAD},
    {"POST", Method::POST},
    {"PUT", Method::PUT},
    {"DELETE", Method::DELETE},
    {"TRACE", Method::TRACE},
    {"OPTIONS", Method::OPTIONS},
    {"CONNECT", Method::CONNECT},
    {"PATCH", Method::PATCH},
};
const std::unordered_map<Method, std::string> Methods::stringmap = {
    {Method::GET, "GET"},
    {Method::HEAD, "HEAD"},
    {Method::POST, "POST"},
    {Method::PUT, "PUT"},
    {Method::DELETE, "DELETE"},
    {Method::TRACE, "TRACE"},
    {Method::OPTIONS, "OPTIONS"},
    {Method::CONNECT, "CONNECT"},
    {Method::PATCH, "PATCH"},
};
Method Methods::getType(const std::string& str) {
    auto search = typemap.find(str);
    if (search != typemap.end()) {
        return search->second;
    }
    return Method::ERROR;
}
std::string Methods::getString(Method method) {
    auto search = stringmap.find(method);
    if (search != stringmap.end()) {
        return search->second;
    }
    return std::string();
}
const std::unordered_map<std::string, Code> Codes::typemap = {
    {"100 Continue", Code::CONTINUE}, {"200 OK", Code::OK},
    {"201 Created", Code::CREATED}, {"202 Accepted", Code::ACCEPTED},
    {"204 No Content", Code::NO_CONTENT}, {"205 Reset Content", Code::RESET_CONTENT},
    {"206 Partial Content", Code::PARTIAL_CONTENT},
    {"300 Multiple Choices", Code::MULTIPLE_CHOICES},
    {"301 Moved Permanently", Code::MOVED_PERMANENTLY},
    {"302 Found", Code::FOUND}, {"400 Bad Request", Code::BAD_REQUEST},
    {"401 Unauthorized", Code::UNAUTHORIZED},
    {"402 Payment Required", Code::PAYMENT_REQUIRED},
    {"403 Forbidden", Code::FORBIDDEN},  {"404 Not Found", Code::NOT_FOUND},
    {"405 Method Not Allowed", Code::METHOD_NOT_ALLOWED},
    {"406 Not Acceptable", Code::NOT_ACCEPTABLE},
    {"408 Request Timeout", Code::REQUEST_TIMEOUT}, {"409 Conflict", Code::CONFLICT},
    {"410 Gone", Code::GONE}, {"411 Length Required", Code::LENGTH_REQUIRED},
    {"414 URI Too Long", Code::URI_TOO_LONG},
    {"415 Unsupported Media Type", Code::UNSUPPORTED_MEDIA_TYPE},
    {"418 I'm a teapot", Code::IM_A_TEAPOT},
    {"500 Internal Server Error", Code::INTERNAL_SERVER_ERROR},
    {"501 Not Implemented", Code::NOT_IMPLEMENTED}, {"502 Bad Gateway", Code::BAD_GATEWAY},
    {"503 Service Unavailable", Code::SERVICE_UNAVAILABLE},
    {"504 Gateway Timeout", Code::GATEWAY_TIMEOUT},
    {"505 HTTP Version Not Supported", Code::HTTP_VERSION_NOT_SUPPORTED},
    {"510 Not Extended", Code::NOT_EXTENDED},
    {"511 Network Authentication Required", Code::NETWORK_AUTHICATION_REQIRED},
};
const std::unordered_map<Code, std::string> Codes::stringmap = {
    {Code::CONTINUE, "100 Continue"}, {Code::OK, "200 OK"},
    {Code::CREATED, "201 Created"}, {Code::ACCEPTED, "202 Accepted"},
    {Code::NO_CONTENT, "204 No Content"}, {Code::RESET_CONTENT, "205 Reset Content"},
    {Code::PARTIAL_CONTENT, "206 Partial Content"},
    {Code::MULTIPLE_CHOICES, "300 Multiple Choices"},
    {Code::MOVED_PERMANENTLY, "301 Moved Permanently"},
    {Code::FOUND, "302 Found"}, {Code::BAD_REQUEST, "400 Bad Request"},
    {Code::UNAUTHORIZED, "401 Unauthorized"},
    {Code::PAYMENT_REQUIRED, "402 Payment Required"},
    {Code::FORBIDDEN, "403 Forbidden"}, {Code::NOT_FOUND, "404 Not Found"},
    {Code::METHOD_NOT_ALLOWED, "405 Method Not Allowed"},
    {Code::NOT_ACCEPTABLE, "406 Not Acceptable"},
    {Code::REQUEST_TIMEOUT, "408 Request Timeout"}, {Code::CONFLICT, "409 Conflict"},
    {Code::GONE, "410 Gone"}, {Code::LENGTH_REQUIRED, "411 Length Required"},
    {Code::URI_TOO_LONG, "414 URI Too Long"},
    {Code::UNSUPPORTED_MEDIA_TYPE, "415 Unsupported Media Type"},
    {Code::IM_A_TEAPOT, "418 I'm a teapot"},
    {Code::INTERNAL_SERVER_ERROR, "500 Internal Server Error"},
    {Code::NOT_IMPLEMENTED, "501 Not Implemented"}, {Code::BAD_GATEWAY, "502 Bad Gateway"},
    {Code::SERVICE_UNAVAILABLE, "503 Service Unavailable"},
    {Code::GATEWAY_TIMEOUT, "504 Gateway Timeout"},
    {Code::HTTP_VERSION_NOT_SUPPORTED, "505 HTTP Version Not Supported"},
    {Code::NOT_EXTENDED, "510 Not Extended"},
    {Code::NETWORK_AUTHICATION_REQIRED, "511 Network Authentication Required"},
};
Code Codes::getType(const std::string& str) {
    std::istringstream extract(str);
    int code;
    extract >> code;
    if (code) {
        return Code(code);
    }
    else {
        auto search = typemap.find(str);
        if (search != typemap.end()) {
            return search->second;
        }
        return Code::ERROR;
    }
}
Code Codes::getType(int code) { //find a way to check param
    return Code(code);
}
std::string Codes::getString(Code code) {
    auto search = stringmap.find(code);
    if (search != stringmap.end()) {
        return search->second;
    }
    return std::string();
}
std::string Codes::getString(int code) {
    auto search = stringmap.find(Code(code));
    if (search != stringmap.end()) {
        return search->second;
    }
    return std::to_string(code);
}

Segment::Segment(const std::string& segment) {
    std::istringstream headstream(segment);
    std::getline(headstream, this->key, ':');   //check direct access works
    std::getline(headstream, this->value);
    value.erase(std::remove_if(std::begin(this->value), std::end(this->value), [](char c) {return std::isspace(c); }), std::end(this->value));
}

std::string* Segment::intKey() {
    return &(this->key);
}
std::string* Segment::intValue() {
    return &(this->value);
}

const std::string& Segment::getKey() const {
    return this->key;
}
const std::string& Segment::getValue() const {
    return this->value;
}

std::string Segment::getSerialized() const {
    std::ostringstream segment;
    segment << this->key << ": " << this->value;
    return segment.str();
}
std::string Segment::getSerialized(const std::string& key, const std::string& value) {
    std::ostringstream segment;
    segment << key << ": " << value;
    return segment.str();
}
Segment Segment::getDeserialized(const std::string& segment) {
    std::istringstream headstream(segment);
    std::string key;
    std::string value;
    std::getline(headstream, key, ':');
    std::getline(headstream, value);
    value.erase(std::remove_if(std::begin(value), std::end(value), [](char c) {return std::isspace(c); }), std::end(value));
    return Segment(std::move(key), std::move(value));
}

HeaderList::HeaderList(const size_t size) {
    this->headers.reserve(size);
}
HeaderList::HeaderList(const std::string& body) {
    this->add(body);
}

void HeaderList::add(const Segment& header) {
    this->headers.emplace_back(header);
}
void HeaderList::add(const std::string& key, const std::string& value) {
    this->headers.emplace_back(key, value);
}
void HeaderList::add(const std::vector<Segment>& list) {
    this->headers.reserve((list.size()) + (this->headers.size()));
    this->headers.insert(this->headers.end(), list.begin(), list.end());
}
void HeaderList::add(const std::string& body) {
    std::istringstream text(body);
    std::string buffer;
    while (std::getline(text, buffer)) {
        clearEnd(buffer);
        this->headers.emplace_back(std::move(buffer));
        buffer.clear();
    }
}

void HeaderList::reserve(size_t size) {
    this->headers.reserve(size);
}
void HeaderList::reset() {
    this->headers.clear();
    this->headers.shrink_to_fit();
}

std::string HeaderList::find(std::string key) const {
    auto map = HeaderList::headerMap(this->headers);
    auto itr = map.find(key);
    if (itr != map.end()) {
        return itr->second;
    }
    return std::string();
}
std::string HeaderList::allHeaders() const {
    std::ostringstream buffer;
    for (uint i = 0; i < this->headers.size(); i++) {
        buffer << this->headers[i].getSerialized() << endline;
    }
    return buffer.str();
}

std::vector<Segment>* HeaderList::intHeaders() {
    return &(this->headers);
}

const std::vector<Segment>& HeaderList::getHeaders() const {
    return this->headers;
}

std::unordered_map<std::string, std::string> HeaderList::headerMap(const std::vector<Segment>& headers) {
    std::unordered_map<std::string, std::string> ret;
    for (uint i = 0; i < headers.size(); i++) {
        ret.insert(std::make_pair(headers[i].getKey(), headers[i].getValue()));
    }
    return ret;
}

Request::Request(const std::string& request) {
    std::istringstream rstream(request);
    std::string buffer;
    std::getline(rstream, buffer, space);
    this->method = Methods::getType(buffer);
    std::getline(rstream, this->resource, space);
    std::getline(rstream, buffer);
    clearEnd(buffer);
    this->version = Versions::getType(buffer);
    if ((this->method != Method::ERROR) && (this->version != Version::ERROR)) { //class error var for future reference?
        while (std::getline(rstream, buffer)) {
            //check for blank line (ex. POST -> deal with the payload)
            clearEnd(buffer);
            this->headers.add(buffer);  //IMPLEMENT CHECKS!!!
        }
    }
}

std::string Request::getSerialized() const {
    std::ostringstream buffer;
    buffer << Methods::getString(this->method)
        << space << this->resource
        << space << Versions::getString(this->version)
        << endline
        << this->headers.allHeaders();
    return buffer.str();
}

Version* Request::intVersion() {
    return &(this->version);
}
Method* Request::intMethod() {
    return &(this->method);
}
std::string* Request::intResource() {
    return &(this->resource);
}
HeaderList* Request::intHeaders() {
    return &(this->headers);
}

const Version Request::getVersion() const {
    return this->version;
}
const Method Request::getMethod() const {
    return this->method;
}
const std::string& Request::getResource() const {
    return this->resource;
}
const HeaderList& Request::getHeaders() const {
    return this->headers;
}

std::string Request::getSerialized(Method method, const std::string& resource, std::vector<Segment>& headers, Version version) {
    std::ostringstream buffer;
    buffer << Methods::getString(method) << space << resource << space << Versions::getString(version) << endline;
    for (uint i = 0; i < headers.size(); i++) {
        buffer << headers[i].getSerialized() << endline;
    }
    return buffer.str();
}

void Request::getSerialized(std::ostream& buffer, Method method, const std::string& resource, std::vector<Segment>& headers, Version version) {
    buffer << Methods::getString(method) << space << resource << space << Versions::getString(version) << endline;
    for (uint i = 0; i < headers.size(); i++) {
        buffer << headers[i].getSerialized() << endline;
    }
}

Response::Response(const std::string& response) {   //implement checks when ready to use this form of initialization
    std::istringstream rstream(response);
    std::string buffer;
    std::getline(rstream, buffer, space);
    this->responsecode = Codes::getType(buffer);
    std::getline(rstream, buffer);
    clearEnd(buffer);
    this->version = Versions::getType(buffer);
    while (std::getline(rstream, buffer)) {
        clearEnd(buffer);
        if (buffer.empty()) {
            break;
        }
        this->headers.add(buffer);
    }
    this->body = response.substr(rstream.tellg());
}

void Response::update(Code responsecode, const std::vector<Segment>& headers, const std::string& body) {
    this->responsecode = responsecode;
    this->body = body;
    this->headers.add(headers);
}
void Response::update(Code responsecode, HeaderList& headers, const std::string& body) {
    this->responsecode = responsecode;
    this->body = body;
    this->headers.add(*headers.intHeaders());
}

std::string Response::getSerialized() const {
    std::ostringstream buffer;
    buffer << Versions::getString(this->version) << space << Codes::getString(this->responsecode) << endline;
    buffer << this->headers.allHeaders();
    if (this->body.length() > 0) {
        buffer << endline << this->body << endline;
    }
    return buffer.str();
}

Code* Response::intCode() {
    return &(this->responsecode);
}
Version* Response::intVersion() {
    return &(this->version);
}
std::string* Response::intBody() {
    return &(this->body);
}
HeaderList* Response::intHeaders() {
    return &(this->headers);
}

const Code Response::getCode() const {
    return this->responsecode;
}
const Version Response::getVersion() const {
    return this->version;
}
const std::string& Response::getBody() const {
    return this->body;
}
const HeaderList& Response::getHeaders() const {
    return this->headers;
}

size_t Response::bodyLen() const {
    return this->body.length();
}

std::string Response::getSerialized(Code responsecode, std::vector<Segment>& headers, const std::string& body, Version version) {
    std::ostringstream buffer;
    buffer << Codes::getString(responsecode) << space << Versions::getString(version) << endline;
    for (uint i = 0; i < headers.size(); i++) {
        buffer << headers[i].getSerialized() << endline;
    }
    buffer << endline << body << endline;
    return buffer.str();
}
void Response::getSerialized(std::ostream& buffer, Code responsecode, std::vector<Segment>& headers, const std::string& body, Version version) {
    buffer << Codes::getString(responsecode) << space << Versions::getString(version) << endline;
    for (uint i = 0; i < headers.size(); i++) {
        buffer << headers[i].getSerialized() << endline;
    }
    buffer << endline << body << endline;
}