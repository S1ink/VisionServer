#pragma once

#include <unordered_map>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>

#include "../basic.h"
#include "../resources.h"

//https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol

enum class Version {
	HTTP_1_0,
	HTTP_1_1,
	HTTP_2_0,
	TOTAL,
	ERROR
};
enum class Method {
	GET, HEAD, POST,
	PUT, DELETE, TRACE,
	OPTIONS, CONNECT, PATCH,
	TOTAL, ERROR
};
//Does not include all possible codes, only ~relevant~ ones -> full list here: https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
enum class Code {
	CONTINUE = 100, OK = 200, CREATED = 201, ACCEPTED = 202,
	NO_CONTENT = 204, RESET_CONTENT = 205, PARTIAL_CONTENT = 206,
	MULTIPLE_CHOICES = 300, MOVED_PERMANENTLY = 301, FOUND = 302,
	BAD_REQUEST = 400, UNAUTHORIZED = 401, PAYMENT_REQUIRED = 402,
	FORBIDDEN = 403, NOT_FOUND = 404, METHOD_NOT_ALLOWED = 405,
	NOT_ACCEPTABLE = 406, REQUEST_TIMEOUT = 408, CONFLICT = 409,
	GONE = 410, LENGTH_REQUIRED = 411, URI_TOO_LONG = 414,
	UNSUPPORTED_MEDIA_TYPE = 415, IM_A_TEAPOT = 418,
	INTERNAL_SERVER_ERROR = 500, NOT_IMPLEMENTED = 501,
	BAD_GATEWAY = 502, SERVICE_UNAVAILABLE = 503, GATEWAY_TIMEOUT = 504,
	HTTP_VERSION_NOT_SUPPORTED = 505, NOT_EXTENDED = 510,
	NETWORK_AUTHICATION_REQIRED = 511, ERROR
};

//implement checks on conversion functions
struct Versions {
private:
	static const std::unordered_map<std::string, Version> typemap;
	static const std::unordered_map<Version, std::string> stringmap;
public:
	static Version getType(const std::string& str);
	static std::string getString(Version version);
};
struct Methods {
private:
	static const std::unordered_map<std::string, Method> typemap;
	static const std::unordered_map<Method, std::string> stringmap;
public:
	static Method getType(const std::string& str);
	static std::string getString(Method method);
};
struct Codes {
private:
	static const std::unordered_map<std::string, Code> typemap;
	static const std::unordered_map<Code, std::string> stringmap;
public:
	static Code getType(const std::string& str);
	static Code getType(int code);
	static std::string getString(Code code);
	static std::string getString(int code);
};


//(header)
//This class represents the header segments (a newline-terminated line) found within both HTTP requests and responses
class Segment {
private:
	std::string key;
	std::string value;
public:
	Segment() = delete;
	Segment(const std::string& key, const std::string& value) : key(key), value(value) {}
	Segment(std::string&& key, std::string&& value) : key(std::move(key)), value(std::move(value)) {}
	Segment(const std::string& segment);
	Segment(const std::pair<std::string, std::string>& pair) : key(pair.first), value(pair.second) {}
	Segment(std::pair<std::string, std::string>&& pair) : key(std::move(pair.first)), value(std::move(pair.second)) {}	//worthless?
	~Segment() = default;

	std::string* intKey();
	std::string* intValue();

	const std::string& getKey() const;
	const std::string& getValue() const;

	std::string getSerialized() const;
	static std::string getSerialized(const std::string& key, const std::string& value);
	static Segment getDeserialized(const std::string& segment);
};
//create inherited classes that define common headers ^

class HeaderList {
private:
	std::vector<Segment> headers;
public:
	HeaderList() = default;
	HeaderList(const std::vector<Segment>& list) : headers(list) {}	//these are so that it will accept std::initializer_list without explicit conversion 
	HeaderList(std::vector<Segment>&& list) : headers(std::move(list)) {}
	HeaderList(const size_t size);
	HeaderList(const HeaderList& other) : headers(other.headers) {}
	HeaderList(HeaderList&& other) : headers(std::move(other.headers)) {}
	HeaderList(const std::string& body);
	~HeaderList() = default;

	void add(const Segment& header);
	void add(const std::string& key, const std::string& value);
	void add(const std::vector<Segment>& list);
	void add(const std::string& body);

	void reserve(size_t reserve);
	void reset();

	std::string find(std::string key) const;
	std::string allHeaders() const;

	std::vector<Segment>* intHeaders();

	const std::vector<Segment>& getHeaders() const;

	static std::unordered_map<std::string, std::string> headerMap(const std::vector<Segment>& headers);
};

class Request {
private:
	Method method;
	std::string resource;
	Version version;
	HeaderList headers;
public:
	Request() = delete;
	Request(Method method, const std::string& resource, const HeaderList& headers, Version version = Version::HTTP_1_1)
		: method(method), resource(resource), version(version), headers(headers) {}
	Request(const std::string& reqest);
	Request(const Request& other) : method(other.method), resource(other.resource), version(other.version), headers(other.headers) {}
	Request(Request&& other) : method(other.method), resource(std::move(other.resource)), version(other.version), headers(std::move(other.headers)) {}
	~Request() = default;

	std::string getSerialized() const;

	Version* intVersion();
	Method* intMethod();
	std::string* intResource();
	HeaderList* intHeaders();

	const Version getVersion() const;
	const Method getMethod() const;
	const std::string& getResource() const;
	const HeaderList& getHeaders() const;

	static std::string getSerialized(Method method, const std::string& resource, std::vector<Segment>& headers, Version version = Version::HTTP_1_1);	//add overload with {move}
	static void getSerialized(std::ostream& buffer, Method method, const std::string& resource, std::vector<Segment>& headers, Version version = Version::HTTP_1_1);
	//static (deserialize) {=constructor} methods here?
};

class Response {
private:
	Code responsecode;
	Version version;
	HeaderList headers;	//std::map<std::string, std::vector<Segment> > -> for complete RFC standard
	std::string body;
public:
	Response() = delete;
	Response(Version version = Version::HTTP_1_1) : version(version) {}
	Response(Code responsecode, const HeaderList& headers, const std::string& body = std::string(), Version version = Version::HTTP_1_1)
		: responsecode(responsecode), version(version), headers(headers), body(body) {}	//add overload with {move}
	Response(const std::string& response);
	Response(const Response& other) : responsecode(other.responsecode), version(other.version), headers(other.headers), body(other.body) {}
	Response(Response&& other) : responsecode(other.responsecode), version(other.version), headers(std::move(other.headers)), body(std::move(other.body)) {}
	~Response() = default;

	void update(Code responsecode, const std::vector<Segment>& headers, const std::string& body = std::string());
	void update(Code responsecode, HeaderList& headers, const std::string& body = std::string());
	std::string getSerialized() const;

	Code* intCode();
	Version* intVersion();
	std::string* intBody();
	HeaderList* intHeaders();

	const Code getCode() const;
	const Version getVersion() const;
	const std::string& getBody() const;
	const HeaderList& getHeaders() const;

	size_t bodyLen() const;

	static std::string getSerialized(Code responsecode, std::vector<Segment>& headers, const std::string& body = std::string(), Version version = Version::HTTP_1_1);
	static void getSerialized(std::ostream& buffer, Code responsecode, std::vector<Segment>& headers, const std::string& body = std::string(), Version version = Version::HTTP_1_1);
	//static (deserialize) {=constructor} methods here?
};