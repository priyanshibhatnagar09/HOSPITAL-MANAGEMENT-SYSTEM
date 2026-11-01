/*
 * patient.c - Patient Management module.
 * Add, view, search, update and delete patient records (patients.dat).
 */
#include <stdio.h>
#include <string.h>
#include "hms.h"

/* Display one patient as a bordered card. */
static void printPatientCard(const Patient *p)
{
    printBorder();
    printBoxLine("Patient ID    : %d", p->id);
    printBoxLine("Name          : %s", p->name);
    printBoxLine("Age / Gender  : %d / %c", p->age, p->gender);
    printBoxLine("Phone         : %s", p->phone);
    printBoxWrapped("Address       : ", p->address);
    printBoxWrapped("Disease       : ", p->disease);
    printBorder();
}

/* Ask for an id until it is valid and not already used. */
static int readUniquePatientId(void)
{
    char prompt[64];
    int id;
    int suggested = storageNextId(PATIENT_FILE, sizeof(Patient),
                                  PATIENT_START_ID);

    for (;;) {
        snprintf(prompt, sizeof prompt, "Patient ID (suggested: %d)", suggested);
        id = readInt(prompt, 1, MAX_ID);
        if (storageFind(PATIENT_FILE, sizeof(Patient), id, NULL)) {
            showError("This Patient ID already exists. Choose another one.");
            continue;
        }
        return id;
    }
}

static void addPatient(void)
{
    Patient p;

    memset(&p, 0, sizeof p);
    printHeader("ADD NEW PATIENT");
    printf("\n");

    p.id = readUniquePatientId();
    readName("Full name", p.name, sizeof p.name);
    p.age = readInt("Age (0-120)", 0, 120);
    p.gender = readGender();
    readPhone(p.phone, sizeof p.phone);
    readText("Address", p.address, sizeof p.address);
    readText("Disease / Complaint", p.disease, sizeof p.disease);

    if (storageAppend(PATIENT_FILE, &p, sizeof p)) {
        showSuccess("Patient record saved successfully!");
        printf("\n");
        printPatientCard(&p);
    } else {
        showError("Could not write to patients.dat.");
    }
    pauseScreen();
}

static void viewPatients(void)
{
    FILE *fp = fopen(PATIENT_FILE, "rb");
    Patient p;
    int count = 0;

    printHeader("ALL PATIENTS");
    if (fp == NULL) {
        showInfo("No patient records found yet.");
        pauseScreen();
        return;
    }

    printf("\n");
    printf(" %-6s %-20s %-4s %-4s %-11s %s\n",
           "ID", "NAME", "AGE", "SEX", "PHONE", "DISEASE");
    printRule();
    while (fread(&p, sizeof p, 1, fp) == 1) {
        printf(" %-6d %-20.20s %-4d %-4c %-11s %.24s\n",
               p.id, p.name, p.age, p.gender, p.phone, p.disease);
        count++;
    }
    fclose(fp);
    printRule();

    if (count == 0) {
        showInfo("No patient records found yet.");
    } else {
        printf("  Total patients: %d\n", count);
    }
    pauseScreen();
}

static void searchPatient(void)
{
    Patient p;
    int id;

    printHeader("SEARCH PATIENT BY ID");
    printf("\n");
    id = readInt("Enter Patient ID", 1, MAX_ID);

    if (storageFind(PATIENT_FILE, sizeof p, id, &p)) {
        printf("\n");
        printPatientCard(&p);
    } else {
        showError("No patient found with that ID.");
    }
    pauseScreen();
}

static void updatePatient(void)
{
    Patient p;
    int id;
    int choice;
    int again = 1;

    printHeader("UPDATE PATIENT DETAILS");
    printf("\n");
    id = readInt("Enter Patient ID to update", 1, MAX_ID);
    if (!storageFind(PATIENT_FILE, sizeof p, id, &p)) {
        showError("No patient found with that ID.");
        pauseScreen();
        return;
    }

    while (again) {
        printHeader("UPDATE PATIENT DETAILS");
        printPatientCard(&p);
        printBoxLine("What would you like to change?");
        printMenuItem(1, "Name");
        printMenuItem(2, "Age");
        printMenuItem(3, "Gender");
        printMenuItem(4, "Phone number");
        printMenuItem(5, "Address");
        printMenuItem(6, "Disease");
        printMenuItem(0, "Save changes and go back");
        printBorder();
        printf("\n");
        choice = readInt("Enter your choice", 0, 6);

        switch (choice) {
        case 1: readName("New name", p.name, sizeof p.name); break;
        case 2: p.age = readInt("New age (0-120)", 0, 120); break;
        case 3: p.gender = readGender(); break;
        case 4: readPhone(p.phone, sizeof p.phone); break;
        case 5: readText("New address", p.address, sizeof p.address); break;
        case 6: readText("New disease", p.disease, sizeof p.disease); break;
        default: again = 0; break;
        }
    }

    if (storageUpdate(PATIENT_FILE, sizeof p, id, &p)) {
        showSuccess("Patient record updated successfully!");
    } else {
        showError("Update failed. Data was not changed.");
    }
    pauseScreen();
}

static void deletePatient(void)
{
    Patient p;
    int id;

    printHeader("DELETE PATIENT RECORD");
    printf("\n");
    id = readInt("Enter Patient ID to delete", 1, MAX_ID);
    if (!storageFind(PATIENT_FILE, sizeof p, id, &p)) {
        showError("No patient found with that ID.");
        pauseScreen();
        return;
    }

    printf("\n");
    printPatientCard(&p);

    /* Do not leave appointments pointing at a patient that no longer exists. */
    if (patientHasAppointments(id)) {
        showError("This patient still has appointments. Cancel them first.");
        pauseScreen();
        return;
    }

    if (confirmAction("Are you sure you want to delete this patient?")) {
        if (storageDelete(PATIENT_FILE, sizeof p, id)) {
            showSuccess("Patient record deleted successfully!");
        } else {
            showError("Delete failed. Data was not changed.");
        }
    } else {
        showInfo("Delete cancelled.");
    }
    pauseScreen();
}

void patientMenu(void)
{
    int choice;

    do {
        printHeader("PATIENT MANAGEMENT");
        printBoxLine("");
        printMenuItem(1, "Add New Patient");
        printMenuItem(2, "View All Patients");
        printMenuItem(3, "Search Patient by ID");
        printMenuItem(4, "Update Patient Details");
        printMenuItem(5, "Delete Patient Record");
        printMenuItem(0, "Back to Main Menu");
        printBoxLine("");
        printBorder();
        printf("\n");
        choice = readInt("Enter your choice", 0, 5);

        switch (choice) {
        case 1: addPatient();    break;
        case 2: viewPatients();  break;
        case 3: searchPatient(); break;
        case 4: updatePatient(); break;
        case 5: deletePatient(); break;
        default: break;
        }
    } while (choice != 0);
}
