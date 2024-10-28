#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_BOATS 120
#define MAX_NAME_LEN 128

// Rates per foot per month
#define SLIP_RATE 12.5
#define LAND_RATE 14.0
#define TRAILOR_RATE 25.0
#define STORAGE_RATE 11.2

typedef enum {
    slip,
    land,
    trailor,
    storage,
    no_place
} PlaceType;

typedef union {
    int slipNumber;
    char bayLetter;
    char trailorTag[20];
    int storageNumber;
} LocationDetail;

typedef struct {
    char name[MAX_NAME_LEN];
    int length; // Length in feet
    PlaceType placeType;
    LocationDetail locationDetail;
    float amountOwed;
} Boat;

Boat* boats[MAX_BOATS];
int boatCount = 0;

// Function to convert string to PlaceType
PlaceType StringToPlaceType(char *PlaceString) {
    if (!strcasecmp(PlaceString, "slip")) return slip;
    if (!strcasecmp(PlaceString, "land")) return land;
    if (!strcasecmp(PlaceString, "trailor")) return trailor;
    if (!strcasecmp(PlaceString, "storage")) return storage;
    return no_place;
}

// Function to convert PlaceType to string
char * PlaceToString(PlaceType Place) {
    switch (Place) {
        case slip: return "slip";
        case land: return "land";
        case trailor: return "trailor";
        case storage: return "storage";
        case no_place: return "no_place";
        default:
            printf("Invalid PlaceType\n");
            exit(EXIT_FAILURE);
            break;
    }
}

// Function to calculate monthly charge based on boat's place type and length
float calculateMonthlyCharge(const Boat *boat) {
    float rate = 0.0f;
    switch (boat->placeType) {
        case slip: rate = SLIP_RATE; break;
        case land: rate = LAND_RATE; break;
        case trailor: rate = TRAILOR_RATE; break;
        case storage: rate = STORAGE_RATE; break;
        case no_place:  // Handle no_place by setting rate to 0
            rate = 0.0f;
            break;
        default: rate = 0.0f;
    }
    return rate * boat->length;
}

// Load boats from CSV file
void loadBoatsFromFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        Boat *newBoat = (Boat*)malloc(sizeof(Boat));
        char type[20], extraInfo[20];
        sscanf(line, "%[^,],%d,%[^,],%[^,],%f", newBoat->name, &newBoat->length, type, extraInfo, &newBoat->amountOwed);
        if (newBoat->placeType == no_place) {
            printf("Warning: Invalid place type '%s' for boat '%s'. Skipping this entry.\n", type, newBoat->name);
            free(newBoat); // Free memory for invalid entry
            continue;
        }
        newBoat->placeType = StringToPlaceType(type);
        switch (newBoat->placeType) {
case slip:
                newBoat->locationDetail.slipNumber = atoi(extraInfo);
                break;
            case land:
                newBoat->locationDetail.bayLetter = extraInfo[0];
                break;
            case trailor:
                strcpy(newBoat->locationDetail.trailorTag, extraInfo);
                break;
            case storage:
                newBoat->locationDetail.storageNumber = atoi(extraInfo);
                break;
            default:
                printf("Invalid place type in data.\n");
                free(newBoat);
                continue;
        }
        boats[boatCount++] = newBoat;
    }
    fclose(file);
}

// Save boats to CSV file
void saveBoatsToFile(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return;
    }
    for (int i = 0; i < boatCount; i++) {
        Boat *boat = boats[i];
        fprintf(file, "%s,%d,%s,", boat->name, boat->length, PlaceToString(boat->placeType));
        switch (boat->placeType) {
            case slip:
                fprintf(file, "%d", boat->locationDetail.slipNumber);
                break;
            case land:
                fprintf(file, "%c", boat->locationDetail.bayLetter);
                break;
            case trailor:
                fprintf(file, "%s", boat->locationDetail.trailorTag);
                break;
            case storage:
                fprintf(file, "%d", boat->locationDetail.storageNumber);
                break;
            case no_place:  // Handle no_place by writing "N/A" in the file
                fprintf(file, "N/A");
                break;
        }
        fprintf(file, ",%.2f\n", boat->amountOwed);
    }
fclose(file);
}

// Display boat inventory sorted alphabetically
void printInventory() {
    for (int i = 0; i < boatCount - 1; i++) {
        for (int j = 0; j < boatCount - i - 1; j++) {
            if (strcasecmp(boats[j]->name, boats[j + 1]->name) > 0) {
 Boat *temp = boats[j];
                boats[j] = boats[j + 1];
                boats[j + 1] = temp;
            }
        }
    }
    for (int i = 0; i < boatCount; i++) {
        Boat *boat = boats[i];
        printf("%-20s %3d' %s ", boat->name, boat->length, PlaceToString(boat->placeType));
        switch (boat->placeType) {
            case slip:
                printf("# %2d   ", boat->locationDetail.slipNumber);
                break;
            case land:
                printf("    %c   ", boat->locationDetail.bayLetter);
                break;
            case trailor:
                printf("%s   ", boat->locationDetail.trailorTag);
                break;
            case storage:
                printf("# %2d   ", boat->locationDetail.storageNumber);
                break;
            case no_place:
                printf("N/A   ");
                break;
            default:
                printf("Unknown location");
        }
        printf("Owes $%.2f\n", boat->amountOwed);
    }
}

