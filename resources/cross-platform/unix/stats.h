#pragma once

#ifdef __unix__

#include <array>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>
#include <thread>
#include <iostream>
#include <string.h>
#include <initializer_list>

#include "../basic.h"
#include "../resources.h"

//namespace stats {
//	CE_STR cpu = "/proc/stat";
//	CE_STR network = "/proc/net/dev";
//	CE_STR load = "/proc/loadavg";
//	CE_STR disk = "/proc/diskstats";
//	CE_STR mem = "/proc/meminfo";
//}

//class SYS {
	//	// all stats here
	//};

class CPU {
public:
	class CoreData {
		friend class CPU;
	private:
		enum class States {
			USER, NICE, SYSTEM,
			IDLE, IOWAIT, IRQ,
			SOFTIRQ, STEAL,
			GUEST, GUEST_NICE,
			TOTAL
		};
		static const std::array<States, 2> s_idle;
		static const std::array<States, 8> s_active;

		enum {
			ISMAIN_m = 0b00000001,	// 1 << 0 (first bit represents wheather this core represents total cpu utilization)
			CORENUM_m = 0b11111110,	// 0xFF & !(1 << 0) (all other bits represent the core num - this means the maximum amount with 8 bits is 128 cores)
		};	

	protected:
		uint8_t content = 0;
		std::array<uint64_t, (size_t)States::TOTAL> data;

		void setIsMain(bool ismain);
		void setCoreNum(uint8_t corenum);
		bool isSet();

		bool correctHead(const char* head);	// compares to internal settings and returns if they match
		static bool checkCore(const char* head, uint8_t corenum);
		static bool validLine(const char* head);	// returns if the header starts with "cpu", indicating a valid line to parse

		void parseSettings(const char* strline);	
		void parseSettings(std::istream& reader);	
		void parseData(std::istream& reader);		

		void parse(std::istream& reader);
		void parseLine(std::istream& reader);

	public:
		//void update(std::istream& reader);	// updates from file stream -> updates from previous corenum first by streampos hint, then raw corenum, then falls back to main
		//void update(std::istream& reader, std::streampos pos);
		//void updateAttempt(uint8_t corenum);	// attempts to find corenum specified, falls back to main on failure
		//void updateMain(std::istream& reader);	// updates from main
		//void updateMain();

		CoreData() {}
		//CoreData(uint8_t core);
		CoreData(std::istream& reader);
		//CoreData(std::istream& reader, std::streampos pos);
		CoreData(const CoreData& other);
		CoreData(CoreData&& other);

		CoreData& operator=(const CoreData& other);
		CoreData& operator=(CoreData&& other);

		bool getIsMain();
		uint8_t getCoreNum();

		uint64_t getIdle();		//return value is in usr_hz(jiffies - 1/100sec) -> the amount of time chunks in said state
		uint64_t getActive();
		uint64_t getTotal();
		uint64_t getState(States state);
		uint64_t getStates(States states[], size_t size);
		uint64_t getStates(std::initializer_list<States> states);
	};

	// create a "source" class to wrap the ifstrem -> line finding methods

	typedef std::vector<float> Uvec;		// a UTILIZATION vector which contains info for each core and the average of all cores
	typedef std::vector<CoreData> Svec;		// a SOURCE vector which contains raw CoreData for each core and the average of all cores		

	static CPU& get();

	uint count();
	static uint rawCount();

	void update(Svec& cores);

	float refPercent();
	float refPercent(uint core);
	Uvec fromReference();
	void fromReference(Uvec& storage);

	void average(Svec& first, Svec& second, Uvec& ret);
	Uvec average(Svec& first, Svec& second);

	static float average(CoreData& first, CoreData& second);
	static void averageVec(Svec& first, Svec& second, Uvec& ret);
	static Uvec averageVec(Svec& first, Svec& second);

