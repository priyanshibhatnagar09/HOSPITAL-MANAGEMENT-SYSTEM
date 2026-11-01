/*
 * doctor.c - Doctor Management module.
 * Add, view, search, update and delete doctor records (doctors.dat).
 */
#include <stdio.h>
#include <string.h>
#include "hms.h"

/* Display one doctor as a bordered card. */
static void printDoctorCard(const Doctor *d)
{
    printBorder();
    printBoxLine("Doctor ID       : %d", d->id);
    printBoxLine("Name            : %s", d->name);
    printBoxLine("Specialization  : %s", d->specialization);
    printBoxLine("Contact number  : %s", d->phone);
    printBoxLine("Availability    : %s", d->availability);
    printBorder();
}

/* Ask for an id until it is valid and not already used. */
static int readUniqueDoctorId(void)
{
    char prompt[64];
    int id;
    int suggested = storageNextId(DOCTOR_FILE, sizeof(Doctor),
                                  DOCTOR_START_ID);

    for (;;) {
        snprintf(prompt, sizeof prompt, "Doctor ID (suggested: %d)", suggested);
        id = readInt(prompt, 1, MAX_ID);
        if (storageFind(DOCTOR_FILE, sizeof(Doctor), id, NULL)) {
            showError("This Doctor ID already exists. Choose another one.");
            continue;
        }
        return id;
    }
}

static void addDoctor(void)
{
    Doctor d;

    memset(&d, 0, sizeof d);
    printHeader("ADD NEW DOCTOR");
    printf("\n");

    d.id = readUniqueDoctorId();
    readName("Doctor name", d.name, sizeof d.name);
    readText("Specialization", d.specialization, sizeof d.specialization);
    readPhone(d.phone, sizeof d.phone);
    readText("Availability (e.g. Mon-Fri 9AM-4PM)", d.availability,
             sizeof d.availability);

    if (storageAppend(DOCTOR_FILE, &d, sizeof d)) {
        showSuccess("Doctor record saved successfully!");
        printf("\n");
        printDoctorCard(&d);
    } else {
        showError("Could not write to doctors.dat.");
    }
    pauseScreen();
}

static void viewDoctors(void)
{
    FILE *fp = fopen(DOCTOR_FILE, "rb");
    Doctor d;
    int count = 0;

    printHeader("ALL DOCTORS");
    if (fp == NULL) {
        showInfo("No doctor records found yet.");
        pauseScreen();
        return;
    }

    printf("\n");
    printf(" %-5s %-20s %-16s %-11s %s\n",
           "ID", "NAME", "SPECIALIZATION", "PHONE", "AVAILABILITY");
    printRule();
    while (fread(&d, sizeof d, 1, fp) == 1) {
        printf(" %-5d %-20.20s %-16.16s %-11s %.20s\n",
               d.id, d.name, d.specialization, d.phone, d.availability);
        count++;
    }
    fclose(fp);
    printRule();

    if (count == 0) {
        showInfo("No doctor records found yet.");
    } else {
        printf("  Total doctors: %d\n", count);
    }
    pauseScreen();
}

static void searchDoctor(void)
{
    Doctor d;
    int id;

    printHeader("SEARCH DOCTOR BY ID");
    printf("\n");
    id = readInt("Enter Doctor ID", 1, MAX_ID);

    if (storageFind(DOCTOR_FILE, sizeof d, id, &d)) {
        printf("\n");
        printDoctorCard(&d);
    } else {
        showError("No doctor found with that ID.");
    }
    pauseScreen();
}

static void updateDoctor(void)
{
    Doctor d;
    int id;
    int choice;
    int again = 1;

    printHeader("UPDATE DOCTOR INFORMATION");
    printf("\n");
    id = readInt("Enter Doctor ID to update", 1, MAX_ID);
    if (!storageFind(DOCTOR_FILE, sizeof d, id, &d)) {
        showError("No doctor found with that ID.");
        pauseScreen();
        return;
    }

    while (again) {
        printHeader("UPDATE DOCTOR INFORMATION");
        printDoctorCard(&d);
        printBoxLine("What would you like to change?");
        printMenuItem(1, "Name");
        printMenuItem(2, "Specialization");
        printMenuItem(3, "Contact number");
        printMenuItem(4, "Availability");
        printMenuItem(0, "Save changes and go back");
        printBorder();
        printf("\n");
        choice = readInt("Enter your choice", 0, 4);

        switch (choice) {
        case 1: readName("New name", d.name, sizeof d.name); break;
        case 2: readText("New specialization", d.specialization,
                         sizeof d.specialization); break;
        case 3: readPhone(d.phone, sizeof d.phone); break;
        case 4: readText("New availability", d.availability,
                         sizeof d.availability); break;
        default: again = 0; break;
        }
    }

    if (storageUpdate(DOCTOR_FILE, sizeof d, id, &d)) {
        showSuccess("Doctor record updated successfully!");
    } else {
        showError("Update failed. Data was not changed.");
    }
    pauseScreen();
}

static void deleteDoctor(void)
{
    Doctor d;
    int id;

    printHeader("DELETE DOCTOR RECORD");
    printf("\n");
    id = readInt("Enter Doctor ID to delete", 1, MAX_ID);
    if (!storageFind(DOCTOR_FILE, sizeof d, id, &d)) {
        showError("No doctor found with that ID.");
        pauseScreen();
        return;
    }

    printf("\n");
    printDoctorCard(&d);

    /* Do not leave appointments pointing at a doctor that no longer exists. */
    if (doctorHasAppointments(id)) {
        showError("This doctor still has appointments. Cancel or reassign them first.");
        pauseScreen();
        return;
    }

    if (confirmAction("Are you sure you want to delete this doctor?")) {
        if (storageDelete(DOCTOR_FILE, sizeof d, id)) {
            showSuccess("Doctor record deleted successfully!");
        } else {
            showError("Delete failed. Data was not changed.");
        }
    } else {
        showInfo("Delete cancelled.");
    }
    pauseScreen();
}

void doctorMenu(void)
{
    int choice;

    do {
        printHeader("DOCTOR MANAGEMENT");
        printBoxLine("");
        printMenuItem(1, "Add New Doctor");
        printMenuItem(2, "View All Doctors");
        printMenuItem(3, "Search Doctor by ID");
        printMenuItem(4, "Update Doctor Information");
        printMenuItem(5, "Delete Doctor Record");
        printMenuItem(0, "Back to Main Menu");
        printBoxLine("");
        printBorder();
        printf("\n");
        choice = readInt("Enter your choice", 0, 5);

        switch (choice) {
        case 1: addDoctor();    break;
        case 2: viewDoctors();  break;
        case 3: searchDoctor(); break;
        case 4: updateDoctor(); break;
        case 5: deleteDoctor(); break;
        default: break;
        }
    } while (choice != 0);
}
