#include "RAM.h"
#include <iomanip>
#include <sstream>

using namespace std;

RAM::RAM(size_t memory_size) :
	main_memory {
		{Delimiter::MEMORY_BEGIN, 0, 0, time_point::max() },
		{Delimiter::MEMORY_END, 1, memory_size - 1, time_point::max() }
	}
{}

const list<Delimiter>& RAM::GetMemory() const {
	return main_memory;
}

bool RAM::ReserveSection(const Process& section) {
	auto previousDelimiterIt = GetMemory().begin();
	bool is_first = true;
	for (auto del = GetMemory().begin(); del != GetMemory().end(); del++) {
		if (is_first) {
			is_first = false;
			continue;
		}
		if (((del->offset - previousDelimiterIt->offset) >= section.size)
			&& (previousDelimiterIt->type == Delimiter::MEMORY_BEGIN 
				|| previousDelimiterIt->type == Delimiter::SECTION_END)) {
			auto end_point = chrono::system_clock::now() + chrono::milliseconds(section.miliseconds_duration);

			seg_table[next_free_id] = { section.size, {} };

			auto it = main_memory.insert(next(previousDelimiterIt), {
				Delimiter::SECTION_BEGIN,
				next_free_id,
				previousDelimiterIt->offset,
				end_point
			});
			main_memory.insert(next(it), { 
				Delimiter::SECTION_END,
				next_free_id,
				previousDelimiterIt->offset + section.size,
				end_point
			});
			next_free_id++;
			return true;
		}
		previousDelimiterIt = del;
	}
	return false;
}

bool RAM::AddSegment(const Segment& segment) {
	if (seg_table.count(segment.process_id)) {
		if (seg_table.at(segment.process_id).first >= segment.size) {
			Segment temp{ segment };
			temp.id = segments.size();
			if (seg_table.at(segment.process_id).second.size()) {
				temp.offset = segments.at(seg_table.at(segment.process_id).second.back()).offset +
					segments.at(seg_table.at(segment.process_id).second.back()).size;
			}
			segments.push_back(temp);
			seg_table.at(segment.process_id).first -= segment.size;
			seg_table.at(segment.process_id).second.push_back(temp.id);
			return true;
		}
		else false;
	}
	else true;
}

void RAM::Compress() {
	size_t offset = 0;
	for (auto del = main_memory.begin(); del != main_memory.end(); del++) {
		if (del->type == Delimiter::SECTION_BEGIN && del->offset > offset) {
			size_t difference = del->offset - offset;
			for (auto& seg_id : seg_table[del->id].second) {
				segments[seg_id].offset -= difference;
			}
			del->offset -= difference; // SECTION BEGIN
			del++;
			del->offset -= difference; // SECTION END
		}
		if (del->type == Delimiter::SECTION_END) {
			offset = del->offset;
		}
	}
}

std::string RAM::GetAddress(std::pair<size_t, size_t> virtual_address) const {
	stringstream ss;
	ss << resetiosflags(ios::dec | ios::left)
		<< setiosflags(ios::hex | ios::uppercase | ios::showbase | ios::right);
	if (segments.size() >= virtual_address.first) {
		ss << "[ERROR] Segment " << virtual_address.first << " doesn't exist";
	}
	else if (segments[virtual_address.first].size < virtual_address.second) {
		ss << "[ERROR] Segment " << virtual_address.first << " less then " << virtual_address.second;
	}
	else if(segments[virtual_address.first].exist) {
		ss << segments[virtual_address.first].offset + virtual_address.second;
	}
	else {
		ss << "[ERROR] Segment " << virtual_address.first << " doesn't exist";
	}

	return ss.str();
}


void RAM::UpdateTimePoints() {
	for (auto it = main_memory.begin(); it != main_memory.end(); it++) {
		if (it->end < chrono::system_clock::now()) {
			for (auto& id : seg_table[it->id].second) {
				segments[id].exist = false;
			}
			it = main_memory.erase(it);
			it--;
		}
	}
}

ostream& operator<< (ostream& os, const RAM& memory) {
	stringstream ss;
	ss << resetiosflags(ios::dec | ios::left) 
		<< setiosflags(ios::hex | ios::uppercase | ios::showbase | ios::right);

	for (const auto& del : memory.GetMemory()) {
		ss << setw(10) << del.offset << "\t";

		switch (del.type) {
		case Delimiter::MEMORY_BEGIN:	{ ss << "BEGIN";  break; }
		case Delimiter::MEMORY_END:		{ ss << "END";  break; }
		case Delimiter::SECTION_BEGIN:	{ 
			ss << "SECTION " << del.id << " BEGIN" << endl;
			for (const auto& segment : memory.GetSegments()) {
				if (segment.process_id == del.id) {
					ss << setw(10) << segment.offset << "\tSEGMENT " << segment.id << " : " << segment.size << endl;
				}
			}
			break;
		}
		case Delimiter::SECTION_END:	{ ss << "SECTION " << del.id << " END";  break; }
		}

		ss << endl;
	}

	os << ss.str();
	return os;
}