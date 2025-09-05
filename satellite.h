#pragma once
#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include "constants.h"

using namespace std;

struct Satellite {
    string id;
    string service_type;
    ServiceType type;
    double urgency;          // Urgency level (1-5 scale)
    double wait_time;        // Minutes waiting in queue
    double priority_score;
    int service_time;        // Service duration
    double entry_time;       // When satellite entered queue
    SatelliteStatus status;

    // Queue management features
    double communication_quality;    // Service quality (0.0 to 1.0)
    double service_rating;          // Service rating
    string assigned_ground_station; // Assigned ground station
    double predicted_success_rate;
    double service_complexity;      // Service complexity score
    int retry_count;

    Satellite() {
        id = "";
        service_type = "";
        type = REGULAR;
        urgency = 0.0;
        wait_time = 0.0;
        priority_score = 0.0;
        service_time = 0;
        entry_time = 0.0;
        status = WAITING;
        communication_quality = 0.0;
        service_rating = 0.0;
        assigned_ground_station = "";
        predicted_success_rate = 0.0;
        service_complexity = 0.0;
        retry_count = 0;
    }

    Satellite(string _id, string _service_type, double _urgency, ServiceType _type) {
        id = _id;
        service_type = _service_type;
        type = _type;
        urgency = _urgency;
        wait_time = 0.0;
        priority_score = 0.0;
        service_time = 0;
        entry_time = (double)time(0);
        status = WAITING;
        communication_quality = 0.8 + (rand() % 20) / 100.0; // 0.8-1.0
        service_rating = 3.0 + (rand() % 20) / 10.0; // 3.0-5.0
        assigned_ground_station = "";
        predicted_success_rate = 0.0;
        service_complexity = 1.0 + (rand() % 50) / 10.0; // 1.0-6.0
        retry_count = 0;
    }
};