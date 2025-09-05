#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <map>
#include <cstdio>
#include "admin_console.h"
#include "constants.h"
#include "hash_table.h"
#include "satellite.h"
#include "satellite_queue.h"

using namespace std;

#pragma comment(lib, "comctl32.lib")

// Global variables
HWND hMainWindow, hLoginWindow;
HWND hUsernameEdit, hPasswordEdit, hLoginButton, hExitButton;
HWND hAddButton, hServeButton, hStatusButton;
HWND hIdEdit, hTypeEdit, hUrgencyEdit, hTypeEnumEdit;
HWND hEmergencyList, hVipList, hRegularList, hPriorityList;
HWND hEmergencyLabel, hVipLabel, hRegularLabel, hPriorityLabel;
HWND hEmergencyCount, hVipCount, hRegularCount, hPriorityCount;
HWND hOutputText, hClearButton;

// System instances
class SmartQueueManagementSystem* queueSystem = nullptr;
AdminConsole* admin = nullptr;
bool isLoggedIn = false;
bool isAdmin = false;

// Window procedures
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LoginWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// GUI helper functions
void CreateLoginControls(HWND hwnd);
void CreateMainControls(HWND hwnd);
void UpdateQueueDisplay();
void AddToOutput(const string& text);
void ClearOutput();
void ShowLoginWindow();
void ShowMainWindow();
void UpdateQueueLists();

// Smart Queue Management System class
class SmartQueueManagementSystem {
private:
    HashTable<Satellite> satellite_table;
    SatelliteQueue emergency_queue;
    SatelliteQueue vip_queue;
    SatelliteQueue regular_queue;
    SatelliteQueue priority_queue;
    Satellite* history_log;
    int history_count;
    AdminConsole* admin;

    int total_communications;
    int successful_communications;

    double getCurrentTime() {
        return static_cast<double>(time(0));
    }

public:
    SmartQueueManagementSystem(AdminConsole* admin_console) {
        admin = admin_console;
        history_count = 0;
        total_communications = 0;
        successful_communications = 0;
        history_log = new Satellite[MAX_HISTORY];
        
        emergency_queue = SatelliteQueue("Emergency");
        vip_queue = SatelliteQueue("VIP");
        regular_queue = SatelliteQueue("Regular");
        priority_queue = SatelliteQueue("Priority");
    }

    ~SmartQueueManagementSystem() {
        delete[] history_log;
    }

    double calculatePriority(Satellite& sat) {
        double priority = sat.urgency * admin->getUrgencyWeight() +
            sat.wait_time * admin->getWaitTimeWeight() +
            admin->getTypeScore(sat.service_type) * admin->getTypeWeight();
        
        double fairness_boost = admin->calculateFairnessBoost(sat.wait_time);
        priority += fairness_boost;
        return priority;
    }

    void addSatellite(string id, string service_type, double urgency, ServiceType type) {
        Satellite sat(id, service_type, urgency, type);
        sat.wait_time = 0.0;
        sat.priority_score = calculatePriority(sat);

        if (sat.communication_quality > 0.8) {
            sat.predicted_success_rate = 0.9;
        } else if (sat.communication_quality > 0.6) {
            sat.predicted_success_rate = 0.7;
        } else {
            sat.predicted_success_rate = 0.5;
        }

        satellite_table.insert(id, sat);

        if (service_type == "emergency") {
            emergency_queue.add(sat);
        }
        else if (service_type == "vip") {
            vip_queue.add(sat);
        }
        else if (service_type == "priority") {
            priority_queue.add(sat);
        }
        else {
            regular_queue.add(sat);
        }

        stringstream ss;
        ss << "[SUCCESS] Satellite " << id << " added successfully!\n";
        ss << "   Priority Score: " << fixed << setprecision(1) << sat.priority_score << "\n";
        ss << "   Service Type: " << service_type << "\n";
        ss << "   Success Probability: " << setprecision(0) << sat.predicted_success_rate * 100 << "%\n";
        ss << "   Service Quality: " << setprecision(2) << sat.communication_quality << "\n\n";
        AddToOutput(ss.str());
        UpdateQueueLists();
        UpdateQueueDisplay();
    }

