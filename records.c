/*
 * records.c - Medical Records module.
 * Keeps each patient's medical history (diagnosis + treatment) in records.dat
 * and lets staff search that history by patient ID.
 */
#include <stdio.h>
#include <string.h>
#include "hms.h"

static void printRecordCard(const MedicalRecord *r)
{
    printBorder();
    printBoxLine("Record ID  : %d", r->id);
    printBoxLine("Date       : %s", r->date);
    printBoxWrapped("Diagnosis  : ", r->diagnosis);
    printBoxWrapped("Treatment  : ", r->treatment);
    printBorder();
}

static void addRecord(void)
{
    MedicalRecord r;
    Patient p;
    int pid;

    memset(&r, 0, sizeof r);
    printHeader("ADD MEDICAL RECORD");
    printf("\n");

    pid = readInt("Patient ID", 1, MAX_ID);
    if (!storageFind(PATIENT_FILE, sizeof p, pid, &p)) {
        showError("No patient found with that ID.");
        pauseScreen();
        return;
    }
    printf("  -> Patient: %s, age %d\n\n", p.name, p.age);

    r.id = storageNextId(RECORD_FILE, sizeof r, RECORD_START_ID);
    r.patientId = pid;
    getToday(r.date);
    readText("Diagnosis", r.diagnosis, sizeof r.diagnosis);
    readText("Treatment details", r.treatment, sizeof r.treatment);

    if (storageAppend(RECORD_FILE, &r, sizeof r)) {
        showSuccess("Medical record saved successfully!");
        printf("\n");
        printRecordCard(&r);
    } else {
        showError("Could not write to records.dat.");
    }
    pauseScreen();
}

/* Show the complete history of one patient. */
static void searchHistory(void)
{
    FILE *fp;
    MedicalRecord r;
    Patient p;
    int pid;
    int count = 0;

    printHeader("PATIENT MEDICAL HISTORY");
    printf("\n");
    pid = readInt("Enter Patient ID", 1, MAX_ID);

    if (!storageFind(PATIENT_FILE, sizeof p, pid, &p)) {
        showError("No patient found with that ID.");
        pauseScreen();
        return;
    }
    printf("\n  Patient: %s   Age: %d   Gender: %c\n\n", p.name, p.age, p.gender);

    fp = fopen(RECORD_FILE, "rb");
    if (fp != NULL) {
        while (fread(&r, sizeof r, 1, fp) == 1) {
            if (r.patientId == pid) {
                printRecordCard(&r);
                count++;
            }
        }
        fclose(fp);
    }

    if (count == 0) {
        showInfo("No medical records exist for this patient yet.");
    } else {
        printf("  Total records for this patient: %d\n", count);
    }
    pauseScreen();
}

static void viewAllRecords(void)
{
    FILE *fp = fopen(RECORD_FILE, "rb");
    MedicalRecord r;
    int count = 0;

    printHeader("ALL MEDICAL RECORDS");
    if (fp == NULL) {
        showInfo("No medical records stored yet.");
        pauseScreen();
        return;
    }

    printf("\n");
    printf(" %-7s %-9s %-11s %s\n", "RECORD", "PATIENT", "DATE", "DIAGNOSIS");
    printRule();
    while (fread(&r, sizeof r, 1, fp) == 1) {
        printf(" %-7d %-9d %-11s %.40s\n",
               r.id, r.patientId, r.date, r.diagnosis);
        count++;
    }
    fclose(fp);
    printRule();

    if (count == 0) {
        showInfo("No medical records stored yet.");
    } else {
        printf("  Total records: %d\n", count);
    }
    pauseScreen();
}

void recordsMenu(void)
{
    int choice;

    do {
        printHeader("MEDICAL RECORDS");
        printBoxLine("");
        printMenuItem(1, "Add Medical Record");
        printMenuItem(2, "Search History by Patient ID");
        printMenuItem(3, "View All Records");
        printMenuItem(0, "Back to Main Menu");
        printBoxLine("");
        printBorder();
        printf("\n");
        choice = readInt("Enter your choice", 0, 3);

        switch (choice) {
        case 1: addRecord();      break;
        case 2: searchHistory();  break;
        case 3: viewAllRecords(); break;
        default: break;
        }
    } while (choice != 0);
}
