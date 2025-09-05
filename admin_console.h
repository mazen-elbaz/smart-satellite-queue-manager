#pragma once
#include <iostream>
#include <string>
#include <iomanip>
#include "constants.h"

using namespace std;

class AdminConsole {
private:
    // Priority weights
    double urgency_weight;
    double waiting_time_weight;
    double service_type_weight;
    
    // Service type scores
    double emergency_score;
    double vip_score;
    double regular_score;
    double priority_score;
    
    // Fairness parameters
    double max_wait_time;
    double boost_multiplier;
    
    // Simulation parameters
    int simulation_duration;
    double arrival_rate;
    int counters;

public:
    AdminConsole() {
        // Default priority weights
        urgency_weight = 0.5;
        waiting_time_weight = 0.3;
        service_type_weight = 0.2;

        // Default service type scores
        emergency_score = 10.0;
        vip_score = 8.0;
        regular_score = 5.0;
        priority_score = 7.0;

        // Default fairness parameters
        max_wait_time = 25.0; // minutes
        boost_multiplier = 0.5;

        // Default simulation parameters
        simulation_duration = 60; // minutes
        arrival_rate = 0.5; // individuals per minute
        counters = 3;
    }

    // Priority weight management
    void updateWeights(double urgency, double wait_time, double type) {
        urgency_weight = urgency;
        waiting_time_weight = wait_time;
        service_type_weight = type;
        cout << "Priority weights updated successfully." << endl;
    }

    // Service type score management
    void updateServiceScores(double emergency, double vip, double regular, double priority) {
        emergency_score = emergency;
        vip_score = vip;
        regular_score = regular;
        priority_score = priority;
        cout << "Service type scores updated successfully." << endl;
    }

    // Fairness parameters
    void updateFairnessParams(double max_wait, double boost) {
        max_wait_time = max_wait;
        boost_multiplier = boost;
        cout << "Fairness parameters updated successfully." << endl;
    }

    // Simulation parameters
    void updateSimulationParams(int duration, double rate, int num_counters) {
        simulation_duration = duration;
        arrival_rate = rate;
        counters = num_counters;
        cout << "Simulation parameters updated successfully." << endl;
    }

    // Getters
    double getUrgencyWeight() { return urgency_weight; }
    double getWaitTimeWeight() { return waiting_time_weight; }
    double getTypeWeight() { return service_type_weight; }
    double getMaxWaitTime() { return max_wait_time; }
    double getBoostMultiplier() { return boost_multiplier; }
    int getSimulationDuration() { return simulation_duration; }
    double getArrivalRate() { return arrival_rate; }
    int getCounters() { return counters; }

    // Service type score getter
    double getTypeScore(string type) {
        if (type == "emergency") return emergency_score;
        if (type == "vip") return vip_score;
        if (type == "regular") return regular_score;
        if (type == "priority") return priority_score;
        return 0.0;
    }

    // Fairness boost calculation
    double calculateFairnessBoost(double current_wait_time) {
        if (current_wait_time <= max_wait_time) return 0.0;
        
        double extra_wait_time = current_wait_time - max_wait_time;
        return extra_wait_time * boost_multiplier;
    }

    void displaySettings() {
        cout << "\n=== SATELLITE QUEUE MANAGEMENT SYSTEM ===" << endl;
        cout << "System Status: ONLINE" << endl;
        cout << "Ground Stations: 5 Active Worldwide" << endl;
        
        cout << "\nPriority Configuration:" << endl;
        cout << "  Urgency Weight (UW): " << fixed << setprecision(2) << urgency_weight << endl;
        cout << "  Wait Time Weight (WW): " << waiting_time_weight << endl;
        cout << "  Service Type Weight (STW): " << service_type_weight << endl;

        cout << "\nService Type Scores:" << endl;
        cout << "  Emergency: " << emergency_score << " (Highest)" << endl;
        cout << "  VIP: " << vip_score << endl;
        cout << "  Priority: " << priority_score << endl;
        cout << "  Regular: " << regular_score << " (Lowest)" << endl;

        cout << "\nFairness Parameters:" << endl;
        cout << "  Max Wait Time: " << max_wait_time << " minutes" << endl;
        cout << "  Boost Multiplier: " << boost_multiplier << endl;

        cout << "\nSimulation Parameters:" << endl;
        cout << "  Duration: " << simulation_duration << " minutes" << endl;
        cout << "  Arrival Rate: " << arrival_rate << " satellites per minute" << endl;
        cout << "  Counters: " << counters << endl;
        cout << "=================================================" << endl;
    }