    Satellite* getNextSatellite() {
        Satellite* highest_priority_sat = nullptr;
        double highest_priority = -1.0;
        
        if (!emergency_queue.isEmpty()) {
            Satellite* sat = emergency_queue.getHighestPriority();
            if (sat && sat->priority_score > highest_priority) {
                highest_priority = sat->priority_score;
                highest_priority_sat = sat;
            }
        }
        
        if (!vip_queue.isEmpty()) {
            Satellite* sat = vip_queue.getHighestPriority();
            if (sat && sat->priority_score > highest_priority) {
                highest_priority = sat->priority_score;
                highest_priority_sat = sat;
            }
        }
        
        if (!priority_queue.isEmpty()) {
            Satellite* sat = priority_queue.getHighestPriority();
            if (sat && sat->priority_score > highest_priority) {
                highest_priority = sat->priority_score;
                highest_priority_sat = sat;
            }
        }
        
        if (!regular_queue.isEmpty()) {
            Satellite* sat = regular_queue.getHighestPriority();
            if (sat && sat->priority_score > highest_priority) {
                highest_priority = sat->priority_score;
                highest_priority_sat = sat;
            }
        }
        
        return highest_priority_sat;
    }

    void serveNextSatellite() {
        Satellite* next_sat = getNextSatellite();
        if (next_sat) {
            stringstream ss;
            ss << "[PROCESSING] Satellite " << next_sat->id << "...\n";
            AddToOutput(ss.str());
            
            next_sat->status = SERVED;
            total_communications++;
            
            if (next_sat->predicted_success_rate > 0.5) {
                successful_communications++;
                AddToOutput("[SUCCESS] Successfully served!\n\n");
            }
            else {
                AddToOutput("[ERROR] Low success rate. Marked as failed.\n\n");
            }
            
            if (history_count < MAX_HISTORY) {
                history_log[history_count++] = *next_sat;
            }
            
            emergency_queue.removeById(next_sat->id);
            vip_queue.removeById(next_sat->id);
            priority_queue.removeById(next_sat->id);
            regular_queue.removeById(next_sat->id);
            
            UpdateQueueLists();
            UpdateQueueDisplay();
        }
        else {
            AddToOutput("[INFO] No satellites in queue to serve.\n\n");
        }
    }

    void displayQueueStatus() {
        stringstream ss;
        ss << "=== QUEUE STATUS ===\n";
        ss << "Emergency Queue: " << emergency_queue.size << " satellites\n";
        ss << "VIP Queue: " << vip_queue.size << " satellites\n";
        ss << "Priority Queue: " << priority_queue.size << " satellites\n";
        ss << "Regular Queue: " << regular_queue.size << " satellites\n";
        ss << "Total in system: " << (emergency_queue.size + vip_queue.size + priority_queue.size + regular_queue.size) << " satellites\n\n";
        AddToOutput(ss.str());
    }

    void runSimulation() {
        int duration = admin->getSimulationDuration();
        double arrival_rate = admin->getArrivalRate();
        int counters = admin->getCounters();
        
        stringstream ss;
        ss << "=== SIMULATION MODE ===\n";
        ss << "Duration: " << duration << " minutes\n";
        ss << "Arrival Rate: " << arrival_rate << " satellites per minute\n";
        ss << "Counters: " << counters << "\n\n";
        AddToOutput(ss.str());

        srand(static_cast<unsigned int>(time(0)));

        for (int minute = 1; minute <= duration; minute++) {
            double prob = ((double)rand() / RAND_MAX);
            if (prob < arrival_rate) {
                string types[] = { "emergency", "vip", "regular", "priority" };
                string type = types[rand() % 4];
                string id = "SIM-" + to_string(rand() % 1000 + minute);
                double urgency = rand() % 5 + 1;
                ServiceType service_type = static_cast<ServiceType>(rand() % 4);
                addSatellite(id, type, urgency, service_type);
            }

            for (int counter = 0; counter < counters; counter++) {
                Satellite* sat = getNextSatellite();
                if (sat) {
                    serveNextSatellite();
                }
            }

            if (minute % 10 == 0) {
                ss.str("");
                ss << "Time: " << minute << " minutes\n";
                ss << "Emergency Queue: " << emergency_queue.size << " satellites\n";
                ss << "VIP Queue: " << vip_queue.size << " satellites\n";
                ss << "Priority Queue: " << priority_queue.size << " satellites\n";
                ss << "Regular Queue: " << regular_queue.size << " satellites\n\n";
                AddToOutput(ss.str());
            }
        }

        ss.str("");
        ss << "Simulation complete!\n";
        ss << "Total communications: " << total_communications << "\n";
        ss << "Successful communications: " << successful_communications << "\n\n";
        AddToOutput(ss.str());
    }

