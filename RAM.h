#pragma once
#include <list>
#include <unordered_map>
#include <string>
#include <ostream>
#include <chrono>

using time_point = std::chrono::system_clock::time_point;

struct Delimiter {
	enum Type {
		MEMORY_BEGIN,
		MEMORY_END,
		SECTION_BEGIN,
		SECTION_END,
		SEGMENT
	} type;

	size_t id;
	size_t offset;

	time_point end;
};

struct Process {
	size_t size;
	size_t miliseconds_duration;
};

struct Segment {
	size_t size;
	size_t process_id;
	size_t id;
	size_t offset = 0;
	bool exist = true;
};

class RAM
{
public:
	RAM(size_t memory_size);

	const std::list<Delimiter>& GetMemory() const;
	const auto& GetSegments() const { return segments; }

	std::string GetAddress(std::pair<size_t, size_t>) const;

	bool ReserveSection(const Process&);
	bool AddSegment(const Segment&);

	void Compress();

	void UpdateTimePoints();

private:
	std::list<Delimiter> main_memory;
	size_t next_free_id = 2;

	std::unordered_map<size_t, std::pair<size_t, std::vector<size_t>>> seg_table;
	std::vector<Segment> segments;
};

std::ostream& operator<< (std::ostream& os, const RAM& memory);
