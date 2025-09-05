// constants.h
#pragma once

// Global constants
const int MAX_SATELLITES = 100;
const int MAX_HISTORY = 200;
const int HASH_TABLE_SIZE = 50;
const int MAX_QUEUES = 5; // Emergency, Communication, Weather, Research, etc.

// Satellite statuses (adapted for queue management)
enum SatelliteStatus {
    WAITING,
    BEING_SERVED,
    SERVED,
    LEFT_QUEUE,
};

// Satellite service types (adapted for queue management)
enum ServiceType {
    REGULAR,    // Regular satellite service
    VIP,        // VIP satellite service  
    EMERGENCY,  // Emergency satellite service
    PRIORITY,   // Priority satellite service
};