    void generateReport() {
        stringstream ss;
        ss << "=== ADVANCED REPORT ===\n";
        ss << "ID | Type | Wait Time | Service Time | Priority\n";
        ss << "------------------------------------------------\n";
        
        for (int i = 0; i < history_count; i++) {
            ss << history_log[i].id << " | " 
               << history_log[i].service_type << " | "
               << setprecision(1) << history_log[i].wait_time << " min | "
               << history_log[i].service_time << " min | "
               << setprecision(1) << history_log[i].priority_score << "\n";
        }
        ss << "\n";
        AddToOutput(ss.str());
    }

    // Getter methods for queue access
    int getEmergencyCount() { return emergency_queue.size; }
    int getVipCount() { return vip_queue.size; }
    int getRegularCount() { return regular_queue.size; }
    int getPriorityCount() { return priority_queue.size; }
};

// GUI Helper Functions
void CreateLoginControls(HWND hwnd) {
    // Create login form
    CreateWindowW(L"STATIC", L"Satellite Queue Management System", 
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        50, 30, 300, 30, hwnd, NULL, NULL, NULL);

    CreateWindowW(L"STATIC", L"Username:", WS_VISIBLE | WS_CHILD,
        50, 80, 80, 20, hwnd, NULL, NULL, NULL);
    hUsernameEdit = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
        140, 80, 200, 25, hwnd, NULL, NULL, NULL);

    CreateWindowW(L"STATIC", L"Password:", WS_VISIBLE | WS_CHILD,
        50, 120, 80, 20, hwnd, NULL, NULL, NULL);
    hPasswordEdit = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD,
        140, 120, 200, 25, hwnd, NULL, NULL, NULL);

    hLoginButton = CreateWindowW(L"BUTTON", L"Sign in", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        140, 160, 90, 30, hwnd, (HMENU)1, NULL, NULL);
    
    hExitButton = CreateWindowW(L"BUTTON", L"Exit", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        250, 160, 90, 30, hwnd, (HMENU)2, NULL, NULL);
}

