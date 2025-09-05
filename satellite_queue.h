#pragma once
#include <iostream>
#include <string>
#include "constants.h"
#include "satellite.h"

using namespace std;

struct SatelliteQueue {
    Satellite satellites[MAX_SATELLITES];
    int size;
    string queue_name; // e.g., "Emergency", "VIP", "Regular"

    SatelliteQueue() {
        size = 0;
        queue_name = "Default";
    }

    SatelliteQueue(string name) {
        size = 0;
        queue_name = name;
    }

    void add(Satellite sat) {
        if (size < MAX_SATELLITES) {
            satellites[size++] = sat;
        }
    }

    bool isEmpty() const {
        return size == 0;
    }

    Satellite* peek() {
        if (isEmpty()) return nullptr;
        return &satellites[0];
    }

    void removeAt(int index) {
        if (index >= 0 && index < size) {
            for (int i = index; i < size - 1; ++i) {
                satellites[i] = satellites[i + 1];
            }
            size--;
        }
    }

    void clear() {
        size = 0;
    }

    // Get satellite with highest priority
    Satellite* getHighestPriority() {
        if (isEmpty()) return nullptr;
        
        Satellite* highest = &satellites[0];
        for (int i = 1; i < size; i++) {
            if (satellites[i].priority_score > highest->priority_score) {
                highest = &satellites[i];
            }
        }
        return highest;
    }

    // Remove specific satellite by ID
    bool removeById(string id) {
        for (int i = 0; i < size; i++) {
            if (satellites[i].id == id) {
                removeAt(i);
                return true;
            }
        }
        return false;
    }

    // Get all satellites (for merging)
    void getAllSatellites(Satellite* result, int& count) {
        count = 0;
        for (int i = 0; i < size && count < MAX_SATELLITES; i++) {
            result[count++] = satellites[i];
        }
    }

    // Update wait times for all satellites
    void updateWaitTimes(double current_time) {
        for (int i = 0; i < size; i++) {
            satellites[i].wait_time = (current_time - satellites[i].entry_time) / 60.0;
        }
    }
};
