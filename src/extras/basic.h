#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <cstring>
#include <unordered_map>

#include "resources.h"

template<class C>
class Instance {
public:
	static Instance& Get() {
		static Instance global;
		return global;
	}
	static Instance& Get(const C* instance) {
		static Instance global;
		global.instances.insert(std::pair<const C*, size_t>(instance, global.size + 1));
		global.size += 1;
		return global;
	}
	void add(const C* instance) {
		this->instances.insert(std::pair<const C*, size_t>(instance, this->size +1));
		this->size += 1;
	}
	size_t indexOf(const C* instance) {
		auto search = this->instances.find(instance);
		return search->second;
	}

private:
	Instance() = default;
	Instance(const Instance&) = delete;
	Instance(Instance&&) = default;
	Instance& operator=(const Instance&) = delete;
	//Instance& operator=(Instance&&) {}

	std::unordered_map<const C*, size_t> instances;
	size_t size;

};

uint16_t toNum(char num);	// return char instead?

char clearEnd(std::string& str);
void replace(std::string& str, const std::string find, const char* replace);

std::string withTime(const char* message);

const char* dateStamp();
const char* dateStamp(time_t* tme);

const char* safeNull(const char* str);
size_t safeLen(const char* str);

template<typename input>
void debug(input identifier) {
	std::cout << dateStamp() << " : DEBUG: " << identifier << newline;
}

void exitError(const char* message);