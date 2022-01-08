#include <iostream>
#include <string>
#include <queue>
#include <utility>
#include "RAM.h"

using namespace std;

bool GetRequest(queue<Process>& processes, queue<Segment>& segments, pair<size_t, size_t>& virtual_address, RAM& memory) {
    string request;

    cin >> request;
    if (request == "ADD") {
        size_t dur;
        cin >> dur;
        processes.push({ static_cast<size_t>(rand() % 100), dur }); // random process size and custom lifetime
        return false;
    }
    else if (request == "SEG") {
        size_t process, size;
        cin >> process >> size;
        segments.push({ size, process });
        return false;
    }
    else if (request == "GET") {
        cin >> virtual_address.first >> virtual_address.second;
        return true;
    }
    else if (request == "COMPRESS") {
        memory.Compress();
    }
    return false;
}

int main() {
    RAM memory(255);
    queue<Process> processes;
    queue<Segment> segments;
    pair<size_t, size_t> virtual_address;
    string last_converted_address{};

    while (1) {
        cout << memory;
        cout << endl << last_converted_address << endl;
        if (GetRequest(processes, segments, virtual_address, memory)) {
            last_converted_address = memory.GetAddress(virtual_address);
        }

        if (processes.size()) {
            if (memory.ReserveSection(processes.back())) {
                processes.pop();
            }
        }

        if (segments.size()) {
            if (memory.AddSegment(segments.back())) {
                segments.pop();
            }
        }

        memory.UpdateTimePoints();
        system("cls");
    }
}

