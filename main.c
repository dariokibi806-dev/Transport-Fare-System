/*
 * =====================================================================
 *   TRANSPORT FARE MANAGEMENT SYSTEM
 *   Group 6 - C Programming Assignment
 *   Manages passengers in a taxi/bus with CRUD, fares & summaries
 * =====================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ======================== CONSTANTS ======================== */
#define MAX_PASSENGERS   50
#define MAX_NAME         50
#define MAX_USERNAME     30
#define MAX_PASSWORD     30
#define MAX_ACCOUNTS     10
#define VEHICLE_CAPACITY 20
#define READY_THRESHOLD  70.0   /* vehicle needs 70% full to depart */
#define MAX_LUGGAGE_KG   20.0   /* free luggage limit in kg         */
#define EXTRA_LUGGAGE_CHARGE 5000.0  /* UGX per extra kg             */

/* Fare rates (UGX per km) */
#define RATE_PER_KM      500.0
#define DISCOUNT_STUDENT 0.20   /* 20% off */
#define DISCOUNT_CHILD   0.30   /* 30% off */
#define DISCOUNT_ELDERLY 0.15   /* 15% off */

/* ======================== STRUCTURES ======================== */

/* Passenger type options */
typedef enum {
    ADULT   = 1,
    STUDENT = 2,
    CHILD   = 3,
    ELDERLY = 4
} PassengerType;

/* A single passenger record */
typedef struct {
    int    id;
    char   name[MAX_NAME];
    int    age;
    PassengerType type;
    double distanceKm;
    double luggageKg;
    double fare;          /* calculated fare */
    int    active;        /* 1 = on board, 0 = deleted */
} Passenger;

/* A user account */
typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    int  isLoggedIn;
} Account;

/* ======================== GLOBAL DATA ======================== */
Passenger passengers[MAX_PASSENGERS];
int       passengerCount = 0;
int       nextID         = 1;

Account accounts[MAX_ACCOUNTS];
int     accountCount    = 0;
int     loggedInIndex   = -1;   /* index of current user, -1 = none */

/* ======================== UTILITY FUNCTIONS ======================== */

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pauseScreen() {
    printf("\n  Press ENTER to continue...");
    while (getchar() != '\n');   /* flush buffer then wait */
    getchar();
}

void printLine(char ch, int len) {
    for (int i = 0; i < len; i++) putchar(ch);
    putchar('\n');
}

void printHeader(const char *title) {
    clearScreen();
    printLine('=', 60);
    printf("   TRANSPORT FARE MANAGEMENT SYSTEM\n");
    printLine('-', 60);
    printf("   %s\n", title);
    printLine('=', 60);
    printf("\n");
}

const char* typeName(PassengerType t) {
    switch (t) {
        case STUDENT: return "Student";
        case CHILD:   return "Child";
        case ELDERLY: return "Elderly";
        default:      return "Adult";
    }
}

/* ======================== FARE CALCULATION ======================== */

double calculateFare(double distanceKm, PassengerType type, double luggageKg) {
    double baseFare = distanceKm * RATE_PER_KM;

    /* Apply discount by passenger type */
    switch (type) {
        case STUDENT: baseFare *= (1.0 - DISCOUNT_STUDENT); break;
        case CHILD:   baseFare *= (1.0 - DISCOUNT_CHILD);   break;
        case ELDERLY: baseFare *= (1.0 - DISCOUNT_ELDERLY); break;
        default: break;   /* Adult - no discount */
    }

    /* Extra luggage charge */
    if (luggageKg > MAX_LUGGAGE_KG) {
        double extraKg = luggageKg - MAX_LUGGAGE_KG;
        baseFare += extraKg * EXTRA_LUGGAGE_CHARGE;
        printf("  [!] Extra luggage charge applied for %.1f kg over limit.\n",
               extraKg);
    }

    return baseFare;
}

/* ======================== ACCOUNT FUNCTIONS ======================== */

void createAccount() {
    printHeader("CREATE NEW ACCOUNT");

    if (accountCount >= MAX_ACCOUNTS) {
        printf("  [!] Maximum accounts reached. Cannot create more.\n");
        pauseScreen();
        return;
    }

    Account newAcc;
    printf("  Enter username : ");
    scanf("%s", newAcc.username);

    /* Check if username already exists */
    for (int i = 0; i < accountCount; i++) {
        if (strcmp(accounts[i].username, newAcc.username) == 0) {
            printf("  [!] Username already exists. Try a different one.\n");
            pauseScreen();
            return;
        }
    }

    printf("  Enter password : ");
    scanf("%s", newAcc.password);
    newAcc.isLoggedIn = 0;

    accounts[accountCount++] = newAcc;
    printf("\n  [OK] Account created successfully! You can now log in.\n");
    pauseScreen();
}

