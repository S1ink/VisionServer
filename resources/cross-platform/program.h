#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

#include "resources.h"

class Pdir {
private:
	Pdir() {}
	Pdir(const Pdir&) = delete;
	//Pdir(Pdir&&);

	std::string dir;
public:
	static Pdir& get();

	const char* setDir(const std::string& pname);
	const char* getDir();
	std::string getDirSlash();
	std::string getItem(const std::string& itempath);
	std::string getRawItem(const std::string& item);
	std::string* intDir();

	static std::string directory(const std::string& fullpath);
	static std::string s_directory(const std::string& fullpath);
};

extern Pdir& progdir;

class ArgsHandler {
public:
	class Variable {	//supports bool, char, int, uint, float, long, str(std)
	private:
		typedef void(Variable::* extractor)(const char*);

		void* data;	//smart?
		extractor ext;

		void extractBool(const char* str);
		void extractChar(const char* str);
		void extractInt(const char* str);
		void extractUint(const char* str);
		void extractFloat(const char* str);
		void extractLong(const char* str);
		void extractStr(const char* str);
	public:
		//Variable() {}
		Variable(bool* item) : data(item), ext(&Variable::extractBool) {}
		Variable(char* item) : data(item), ext(&Variable::extractChar) {}
		Variable(int* item) : data(item), ext(&Variable::extractInt) {}
		Variable(uint* item) : data(item), ext(&Variable::extractUint) {}
		Variable(float* item) : data(item), ext(&Variable::extractFloat) {}
		Variable(long* item) : data(item), ext(&Variable::extractLong) {}
		Variable(std::string* item) : data(item), ext(&Variable::extractStr) {}

		void extract(const char* str);
	};

	static ArgsHandler& get();

	std::unordered_map<std::string, Variable>* getVars();
	void insertVars(std::initializer_list< std::pair<const std::string, Variable> > list);
	uint parse(int argc, char* argv[]);
private:
	ArgsHandler() {}
	ArgsHandler(const ArgsHandler&) = delete;
	//ArgsHandler(ArgsHandler&&);

	std::unordered_map<std::string, Variable> vars;
};