void CreateMainControls(HWND hwnd) {
    // Create main interface
    hAddButton = CreateWindowW(L"BUTTON", L"Add Satellite", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        10, 10, 120, 30, hwnd, (HMENU)1, NULL, NULL);
    
    hServeButton = CreateWindowW(L"BUTTON", L"Serve Next", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        140, 10, 120, 30, hwnd, (HMENU)2, NULL, NULL);
    
    hStatusButton = CreateWindowW(L"BUTTON", L"Queue Status", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        270, 10, 120, 30, hwnd, (HMENU)3, NULL, NULL);

    // Input fields
    CreateWindowW(L"STATIC", L"Satellite ID:", WS_VISIBLE | WS_CHILD,
        10, 50, 80, 20, hwnd, NULL, NULL, NULL);
    hIdEdit = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
        100, 50, 100, 20, hwnd, NULL, NULL, NULL);

    CreateWindowW(L"STATIC", L"Service Type:", WS_VISIBLE | WS_CHILD,
        210, 50, 80, 20, hwnd, NULL, NULL, NULL);
    hTypeEdit = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
        300, 50, 100, 20, hwnd, NULL, NULL, NULL);

    CreateWindowW(L"STATIC", L"Urgency (1-5):", WS_VISIBLE | WS_CHILD,
        410, 50, 80, 20, hwnd, NULL, NULL, NULL);
    hUrgencyEdit = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
        500, 50, 100, 20, hwnd, NULL, NULL, NULL);

    CreateWindowW(L"STATIC", L"Type Enum (0-3):", WS_VISIBLE | WS_CHILD,
        610, 50, 80, 20, hwnd, NULL, NULL, NULL);
    hTypeEnumEdit = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
        700, 50, 100, 20, hwnd, NULL, NULL, NULL);

    // Queue status display
    hEmergencyLabel = CreateWindowW(L"STATIC", L"Emergency Queue:", WS_VISIBLE | WS_CHILD,
        10, 80, 100, 20, hwnd, NULL, NULL, NULL);
    hEmergencyCount = CreateWindowW(L"STATIC", L"0", WS_VISIBLE | WS_CHILD,
        120, 80, 50, 20, hwnd, NULL, NULL, NULL);

    hVipLabel = CreateWindowW(L"STATIC", L"VIP Queue:", WS_VISIBLE | WS_CHILD,
        180, 80, 80, 20, hwnd, NULL, NULL, NULL);
    hVipCount = CreateWindowW(L"STATIC", L"0", WS_VISIBLE | WS_CHILD,
        270, 80, 50, 20, hwnd, NULL, NULL, NULL);

    hRegularLabel = CreateWindowW(L"STATIC", L"Regular Queue:", WS_VISIBLE | WS_CHILD,
        330, 80, 90, 20, hwnd, NULL, NULL, NULL);
    hRegularCount = CreateWindowW(L"STATIC", L"0", WS_VISIBLE | WS_CHILD,
        430, 80, 50, 20, hwnd, NULL, NULL, NULL);

    hPriorityLabel = CreateWindowW(L"STATIC", L"Priority Queue:", WS_VISIBLE | WS_CHILD,
        490, 80, 90, 20, hwnd, NULL, NULL, NULL);
    hPriorityCount = CreateWindowW(L"STATIC", L"0", WS_VISIBLE | WS_CHILD,
        590, 80, 50, 20, hwnd, NULL, NULL, NULL);

    // Queue lists (visual display)
    hEmergencyList = CreateWindowW(L"LISTBOX", L"", 
        WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY | WS_VSCROLL,
        10, 110, 180, 150, hwnd, NULL, NULL, NULL);

    hVipList = CreateWindowW(L"LISTBOX", L"", 
        WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY | WS_VSCROLL,
        200, 110, 180, 150, hwnd, NULL, NULL, NULL);

    hRegularList = CreateWindowW(L"LISTBOX", L"", 
        WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY | WS_VSCROLL,
        390, 110, 180, 150, hwnd, NULL, NULL, NULL);

    hPriorityList = CreateWindowW(L"LISTBOX", L"", 
        WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY | WS_VSCROLL,
        580, 110, 180, 150, hwnd, NULL, NULL, NULL);

    // Output area
    hOutputText = CreateWindowW(L"EDIT", L"", 
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
        10, 270, 750, 150, hwnd, NULL, NULL, NULL);

    hClearButton = CreateWindowW(L"BUTTON", L"Clear Output", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        680, 430, 80, 25, hwnd, (HMENU)4, NULL, NULL);
}

void UpdateQueueDisplay() {
    if (!queueSystem) return;
    
    wchar_t count[10];
    swprintf_s(count, L"%d", queueSystem->getEmergencyCount());
    SetWindowTextW(hEmergencyCount, count);
    
    swprintf_s(count, L"%d", queueSystem->getVipCount());
    SetWindowTextW(hVipCount, count);
    
    swprintf_s(count, L"%d", queueSystem->getRegularCount());
    SetWindowTextW(hRegularCount, count);
    
    swprintf_s(count, L"%d", queueSystem->getPriorityCount());
    SetWindowTextW(hPriorityCount, count);
}

void UpdateQueueLists() {
    if (!queueSystem) return;
    
    // Clear all lists
    SendMessageW(hEmergencyList, LB_RESETCONTENT, 0, 0);
    SendMessageW(hVipList, LB_RESETCONTENT, 0, 0);
    SendMessageW(hRegularList, LB_RESETCONTENT, 0, 0);
    SendMessageW(hPriorityList, LB_RESETCONTENT, 0, 0);
    
    // Add satellites to appropriate lists (simplified - you'd need to add getter methods)
    // This is a placeholder - in a real implementation you'd iterate through each queue
    // and add each satellite's ID to the appropriate listbox
}

void AddToOutput(const string& text) {
    int length = GetWindowTextLengthW(hOutputText);
    SendMessageW(hOutputText, EM_SETSEL, length, length);
    
    // Convert string to wide string
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
    wchar_t* wide_text = new wchar_t[size_needed];
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide_text, size_needed);
    
    SendMessageW(hOutputText, EM_REPLACESEL, FALSE, (LPARAM)wide_text);
    SendMessageW(hOutputText, EM_SCROLLCARET, 0, 0);
    
    delete[] wide_text;
}

void ClearOutput() {
    SetWindowTextW(hOutputText, L"");
}

void ShowLoginWindow() {
    ShowWindow(hLoginWindow, SW_SHOW);
    ShowWindow(hMainWindow, SW_HIDE);
}

void ShowMainWindow() {
    ShowWindow(hLoginWindow, SW_HIDE);
    ShowWindow(hMainWindow, SW_SHOW);
}

