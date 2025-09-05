//#include <iostream>
//#include <string>
//#include <ctime>
//#include <cstdlib>
//#include <iomanip>
//#include <cmath>
//#include <fstream>
//#include <sstream>
//#include "admin_console.h"
//#include "constants.h"
//#include "hash_table.h"
//#include "satellite.h"
//#include "satellite_queue.h"
//
//using namespace std;
//
//class SmartQueueManagementSystem {
//private:
//    HashTable<Satellite> satellite_table;
//    SatelliteQueue emergency_queue;
//    SatelliteQueue vip_queue;
//    SatelliteQueue regular_queue;
//    SatelliteQueue priority_queue;
//    Satellite history_log[MAX_HISTORY];
//    int history_count;
//    AdminConsole* admin;
//
//    // System statistics
//    int total_communications;
//    int successful_communications;
//    double system_uptime;
//
//    double getCurrentTime() {
//        return (double)time(0);
//    }
//
//    double getWaitTime(Satellite& sat) {
//        return (getCurrentTime() - sat.entry_time) / 60.0;
//    }
//
//public:
//    SmartQueueManagementSystem(AdminConsole* admin_console) {
//        admin = admin_console;
//        history_count = 0;
//        total_communications = 0;
//        successful_communications = 0;
//        system_uptime = 0.0;
//        
//        // Initialize queues with names
//        emergency_queue = SatelliteQueue("Emergency");
//        vip_queue = SatelliteQueue("VIP");
//        regular_queue = SatelliteQueue("Regular");
//        priority_queue = SatelliteQueue("Priority");
//    }
//
//    // 1. Modular Priority Engine
//    double calculatePriority(Satellite& sat) {
//        double priority = sat.urgency * admin->getUrgencyWeight() +
//            sat.wait_time * admin->getWaitTimeWeight() +
//            admin->getTypeScore(sat.service_type) * admin->getTypeWeight();
//
//        // Apply fairness boost if waiting too long
//        double fairness_boost = admin->calculateFairnessBoost(sat.wait_time);
//        priority += fairness_boost;
//
//        return priority;
//    }
//
//    // 2. Dynamic Priority Updates
//    void updateAllPriorities() {
//        double current_time = getCurrentTime();
//        
//        // Update emergency queue
//        emergency_queue.updateWaitTimes(current_time);
//        for (int i = 0; i < emergency_queue.size; i++) {
//            emergency_queue.satellites[i].priority_score = calculatePriority(emergency_queue.satellites[i]);
//        }
//        
//        // Update VIP queue
//        vip_queue.updateWaitTimes(current_time);
//        for (int i = 0; i < vip_queue.size; i++) {
//            vip_queue.satellites[i].priority_score = calculatePriority(vip_queue.satellites[i]);
//        }
//        
//        // Update regular queue
//        regular_queue.updateWaitTimes(current_time);
//        for (int i = 0; i < regular_queue.size; i++) {
//            regular_queue.satellites[i].priority_score = calculatePriority(regular_queue.satellites[i]);
//        }
//        
//        // Update priority queue
//        priority_queue.updateWaitTimes(current_time);
//        for (int i = 0; i < priority_queue.size; i++) {
//            priority_queue.satellites[i].priority_score = calculatePriority(priority_queue.satellites[i]);
//        }
//    }
//
//    void addSatellite(string id, string service_type, double urgency, ServiceType type) {
//        Satellite sat(id, service_type, urgency, type);
//        sat.wait_time = 0.0;
//
//        // Calculate initial priority
//        sat.priority_score = calculatePriority(sat);
//
//        // Simple success rate based on service quality
//        if (sat.communication_quality > 0.8) {
//            sat.predicted_success_rate = 0.9; // 90% success
//        } else if (sat.communication_quality > 0.6) {
//            sat.predicted_success_rate = 0.7; // 70% success
//        } else {
//            sat.predicted_success_rate = 0.5; // 50% success
//        }
//
//        // Insert into hash table
//        satellite_table.insert(id, sat);
//
//        // Add to appropriate queue
//        if (service_type == "emergency") {
//            emergency_queue.add(sat);
//        }
//        else if (service_type == "vip") {
//            vip_queue.add(sat);
//        }
//        else if (service_type == "priority") {
//            priority_queue.add(sat);
//        }
//        else {
//            regular_queue.add(sat);
//        }
//
//        cout << "Satellite " << id << " added with priority "
//            << fixed << setprecision(1) << sat.priority_score << endl;
//        cout << "Service Type: " << service_type << endl;
//        cout << "Success probability: " << setprecision(0) << sat.predicted_success_rate * 100 << "%" << endl;
//        cout << "Service quality: " << setprecision(2) << sat.communication_quality << endl;
//    }
//
//    // 3. Multiple Queues with Merging
//    Satellite* getNextSatellite() {
//        // Find the satellite with highest priority across all queues
//        Satellite* highest_priority_sat = nullptr;
//        double highest_priority = -1.0;
//        
//        // Check emergency queue
//        if (!emergency_queue.isEmpty()) {
//            Satellite* sat = emergency_queue.getHighestPriority();
//            if (sat && sat->priority_score > highest_priority) {
//                highest_priority = sat->priority_score;
//                highest_priority_sat = sat;
//            }
//        }
//        
//        // Check VIP queue
//        if (!vip_queue.isEmpty()) {
//            Satellite* sat = vip_queue.getHighestPriority();
//            if (sat && sat->priority_score > highest_priority) {
//                highest_priority = sat->priority_score;
//                highest_priority_sat = sat;
//            }
//        }
//        
//        // Check priority queue
//        if (!priority_queue.isEmpty()) {
//            Satellite* sat = priority_queue.getHighestPriority();
//            if (sat && sat->priority_score > highest_priority) {
//                highest_priority = sat->priority_score;
//                highest_priority_sat = sat;
//            }
//        }
//        
//        // Check regular queue
//        if (!regular_queue.isEmpty()) {
//            Satellite* sat = regular_queue.getHighestPriority();
//            if (sat && sat->priority_score > highest_priority) {
//                highest_priority = sat->priority_score;
//                highest_priority_sat = sat;
//            }
//        }
//        
//        return highest_priority_sat;
//    }
//
//    // 4. Fairness Monitor
//    void checkFairness() {
//        double current_time = getCurrentTime();
//        double max_wait = admin->getMaxWaitTime();
//        
//        // Check all queues for fairness violations
//        for (int i = 0; i < emergency_queue.size; i++) {
//            double wait_time = (current_time - emergency_queue.satellites[i].entry_time) / 60.0;
//            if (wait_time > max_wait) {
//                cout << "Warning: Satellite " << emergency_queue.satellites[i].id 
//                     << " has waited " << setprecision(1) << wait_time 
//                     << " minutes (" << (wait_time - max_wait) << " minutes over threshold)." << endl;
//            }
//        }
//        
//        for (int i = 0; i < vip_queue.size; i++) {
//            double wait_time = (current_time - vip_queue.satellites[i].entry_time) / 60.0;
//            if (wait_time > max_wait) {
//                cout << "Warning: Satellite " << vip_queue.satellites[i].id 
//                     << " has waited " << setprecision(1) << wait_time 
//                     << " minutes (" << (wait_time - max_wait) << " minutes over threshold)." << endl;
//            }
//        }
//        
//        for (int i = 0; i < regular_queue.size; i++) {
//            double wait_time = (current_time - regular_queue.satellites[i].entry_time) / 60.0;
//            if (wait_time > max_wait) {
//                cout << "Warning: Satellite " << regular_queue.satellites[i].id 
//                     << " has waited " << setprecision(1) << wait_time 
//                     << " minutes (" << (wait_time - max_wait) << " minutes over threshold)." << endl;
//            }
//        }
//        
//        for (int i = 0; i < priority_queue.size; i++) {
//            double wait_time = (current_time - priority_queue.satellites[i].entry_time) / 60.0;
//            if (wait_time > max_wait) {
//                cout << "Warning: Satellite " << priority_queue.satellites[i].id 
//                     << " has waited " << setprecision(1) << wait_time 
//                     << " minutes (" << (wait_time - max_wait) << " minutes over threshold)." << endl;
//            }
//        }
//    }
//
//    void processSatellite(Satellite& sat) {
//        cout << "Processing Satellite " << sat.id << "..." << endl;
//        sat.status = SERVED;
//        total_communications++;
//        if (sat.predicted_success_rate > 0.5) {
//            successful_communications++;
//            cout << "Successfully served!" << endl;
//        }
//        else {
//            cout << "Low success rate. Marked as failed." << endl;
//        }
//    }
//
//    void serveNextSatellite() {
//        updateAllPriorities(); // Dynamic priority updates
//        checkFairness(); // Fairness monitor
//        
//        Satellite* next_sat = getNextSatellite();
//        if (next_sat) {
//            processSatellite(*next_sat);
//            // Log served satellite
//            if (history_count < MAX_HISTORY) {
//                history_log[history_count++] = *next_sat;
//            }
//            
//            // Remove from its queue
//            bool found = false;
//            found = emergency_queue.removeById(next_sat->id);
//            if (!found) found = vip_queue.removeById(next_sat->id);
//            if (!found) found = priority_queue.removeById(next_sat->id);
//            if (!found) found = regular_queue.removeById(next_sat->id);
//        }
//        else {
//            cout << "No satellites in queue to serve." << endl;
//        }
//    }
//
//    // 5. Simulation Mode
//    void runSimulation() {
//        int duration = admin->getSimulationDuration();
//        double arrival_rate = admin->getArrivalRate();
//        int counters = admin->getCounters();
//        
//        cout << "\n=== SATELLITE QUEUE SIMULATION ===" << endl;
//        cout << "Duration: " << duration << " minutes" << endl;
//        cout << "Arrival Rate: " << arrival_rate << " satellites per minute" << endl;
//        cout << "Counters: " << counters << endl;
//
//        srand(time(0));
//
//        for (int minute = 1; minute <= duration; minute++) {
//            // Random satellite arrival
//            double prob = ((double)rand() / RAND_MAX);
//            if (prob < arrival_rate) {
//                // Randomly assign type
//                string types[] = { "emergency", "vip", "regular", "priority" };
//                string type = types[rand() % 4];
//
//                // Generate ID and urgency
//                string id = "SIM-" + to_string(rand() % 1000 + minute);
//                double urgency = rand() % 5 + 1; // 1-5 scale
//
//                // Random service type
//                ServiceType service_type = static_cast<ServiceType>(rand() % 4);
//
//                addSatellite(id, type, urgency, service_type);
//            }
//
//            // Try to serve satellites (based on number of counters)
//            for (int counter = 0; counter < counters; counter++) {
//                Satellite* sat = getNextSatellite();
//                if (sat) {
//                    processSatellite(*sat);
//                    // Remove from queue
//                    emergency_queue.removeById(sat->id);
//                    vip_queue.removeById(sat->id);
//                    priority_queue.removeById(sat->id);
//                    regular_queue.removeById(sat->id);
//                }
//            }
//
//            // Display queue status every 10 minutes
//            if (minute % 10 == 0) {
//                cout << "\n--- Time: " << minute << " minutes ---" << endl;
//                cout << "Emergency Queue: " << emergency_queue.size << " satellites" << endl;
//                cout << "VIP Queue: " << vip_queue.size << " satellites" << endl;
//                cout << "Priority Queue: " << priority_queue.size << " satellites" << endl;
//                cout << "Regular Queue: " << regular_queue.size << " satellites" << endl;
//            }
//        }
//
//        cout << "\nSimulation complete. Total communications: " << total_communications
//            << ", Successes: " << successful_communications << endl;
//    }
//
//    // 6. Advanced Reporting and Sorting
//    void generateReport(string filter_type = "", string sort_by = "wait_time") {
//        cout << "\n=== ADVANCED REPORT ===" << endl;
//        cout << "Filter: " << (filter_type.empty() ? "All" : filter_type) << endl;
//        cout << "Sort by: " << sort_by << endl;
//        cout << "ID | Type | Wait Time | Service Time | Priority" << endl;
//        cout << "------------------------------------------------" << endl;
//        
//        // Collect all satellites from history
//        for (int i = 0; i < history_count; i++) {
//            if (filter_type.empty() || history_log[i].service_type == filter_type) {
//                cout << history_log[i].id << " | " 
//                     << history_log[i].service_type << " | "
//                     << setprecision(1) << history_log[i].wait_time << " min | "
//                     << history_log[i].service_time << " min | "
//                     << setprecision(1) << history_log[i].priority_score << endl;
//            }
//        }
//    }
//
//    void displayQueueStatus() {
//        cout << "\n=== QUEUE STATUS ===" << endl;
//        cout << "Emergency Queue: " << emergency_queue.size << " satellites" << endl;
//        cout << "VIP Queue: " << vip_queue.size << " satellites" << endl;
//        cout << "Priority Queue: " << priority_queue.size << " satellites" << endl;
//        cout << "Regular Queue: " << regular_queue.size << " satellites" << endl;
//        cout << "Total in system: " << (emergency_queue.size + vip_queue.size + 
//                                       priority_queue.size + regular_queue.size) << " satellites" << endl;
//    }
//
//    void runManager() {
//        int choice;
//        while (true) {
//            cout << "\n==== SATELLITE QUEUE MANAGEMENT SYSTEM ====" << endl;
//            cout << "1. Add Satellite" << endl;
//            cout << "2. Serve Next Satellite" << endl;
//            cout << "3. Display Queue Status" << endl;
//            cout << "4. Run Simulation" << endl;
//            cout << "5. Generate Report" << endl;
//            cout << "6. Admin Console" << endl;
//            cout << "7. Exit" << endl;
//            cout << "Enter choice: ";
//            cin >> choice;
//            
//            if (choice == 1) {
//                string id, service_type;
//                double urgency;
//                int type_choice;
//                cout << "Enter satellite ID: ";
//                cin >> id;
//                cout << "Enter service type (emergency/vip/regular/priority): ";
//                cin >> service_type;
//                cout << "Enter urgency (1-5): ";
//                cin >> urgency;
//                cout << "Enter service type enum (0=REGULAR, 1=VIP, 2=EMERGENCY, 3=PRIORITY): ";
//                cin >> type_choice;
//                ServiceType type = static_cast<ServiceType>(type_choice);
//                addSatellite(id, service_type, urgency, type);
//            }
//            else if (choice == 2) {
//                serveNextSatellite();
//            }
//            else if (choice == 3) {
//                displayQueueStatus();
//            }
//            else if (choice == 4) {
//                runSimulation();
//            }
//            else if (choice == 5) {
//                string filter, sort;
//                cout << "Enter filter type (or press Enter for all): ";
//                cin.ignore();
//                getline(cin, filter);
//                cout << "Enter sort by (wait_time/priority/service_time): ";
//                cin >> sort;
//                generateReport(filter, sort);
//            }
//            else if (choice == 6) {
//                admin->runAdvancedAdmin();
//            }
//            else if (choice == 7) {
//                cout << "Exiting system..." << endl;
//                break;
//            }
//            else {
//                cout << "Invalid choice. Try again." << endl;
//            }
//        }
//    }
//};
//
//// Main entry point
//int main() {
//    AdminConsole admin;
//    SmartQueueManagementSystem manager(&admin);
//    manager.runManager();
//    return 0;
//}
