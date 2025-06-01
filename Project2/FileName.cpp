#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

const int MAX_TICKETS = 100;
const int MAX_CANCELLED = 100;
const int TABLE_SIZE = 101; 


struct Ticket {
    int ticketID;
    string passengerName;
    int age;
    int busID;
};


struct Bus {
    int busID;
    string busName;
    string source;
    string destination;
    int totalSeats;
    int bookedSeats;
    Bus* next;
};
Bus* busHead = nullptr;


Bus* hashTable[TABLE_SIZE] = { nullptr };

int hashFunction(int key) {
    return key % TABLE_SIZE;
}

void insertBusHash(Bus* bus) {
    int idx = hashFunction(bus->busID);
    int start = idx;
    while (hashTable[idx] != nullptr) {
        idx = (idx + 1) % TABLE_SIZE;
        if (idx == start) {
            cout << "Hash table full! Cannot insert bus." << endl;
            return;
        }
    }
    hashTable[idx] = bus;
}

Bus* searchBusHash(int busID) {
    int idx = hashFunction(busID);
    int start = idx;
    while (hashTable[idx] != nullptr) {
        if (hashTable[idx]->busID == busID)
            return hashTable[idx];
        idx = (idx + 1) % TABLE_SIZE;
        if (idx == start) break;
    }
    return nullptr;
}

void saveBusesToFile() {
    ofstream out("buses.txt");
    for (Bus* cur = busHead; cur; cur = cur->next) {
        out << cur->busID << ','
            << cur->busName << ','
            << cur->source << ','
            << cur->destination << ','
            << cur->totalSeats << ','
            << cur->bookedSeats << '\n';
    }
}

void loadBusesFromFile() {
    ifstream in("buses.txt");
    if (!in) return;
    string line;
    while (getline(in, line)) {
        stringstream ss(line);
        Bus* b = new Bus;
        getline(ss, line, ','); b->busID = stoi(line);
        getline(ss, b->busName, ',');
        getline(ss, b->source, ',');
        getline(ss, b->destination, ',');
        getline(ss, line, ','); b->totalSeats = stoi(line);
        getline(ss, line); b->bookedSeats = stoi(line);
        b->next = nullptr;
        
        if (!busHead) busHead = b;
        else {
            Bus* t = busHead;
            while (t->next) t = t->next;
            t->next = b;
        }
        
        insertBusHash(b);
    }
}


Ticket ticketQueue[MAX_TICKETS];
int qFront = 0, qRear = 0;
bool isQueueEmpty() { return qFront == qRear; }
bool isQueueFull() { return (qRear + 1) % MAX_TICKETS == qFront; }

void enqueueTicket(const Ticket& t) {
    if (isQueueFull()) {
        cout << "Ticket queue is full!" << endl;
        return;
    }
    ticketQueue[qRear] = t;
    qRear = (qRear + 1) % MAX_TICKETS;
}


Ticket cancelStack[MAX_CANCELLED];
int cancelTop = 0;

bool isStackEmpty() { return cancelTop == 0; }
bool isStackFull() { return cancelTop == MAX_CANCELLED; }

void pushCancelled(const Ticket& t) {
    if (isStackFull()) {
        cout << "Cancellation history full!" << endl;
        return;
    }
    cancelStack[cancelTop++] = t;
}

void viewCancelledHistory() {
    if (isStackEmpty()) {
        cout << "No cancelled tickets yet." << endl;
        return;
    }
    cout << "--- Cancelled Tickets ---" << endl;
    for (int i = cancelTop - 1; i >= 0; --i) {
        cout << "TicketID:" << cancelStack[i].ticketID
            << " Name:" << cancelStack[i].passengerName
            << " BusID:" << cancelStack[i].busID << endl;
    }
}

void addBus() {
    Bus* b = new Bus;
    cout << "Enter Bus ID: "; cin >> b->busID;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter Bus Name: "; getline(cin, b->busName);
    cout << "Enter Source: "; getline(cin, b->source);
    cout << "Enter Destination: "; getline(cin, b->destination);
    cout << "Enter Total Seats: "; cin >> b->totalSeats;
    b->bookedSeats = 0;
    b->next = nullptr;
    
    if (!busHead) busHead = b;
    else {
        Bus* t = busHead;
        while (t->next) t = t->next;
        t->next = b;
    }
   
    insertBusHash(b);
    saveBusesToFile();
    cout << "Bus added successfully." << endl;
}

void viewAllBuses() {
    if (!busHead) {
        cout << "No buses available." << endl;
        return;
    }
    cout << "--- Available Buses ---" << endl;
    for (Bus* t = busHead; t; t = t->next) {
        cout << "ID:" << t->busID
            << " " << t->busName
            << " [" << t->source << "->" << t->destination << "]"
            << " Seats:" << t->bookedSeats << "/" << t->totalSeats
            << endl;
    }
}