// Login Window Procedure
LRESULT CALLBACK LoginWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        CreateLoginControls(hwnd);
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1: // Login button
            {
                wchar_t username[256], password[256];
                GetWindowTextW(hUsernameEdit, username, 256);
                GetWindowTextW(hPasswordEdit, password, 256);
                
                if (wcscmp(username, L"admin") == 0 && wcscmp(password, L"admin123") == 0) {
                    isLoggedIn = true;
                    isAdmin = true;
                    ShowMainWindow();
                    AddToOutput("[SUCCESS] Admin login successful!\n\n");
                }
                else if (wcscmp(username, L"user") == 0 && wcscmp(password, L"user123") == 0) {
                    isLoggedIn = true;
                    isAdmin = false;
                    ShowMainWindow();
                    AddToOutput("[SUCCESS] User login successful!\n\n");
                }
                else {
                    MessageBoxW(hwnd, L"Invalid username or password!", L"Login Error", MB_OK | MB_ICONERROR);
                }
                
                SetWindowTextW(hUsernameEdit, L"");
                SetWindowTextW(hPasswordEdit, L"");
            }
            break;

        case 2: // Exit button
            PostQuitMessage(0);
            break;
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

// Main Window Procedure
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        CreateMainControls(hwnd);
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1: // Add Satellite
            {
                wchar_t id[256], type[256], urgency[256], typeEnum[256];
                GetWindowTextW(hIdEdit, id, 256);
                GetWindowTextW(hTypeEdit, type, 256);
                GetWindowTextW(hUrgencyEdit, urgency, 256);
                GetWindowTextW(hTypeEnumEdit, typeEnum, 256);
                
                if (wcslen(id) > 0 && wcslen(type) > 0 && wcslen(urgency) > 0 && wcslen(typeEnum) > 0) {
                    // Convert wide strings to regular strings
                    char id_char[256], type_char[256], urgency_char[256], typeEnum_char[256];
                    wcstombs_s(nullptr, id_char, 256, id, 256);
                    wcstombs_s(nullptr, type_char, 256, type, 256);
                    wcstombs_s(nullptr, urgency_char, 256, urgency, 256);
                    wcstombs_s(nullptr, typeEnum_char, 256, typeEnum, 256);
                    
                    queueSystem->addSatellite(string(id_char), string(type_char), atof(urgency_char), static_cast<ServiceType>(atoi(typeEnum_char)));
                    SetWindowTextW(hIdEdit, L"");
                    SetWindowTextW(hTypeEdit, L"");
                    SetWindowTextW(hUrgencyEdit, L"");
                    SetWindowTextW(hTypeEnumEdit, L"");
                } else {
                    AddToOutput("[ERROR] Please fill all fields!\n\n");
                }
            }
            break;

        case 2: // Serve Next
            queueSystem->serveNextSatellite();
            break;

        case 3: // Queue Status
            queueSystem->displayQueueStatus();
            break;

        case 4: // Clear Output
            ClearOutput();
            break;
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize common controls
    InitCommonControls();
    
    // Initialize system
    admin = new AdminConsole();
    queueSystem = new SmartQueueManagementSystem(admin);

    // Register window classes
    WNDCLASSW wcLogin = {};
    wcLogin.lpfnWndProc = LoginWindowProc;
    wcLogin.hInstance = hInstance;
    wcLogin.lpszClassName = L"SatelliteQueueLogin";
    wcLogin.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcLogin.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wcLogin);

    WNDCLASSW wcMain = {};
    wcMain.lpfnWndProc = MainWindowProc;
    wcMain.hInstance = hInstance;
    wcMain.lpszClassName = L"SatelliteQueueMain";
    wcMain.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcMain.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wcMain);

    // Create windows
    hLoginWindow = CreateWindowExW(
        0,
        L"SatelliteQueueLogin",
        L"Satellite Queue Management System - Login",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 250,
        NULL, NULL, hInstance, NULL
    );

    hMainWindow = CreateWindowExW(
        0,
        L"SatelliteQueueMain",
        L"Satellite Queue Management System - Professional GUI",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 500,
        NULL, NULL, hInstance, NULL
    );

    if (hLoginWindow == NULL || hMainWindow == NULL) {
        return 0;
    }

    ShowLoginWindow();

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    delete queueSystem;
    delete admin;

    return 0;
} 