void login() {
    printHeader("LOGIN");

    if (loggedInIndex != -1) {
        printf("  [!] Already logged in as: %s\n",
               accounts[loggedInIndex].username);
        pauseScreen();
        return;
    }

    char uname[MAX_USERNAME], pass[MAX_PASSWORD];
    printf("  Username : ");
    scanf("%s", uname);
    printf("  Password : ");
    scanf("%s", pass);

    for (int i = 0; i < accountCount; i++) {
        if (strcmp(accounts[i].username, uname) == 0 &&
            strcmp(accounts[i].password, pass)  == 0) {
            loggedInIndex = i;
            accounts[i].isLoggedIn = 1;
            printf("\n  [OK] Welcome, %s! Login successful.\n", uname);
            pauseScreen();
            return;
        }
    }

    printf("\n  [!] Invalid username or password.\n");
    pauseScreen();
}

void logout() {
    if (loggedInIndex == -1) {
        printf("  [!] No user is currently logged in.\n");
    } else {
        printf("  [OK] %s has been logged out.\n",
               accounts[loggedInIndex].username);
        accounts[loggedInIndex].isLoggedIn = 0;
        loggedInIndex = -1;
    }
    pauseScreen();
}

void changePassword() {
    printHeader("CHANGE PASSWORD");

    if (loggedInIndex == -1) {
        printf("  [!] Please log in first.\n");
        pauseScreen();
        return;
    }

    char oldPass[MAX_PASSWORD], newPass[MAX_PASSWORD];
    printf("  Current password : ");
    scanf("%s", oldPass);

    if (strcmp(accounts[loggedInIndex].password, oldPass) != 0) {
        printf("  [!] Incorrect current password.\n");
        pauseScreen();
        return;
    }

    printf("  New password     : ");
    scanf("%s", newPass);
    strcpy(accounts[loggedInIndex].password, newPass);
    printf("\n  [OK] Password changed successfully.\n");
    pauseScreen();
}

void viewAllAccounts() {
    printHeader("ALL REGISTERED ACCOUNTS");

    if (accountCount == 0) {
        printf("  No accounts registered yet.\n");
    } else {
        printf("  %-5s %-20s %-10s\n", "No.", "Username", "Status");
        printLine('-', 40);
        for (int i = 0; i < accountCount; i++) {
            printf("  %-5d %-20s %-10s\n",
                   i + 1,
                   accounts[i].username,
                   (i == loggedInIndex) ? "LOGGED IN" : "Offline");
        }
    }
    pauseScreen();
}

/* ======================== PASSENGER FUNCTIONS ======================== */

/* Count currently active (on-board) passengers */
int activeCount() {
    int count = 0;
    for (int i = 0; i < passengerCount; i++)
        if (passengers[i].active) count++;
    return count;
}

void registerPassenger() {
    printHeader("REGISTER NEW PASSENGER");

    if (loggedInIndex == -1) {
        printf("  [!] Please log in first.\n");
        pauseScreen();
        return;
    }

    if (activeCount() >= VEHICLE_CAPACITY) {
        printf("  [!] Vehicle is FULL. Cannot accept more passengers.\n");
        pauseScreen();
        return;
    }

    Passenger p;
    p.id     = nextID++;
    p.active = 1;

    printf("  Passenger Name    : ");
    getchar();   /* clear leftover newline */
    fgets(p.name, MAX_NAME, stdin);
    p.name[strcspn(p.name, "\n")] = '\0';   /* remove trailing newline */

    printf("  Age               : ");
    scanf("%d", &p.age);

    printf("\n  Passenger Type:\n");
    printf("    1. Adult\n    2. Student (20%% discount)\n");
    printf("    3. Child  (30%% discount)\n    4. Elderly (15%% discount)\n");
    printf("  Choice            : ");
    int choice;
    scanf("%d", &choice);

    if (choice < 1 || choice > 4) choice = 1;
    p.type = (PassengerType) choice;

    printf("  Distance (km)     : ");
    scanf("%lf", &p.distanceKm);

    printf("  Luggage weight(kg): ");
    scanf("%lf", &p.luggageKg);

    p.fare = calculateFare(p.distanceKm, p.type, p.luggageKg);

    passengers[passengerCount++] = p;

    printf("\n  -----------------------------------------------\n");
    printf("  [OK] Passenger registered successfully!\n");
    printf("  ID     : %d\n",    p.id);
    printf("  Name   : %s\n",    p.name);
    printf("  Type   : %s\n",    typeName(p.type));
    printf("  Fare   : UGX %.2f\n", p.fare);
    printf("  -----------------------------------------------\n");
    pauseScreen();
}