	template<typename rep, typename period>
	float intervalPercent(CHRONO::duration<rep, period> interval) {
		std::ifstream reader(source);
		this->buffer[0].parse(reader);
		reader.close();
		std::this_thread::sleep_for(interval);
		update(this->reference);
		return average(this->buffer[0], this->reference[0]);
	}
	template<typename rep, typename period>
	float intervalPercent(uint core, CHRONO::duration<rep, period> interval) {
		std::ifstream reader(source);
		if (core > this->c_cores) {
			core = 0;
			this->buffer[core].parse(reader);
		}
		else {
			char buff[5];
			reader.rdbuf()->sgetn(buff, 5);
			while (toNum(buff[3]) != core) {
				while (reader.rdbuf()->sbumpc() != 10);
				reader.rdbuf()->sgetn(buff, 5);
			}
			this->buffer[core].parseSettings(buff);
			this->buffer[core].parseData(reader);
		}
		std::this_thread::sleep_for(interval);
		update(this->reference);
		return average(this->buffer[core], this->reference[core]);
	}
	template<typename rep, typename period>
	Uvec fromInterval(CHRONO::duration<rep, period> interval) {
		update(this->buffer);
		std::this_thread::sleep_for(interval);
		update(this->reference);
		return averageVec(this->buffer, this->reference);
	}

	static float temp();
	//static float ctemp();

	template<typename rep, typename period>
	static float percent(CHRONO::duration<rep, period> interval) {
		std::ifstream reader(source);
		CoreData second, first(reader);
		reader.close();
		std::this_thread::sleep_for(interval);
		reader.open(source);
		second.parse(reader);
		return average(first, second);
	}
	static float percent(int seconds = 1);
private:
	CPU();
	CPU(const CPU&) = delete;

	uint c_cores;
	Svec reference, buffer;
	static CE_STR source = "/proc/stat";	// source of info
};

class NET {
private:
	class Interface {
	public:
		enum class Stats {
			RBYTES, RPACKETS, RERR, RDROP,
			RFIFO, FRAME, RCOMPRESSED, MULTICAST,
			TBYTES, TPACKETS, TERR, TDROP, TFIFO,
			COLLS, CARRIER, TCOMPRESSED, TOTAL
		};
		struct MultiStat {
			ulong sent, recv;
		};

		std::string title;
		std::array<ulong, (uint)Stats::TOTAL> data;

		void update();
		bool update(const std::string& id);
		void parseFrom(const std::string& line);

		Interface();
		Interface(const std::string& id);
		Interface(const Interface& other);

		ulong getSent();	//bytes
		ulong getRecv();
		MultiStat getBytes();
		MultiStat getPackets();
		ulong getStat(Stats stat);

	private:
		static const std::array<Stats, 2> s_speeds;	//use a different system if more stats become relevant
		static const std::array<Stats, 2> s_packets;
	};
public:

private:
	static CE_STR source = "/proc/net/dev";
};

//old
namespace sys {
	namespace cpu {
		static CE_STR source = "/proc/stat";
		constexpr int parsable_states = 10;

		struct LineParse {
			std::string title;
			size_t time[parsable_states];
		};

		struct ActivityData {
			std::string title;
			float activity;
		};

		size_t getIdle(const LineParse& data);
		size_t getActive(const LineParse& data);
		void readMain(LineParse& container);
		void readVector(std::vector<LineParse>& container);
		void convertData(LineParse& snapshot1, LineParse& snapshot2, ActivityData& converted);
		void convertVectorData(std::vector<LineParse>& snapshot1, std::vector<LineParse>& snapshot2, std::vector<ActivityData>& converted);

		template<typename rep, typename period>
		void takeSample(std::vector<LineParse>& vector1, std::vector<LineParse>& vector2, std::vector<ActivityData>& result, CHRONO::duration<rep, period> interval) {
			vector1.clear();
			vector2.clear();
			result.clear();
			readVector(vector1);
			std::this_thread::sleep_for(interval);
			readVector(vector2);
			convertVectorData(vector1, vector2, result);
		}
	}

	float cpuTemp();
	float gpuTemp();

	float cpuPercent(CHRONO::duration<CHRONO::seconds::rep, CHRONO::seconds::period> interval);
}

/* CPU States
user – time spent in user mode.
nice – time spent in user mode with low priority.
system – time spent in system mode.
idle – time spent in the idle task.
iowait –  time waiting for I/O to complete.
irq – time servicing hardware interrupts.
softirq – time servicing software interrupts.
steal – time spent in other operating systems when running in a virtualized environment.
guest – time spent running a virtual CPU for guest operating systems.
guest_nice – time spent running a low priority virtual CPU for guest operating systems.
* * * *
idle + iowait = time doing nothing (usr_hz)
all others = time spend utilized
*/

#else 
#pragma message "This code only works on unix (for now), including will only increase binary size!"
#endif