void viewAllTickets() {
    if (isQueueEmpty()) {
        cout << "No tickets booked yet." << endl;
        return;
    }
    cout << "--- Booked Tickets ---" << endl;
    int idx = qFront;
    while (idx != qRear) {
        auto& t = ticketQueue[idx];
        cout << "TicketID:" << t.ticketID
            << " Name:" << t.passengerName
            << " Age:" << t.age
            << " BusID:" << t.busID << endl;
        idx = (idx + 1) % MAX_TICKETS;
    }
}

void bookTicket() {
    Ticket t;
    cout << "Enter Ticket ID: "; cin >> t.ticketID;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter Name: "; getline(cin, t.passengerName);
    cout << "Enter Age: "; cin >> t.age;
    cout << "Enter Bus ID: "; cin >> t.busID;
    Bus* b = searchBusHash(t.busID);
    if (!b) {
        cout << "Bus not found." << endl;
        return;
    }
    if (b->bookedSeats < b->totalSeats) {
        enqueueTicket(t);
        b->bookedSeats++;
        saveBusesToFile();
        cout << "Ticket booked successfully." << endl;
    }
    else {
        cout << "Bus is full." << endl;
    }
}

void cancelTicketByID() {
    if (isQueueEmpty()) {
        cout << "No tickets to cancel." << endl;
        return;
    }
    int id; cout << "Enter Ticket ID to cancel: "; cin >> id;
    Ticket tmp[MAX_TICKETS];
    int nf = 0, nr = 0;
    bool found = false;
    int idx = qFront;
    while (idx != qRear) {
        auto& t = ticketQueue[idx];
        if (!found && t.ticketID == id) {
            Bus* b = searchBusHash(t.busID);
            if (b) { b->bookedSeats--; saveBusesToFile(); }
            pushCancelled(t);
            found = true;
        }
        else {
            tmp[nr++] = t;
        }
        idx = (idx + 1) % MAX_TICKETS;
    }
    if (!found) {
        cout << "Ticket ID not found." << endl;
        return;
    }
    qFront = 0; qRear = nr;
    for (int i = 0; i < nr; ++i) ticketQueue[i] = tmp[i];
    cout << "Ticket cancelled successfully." << endl;
}

void searchBusByID() {
    int id; cout << "Enter Bus ID to search: "; cin >> id;
    Bus* b = searchBusHash(id);
    if (b) {
        cout << "Found Bus -> ID:" << b->busID
            << " Name:" << b->busName
            << " From:" << b->source
            << " To:" << b->destination
            << " Seats:" << b->bookedSeats << "/" << b->totalSeats
            << endl;
    }
    else {
        cout << "Bus with ID " << id << " not found." << endl;
    }
}

bool adminLogin() {
    string u, p;
    cout << "Username: "; cin >> u;
    cout << "Password: "; cin >> p;
    return (u == "admin" && p == "admin123");
}

void adminMenu() {
    int c;
    do {
        cout << "\n--- Admin Panel ---\n"
            << "1. Add Bus\n"
            << "2. View Buses\n"
            << "3. View Tickets\n"
            << "4. View Cancellations\n"
            << "5. Cancel Ticket\n"
            << "6. Search Bus\n"
            << "7. Back\nChoice: ";
        cin >> c;
        switch (c) {
        case 1: addBus(); break;
        case 2: viewAllBuses(); break;
        case 3: viewAllTickets(); break;
        case 4: viewCancelledHistory(); break;
        case 5: cancelTicketByID(); break;
        case 6: searchBusByID(); break;
        case 7: return;
        default: cout << "Invalid choice." << endl;
        }
    } while (true);
}

void passengerMenu() {
    int c;
    do {
        cout << "\n--- Passenger ---\n"
            << "1. View Buses\n"
            << "2. Book Ticket\n"
            << "3. Cancel Ticket\n"
            << "4. Back\nChoice: ";
        cin >> c;
        switch (c) {
        case 1: viewAllBuses(); break;
        case 2: bookTicket(); break;
        case 3: cancelTicketByID(); break;
        case 4: return;
        default: cout << "Invalid choice." << endl;
        }
    } while (true);
}

int main() {
    loadBusesFromFile();
    int choice;
    do {
        cout << "\n====== Bus Reservation System ======\n"
            << "1. Admin Login\n"
            << "2. Passenger Menu\n"
            << "3. Exit\nChoice: ";
        cin >> choice;
        switch (choice) {
        case 1:
            if (adminLogin()) adminMenu();
            else cout << "Invalid credentials." << endl;
            break;
        case 2: passengerMenu(); break;
        case 3: cout << "Goodbye!" << endl; break;
        default: cout << "Invalid choice." << endl;
        }
    } while (choice != 3);
    return 0;
}