// Add a boat from CSV-style input
void addBoatFromCSV(const char *csvLine) {
    if (boatCount >= MAX_BOATS) {
        printf("Error: Marina is full.\n");
        return;
    }
    Boat *newBoat = (Boat*)malloc(sizeof(Boat));
    char type[20], extraInfo[20];
    sscanf(csvLine, "%[^,],%d,%[^,],%[^,],%f", newBoat->name, &newBoat->length, type, extraInfo, &newBoat->amountOwed);
    newBoat->placeType = StringToPlaceType(type);
    if (newBoat->placeType == no_place) {
        printf("Error: Invalid place type '%s' for boat '%s'. Boat not added.\n", type, newBoat->name);
        free(newBoat); // Free memory for invalid entry
        return;
    }
    switch (newBoat->placeType) {
        case slip:
            newBoat->locationDetail.slipNumber = atoi(extraInfo);
            break;
        case land:
            newBoat->locationDetail.bayLetter = extraInfo[0];
            break;
        case trailor:
            strcpy(newBoat->locationDetail.trailorTag, extraInfo);
            break;
 case storage:
            newBoat->locationDetail.storageNumber = atoi(extraInfo);
            break;
        default:
            printf("Invalid location type.\n");
            free(newBoat);
            return;
}
    boats[boatCount++] = newBoat;
}

// Remove a boat by name
void removeBoatByName(const char *name) {
    int index = -1;
    for (int i = 0; i < boatCount; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        printf("No boat with that name.\n");
        return;
    }
    free(boats[index]);
    for (int i = index; i < boatCount - 1; i++) {
        boats[i] = boats[i + 1];
    }
    boatCount--;
}

// Accept a payment for a boat
void acceptPayment(const char *name, float amount) {
    int index = -1;
    for (int i = 0; i < boatCount; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        printf("No boat with that name.\n");
        return;
    }
    if (amount > boats[index]->amountOwed) {
        printf("That is more than the amount owed, $%.2f\n", boats[index]->amountOwed);
        return;
    }
    boats[index]->amountOwed -= amount;
    printf("Payment accepted. New balance: $%.2f\n", boats[index]->amountOwed);
}

// Update charges for a new month
void updateForNewMonth() {
    for (int i = 0; i < boatCount; i++) {
        float charge = calculateMonthlyCharge(boats[i]);
        boats[i]->amountOwed += charge;
        printf("Updated charges for '%s'. New amount owed: $%.2f\n", boats[i]->name, boats[i]->amountOwed);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <BoatData.csv>\n", argv[0]);
        return 1;
    }
    loadBoatsFromFile(argv[1]);
    printf("Welcome to the Boat Management System\n-------------------------------------\n");
    char option;
    while (1) {
        printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
        scanf(" %c", &option);
        option = tolower(option);
        switch (option) {
            case 'i':
                printInventory();
break;
            case 'a': {
                char csvLine[256];
                printf("Please enter the boat data in CSV format                 : ");
                scanf(" %[^\n]", csvLine);
                addBoatFromCSV(csvLine);
                break;
            }
            case 'r': {
                char name[MAX_NAME_LEN];
                printf("Please enter the boat name                               : ");
                scanf(" %[^\n]", name);
                removeBoatByName(name);
                break;
            }
            case 'p': {
                char name[MAX_NAME_LEN];
                float amount;
                printf("Please enter the boat name                               : ");
                scanf(" %[^\n]", name);
                int index = -1;
                for (int i = 0; i < boatCount; i++) {
                    if (strcasecmp(boats[i]->name, name) == 0) {
                        index = i;
                        break;
                    }
                }
                if (index == -1) {
                    printf("No boat with that name.\n");
                    break;
                }
                printf("Please enter the amount to be paid                       : ");
                scanf("%f", &amount);
                if (amount > boats[index]->amountOwed) {
                    printf("That is more than the amount owed, $%.2f\n", boats[index]->amountOwed);
                } else {
                    boats[index]->amountOwed -= amount;
                    printf("Payment accepted. New balance: $%.2f\n", boats[index]->amountOwed);
                }
                break;
            }
            case 'm':
                updateForNewMonth();
                break;
            case 'x':
                saveBoatsToFile(argv[1]);
                printf("Exiting the Boat Management System\n");
                return 0;
            default:
                printf("Invalid option %c\n", option);
        }
    }
    return 0;
}

