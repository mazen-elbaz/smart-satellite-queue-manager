#pragma once
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "constants.h"

using namespace std;

class OrbitalMechanics {
public:
    struct OrbitalElements {
        double semi_major_axis;     // km
        double eccentricity;        // 0-1
        double inclination;         // degrees
        double longitude_of_node;   // degrees
        double argument_of_perigee; // degrees
        double mean_anomaly;        // degrees
        double epoch;               // time reference
    };

    struct Position3D {
        double x, y, z; // km
        double lat, lon, alt; // degrees, degrees, km
    };

    struct GroundStation {
        double latitude;    // degrees
        double longitude;   // degrees
        double altitude;    // meters
        string name;
    };

    GroundStation ground_stations[5];
    int num_stations;

    OrbitalMechanics() {
        // Initialize ground stations worldwide
        ground_stations[0] = { 30.0, 31.0, 100, "Cairo Station" };
        ground_stations[1] = { 37.4, -122.1, 56, "California Station" };
        ground_stations[2] = { 51.5, 0.1, 25, "London Station" };
        ground_stations[3] = { 35.7, 139.7, 40, "Tokyo Station" };
        ground_stations[4] = { -34.6, -58.4, 25, "Buenos Aires Station" };
        num_stations = 5;
    }

    // Calculate satellite position based on orbital elements and time
    Position3D calculateSatellitePosition(const OrbitalElements& elements, double time_minutes) {
        Position3D pos;

        // Simplified orbital mechanics calculation
        double mean_motion = sqrt(398600.4418 / pow(elements.semi_major_axis, 3)); // rad/s
        double time_seconds = time_minutes * 60;
        double mean_anomaly_current = elements.mean_anomaly + mean_motion * time_seconds;

        // Convert to Cartesian coordinates (simplified)
        double radius = elements.semi_major_axis * (1 - elements.eccentricity * cos(mean_anomaly_current));
        pos.x = radius * cos(mean_anomaly_current);
        pos.y = radius * sin(mean_anomaly_current);
        pos.z = radius * sin(elements.inclination * PI / 180.0);

        // Convert to lat/lon/alt
        pos.lat = asin(pos.z / radius) * 180.0 / PI;
        pos.lon = atan2(pos.y, pos.x) * 180.0 / PI;
        pos.alt = radius - EARTH_RADIUS;

        return pos;
    }

    // Calculate visibility window for satellite from ground station
    double calculateVisibilityWindow(const OrbitalElements& elements, int station_index) {
        if (station_index >= num_stations) return 0.0;

        GroundStation station = ground_stations[station_index];
        double max_visible_time = 0.0;

        // Check visibility over next 24 hours
        for (double time = 0; time < 1440; time += 1.0) { // 1440 minutes = 24 hours
            Position3D sat_pos = calculateSatellitePosition(elements, time);

            // Calculate elevation angle
            double distance = sqrt(pow(sat_pos.x, 2) + pow(sat_pos.y, 2) + pow(sat_pos.z, 2));
            double elevation = asin((sat_pos.alt) / distance) * 180.0 / PI;

            if (elevation > 10.0) { // Satellite is visible (above 10 degrees)
                max_visible_time = max(max_visible_time, 15.0 - time / 60.0); // Approximate remaining time
            }
        }

        return max_visible_time;
    }

    // Get orbital parameters based on satellite type
    OrbitalElements getOrbitalElements(OrbitType orbit_type, string sat_id) {
        OrbitalElements elements;

        switch (orbit_type) {
        case LEO:
            elements.semi_major_axis = 6800 + (rand() % 1000); // 400-1400 km altitude
            elements.eccentricity = 0.001 + (rand() % 50) / 1000.0;
            elements.inclination = 45 + (rand() % 90);
            break;
        case MEO:
            elements.semi_major_axis = 13000 + (rand() % 7000); // 6600-13600 km altitude
            elements.eccentricity = 0.01 + (rand() % 20) / 1000.0;
            elements.inclination = 30 + (rand() % 60);
            break;
        case GEO:
            elements.semi_major_axis = 42164; // Geostationary altitude
            elements.eccentricity = 0.001;
            elements.inclination = 0.1 + (rand() % 5);
            break;
        }

        elements.longitude_of_node = rand() % 360;
        elements.argument_of_perigee = rand() % 360;
        elements.mean_anomaly = rand() % 360;
        elements.epoch = time(0);

        return elements;
    }

    string getBestGroundStation(const OrbitalElements& elements) {
        double best_visibility = 0.0;
        int best_station = 0;

        for (int i = 0; i < num_stations; i++) {
            double visibility = calculateVisibilityWindow(elements, i);
            if (visibility > best_visibility) {
                best_visibility = visibility;
                best_station = i;
            }
        }

        return ground_stations[best_station].name;
    }
};