void searchPassenger() {
    printHeader("SEARCH PASSENGER");

    if (loggedInIndex == -1) {
        printf("  [!] Please log in first.\n");
        pauseScreen();
        return;
    }

    int id;
    printf("  Enter Passenger ID to search: ");
    scanf("%d", &id);

    for (int i = 0; i < passengerCount; i++) {
        if (passengers[i].id == id && passengers[i].active) {
            Passenger *p = &passengers[i];
            printf("\n  --- Passenger Found ---\n");
            printf("  ID       : %d\n",       p->id);
            printf("  Name     : %s\n",       p->name);
            printf("  Age      : %d\n",       p->age);
            printf("  Type     : %s\n",       typeName(p->type));
            printf("  Distance : %.1f km\n",  p->distanceKm);
            printf("  Luggage  : %.1f kg\n",  p->luggageKg);
            printf("  Fare     : UGX %.2f\n", p->fare);
            pauseScreen();
            return;
        }
    }

    printf("\n  [!] Passenger with ID %d not found.\n", id);
    pauseScreen();
}

void updatePassenger() {
    printHeader("UPDATE PASSENGER DETAILS");

    if (loggedInIndex == -1) {
        printf("  [!] Please log in first.\n");
        pauseScreen();
        return;
    }

    int id;
    printf("  Enter Passenger ID to update: ");
    scanf("%d", &id);

    for (int i = 0; i < passengerCount; i++) {
        if (passengers[i].id == id && passengers[i].active) {
            Passenger *p = &passengers[i];
            printf("  Current Name     : %s\n",       p->name);
            printf("  Current Distance : %.1f km\n",  p->distanceKm);
            printf("  Current Luggage  : %.1f kg\n",  p->luggageKg);
            printf("  Current Type     : %s\n",       typeName(p->type));

            printf("\n  New Name (press Enter to keep): ");
            getchar();
            char newName[MAX_NAME];
            fgets(newName, MAX_NAME, stdin);
            newName[strcspn(newName, "\n")] = '\0';
            if (strlen(newName) > 0)
                strcpy(p->name, newName);

            printf("  New Distance in km (0 = keep): ");
            double d; scanf("%lf", &d);
            if (d > 0) p->distanceKm = d;

            printf("  New Luggage kg   (0 = keep)  : ");
            double l; scanf("%lf", &l);
            if (l >= 0) p->luggageKg = l;

            /* Recalculate fare */
            p->fare = calculateFare(p->distanceKm, p->type, p->luggageKg);

            printf("\n  [OK] Record updated. New fare: UGX %.2f\n", p->fare);
            pauseScreen();
            return;
        }
    }

    printf("  [!] Passenger ID %d not found.\n", id);
    pauseScreen();
}

void deletePassenger() {
    printHeader("DELETE PASSENGER RECORD");

    if (loggedInIndex == -1) {
        printf("  [!] Please log in first.\n");
        pauseScreen();
        return;
    }

    int id;
    printf("  Enter Passenger ID to delete: ");
    scanf("%d", &id);

    for (int i = 0; i < passengerCount; i++) {
        if (passengers[i].id == id && passengers[i].active) {
            passengers[i].active = 0;   /* soft delete */
            printf("\n  [OK] Passenger %s (ID: %d) removed from records.\n",
                   passengers[i].name, id);
            pauseScreen();
            return;
        }
    }

    printf("  [!] Passenger ID %d not found.\n", id);
    pauseScreen();
}

void displayAllPassengers() {
    printHeader("ALL PASSENGERS ON BOARD");

    if (loggedInIndex == -1) {
        printf("  [!] Please log in first.\n");
        pauseScreen();
        return;
    }

    int found = 0;
    printf("  %-4s %-20s %-5s %-8s %-8s %-8s %s\n",
           "ID", "Name", "Age", "Type", "Dist(km)", "Lug(kg)", "Fare(UGX)");
    printLine('-', 72);

    for (int i = 0; i < passengerCount; i++) {
        if (passengers[i].active) {
            Passenger *p = &passengers[i];
            printf("  %-4d %-20s %-5d %-8s %-8.1f %-8.1f %.2f\n",
                   p->id, p->name, p->age,
                   typeName(p->type),
                   p->distanceKm, p->luggageKg, p->fare);
            found = 1;
        }
    }

    if (!found)
        printf("  No passengers registered yet.\n");

    pauseScreen();
}