    void runAdvancedAdmin() {
        int choice;
        while (true) {
            cout << "\n=== ADMIN CONSOLE ===" << endl;
            cout << "1. View System Status" << endl;
            cout << "2. Update Priority Weights" << endl;
            cout << "3. Configure Service Type Scores" << endl;
            cout << "4. Fairness Parameters" << endl;
            cout << "5. Simulation Parameters" << endl;
            cout << "6. Export Configuration" << endl;
            cout << "7. Exit Admin Console" << endl;
            cout << "Enter choice: ";
            cin >> choice;

            switch (choice) {
            case 1:
                displaySettings();
                break;
            case 2: {
                double urgency, wait_time, type;
                cout << "Enter urgency weight (0.0-1.0): ";
                cin >> urgency;
                cout << "Enter wait time weight (0.0-1.0): ";
                cin >> wait_time;
                cout << "Enter service type weight (0.0-1.0): ";
                cin >> type;

                if (urgency + wait_time + type != 1.0) {
                    cout << "Warning: Weights don't sum to 1.0. Normalizing..." << endl;
                    double total = urgency + wait_time + type;
                    urgency /= total;
                    wait_time /= total;
                    type /= total;
                }

                updateWeights(urgency, wait_time, type);
                break;
            }
            case 3: {
                double emergency, vip, regular, priority;
                cout << "Enter Emergency score (1-10): ";
                cin >> emergency;
                cout << "Enter VIP score (1-10): ";
                cin >> vip;
                cout << "Enter Regular score (1-10): ";
                cin >> regular;
                cout << "Enter Priority score (1-10): ";
                cin >> priority;
                
                updateServiceScores(emergency, vip, regular, priority);
                break;
            }
            case 4: {
                double max_wait, boost;
                cout << "Enter Max Wait Time (minutes): ";
                cin >> max_wait;
                cout << "Enter Boost Multiplier (0.1-1.0): ";
                cin >> boost;
                
                updateFairnessParams(max_wait, boost);
                break;
            }
            case 5: {
                int duration, num_counters;
                double rate;
                cout << "Enter Simulation Duration (minutes): ";
                cin >> duration;
                cout << "Enter Arrival Rate (satellites per minute): ";
                cin >> rate;
                cout << "Enter Number of Counters: ";
                cin >> num_counters;
                
                updateSimulationParams(duration, rate, num_counters);
                break;
            }
            case 6: {
                cout << "\nExport Configuration:" << endl;
                cout << "Current configuration:" << endl;
                cout << "  Urgency Weight: " << urgency_weight << endl;
                cout << "  Wait Time Weight: " << waiting_time_weight << endl;
                cout << "  Service Type Weight: " << service_type_weight << endl;
                cout << "  Emergency Score: " << emergency_score << endl;
                cout << "  VIP Score: " << vip_score << endl;
                cout << "  Regular Score: " << regular_score << endl;
                cout << "  Priority Score: " << priority_score << endl;
                cout << "  Max Wait Time: " << max_wait_time << endl;
                cout << "  Boost Multiplier: " << boost_multiplier << endl;
                cout << "Configuration exported successfully!" << endl;
                break;
            }
            case 7:
                return;
            default:
                cout << "Invalid choice. Please try again." << endl;
            }
        }
    }
};