/* ======================== REPORTS & SUMMARY ======================== */

void tripSummary() {
    printHeader("TRIP SUMMARY & VEHICLE STATUS");

    if (loggedInIndex == -1) {
        printf("  [!] Please log in first.\n");
        pauseScreen();
        return;
    }

    int    count        = activeCount();
    double totalRevenue = 0.0;
    double highestFare  = 0.0;
    char   topPassenger[MAX_NAME] = "None";

    for (int i = 0; i < passengerCount; i++) {
        if (passengers[i].active) {
            totalRevenue += passengers[i].fare;
            if (passengers[i].fare > highestFare) {
                highestFare = passengers[i].fare;
                strcpy(topPassenger, passengers[i].name);
            }
        }
    }

    double occupancy = (count * 100.0) / VEHICLE_CAPACITY;

    printf("  Vehicle Capacity   : %d seats\n",    VEHICLE_CAPACITY);
    printf("  Passengers On Board: %d\n",          count);
    printf("  Occupancy          : %.1f%%\n",      occupancy);
    printf("  Total Revenue      : UGX %.2f\n",    totalRevenue);
    printf("  Highest Fare Paid  : UGX %.2f  by %s\n",
           highestFare, topPassenger);

    printf("\n  Departure Status   : ");
    if (occupancy >= READY_THRESHOLD)
        printf("[READY TO DEPART]\n");
    else
        printf("[NOT READY TO DEPART - %.1f%% more needed]\n",
               READY_THRESHOLD - occupancy);

    pauseScreen();
}

/* ======================== MENUS ======================== */

void accountMenu() {
    int choice;
    do {
        printHeader("ACCOUNT MANAGEMENT");
        if (loggedInIndex != -1)
            printf("  Logged in as: %s\n\n", accounts[loggedInIndex].username);
        else
            printf("  Status: Not logged in\n\n");

        printf("  1. Create New Account\n");
        printf("  2. Login\n");
        printf("  3. Logout\n");
        printf("  4. Change Password\n");
        printf("  5. View All Accounts\n");
        printf("  0. Back to Main Menu\n");
        printLine('-', 40);
        printf("  Your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: createAccount();    break;
            case 2: login();            break;
            case 3: logout();           break;
            case 4: changePassword();   break;
            case 5: viewAllAccounts();  break;
            case 0: break;
            default:
                printf("  [!] Invalid option. Try again.\n");
                pauseScreen();
        }
    } while (choice != 0);
}

void passengerMenu() {
    int choice;
    do {
        printHeader("PASSENGER MANAGEMENT");
        printf("  Active passengers: %d / %d\n\n",
               activeCount(), VEHICLE_CAPACITY);

        printf("  1. Register New Passenger\n");
        printf("  2. Search Passenger by ID\n");
        printf("  3. Update Passenger Details\n");
        printf("  4. Delete Passenger Record\n");
        printf("  5. View All Passengers\n");
        printf("  0. Back to Main Menu\n");
        printLine('-', 40);
        printf("  Your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: registerPassenger();    break;
            case 2: searchPassenger();      break;
            case 3: updatePassenger();      break;
            case 4: deletePassenger();      break;
            case 5: displayAllPassengers(); break;
            case 0: break;
            default:
                printf("  [!] Invalid option. Try again.\n");
                pauseScreen();
        }
    } while (choice != 0);
}

void mainMenu() {
    int choice;
    do {
        printHeader("MAIN MENU");
        if (loggedInIndex != -1)
            printf("  Logged in as: %s\n\n", accounts[loggedInIndex].username);

        printf("  1. Account Management\n");
        printf("  2. Passenger Management\n");
        printf("  3. Trip Summary & Vehicle Status\n");
        printf("  0. Exit Program\n");
        printLine('-', 40);
        printf("  Your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: accountMenu();   break;
            case 2: passengerMenu(); break;
            case 3: tripSummary();   break;
            case 0:
                printHeader("GOODBYE");
                printf("  Thank you for using the Transport Fare Management System.\n");
                printf("  Safe travels!\n\n");
                break;
            default:
                printf("  [!] Invalid option. Please try again.\n");
                pauseScreen();
        }
    } while (choice != 0);
}

/* ======================== MAIN ======================== */

int main() {
    /* Seed with one default admin account for easy testing */
    strcpy(accounts[0].username, "admin");
    strcpy(accounts[0].password, "admin123");
    accounts[0].isLoggedIn = 0;
    accountCount = 1;

    mainMenu();
    return 0;
}
