/*
 * appointment.c - Appointment Management module.
 * Book, view, reassign doctor and cancel appointments (appointments.dat).
 * A doctor cannot be booked twice for the same date and time.
 */
#include <stdio.h>
#include <string.h>
#include "hms.h"

/* Returns 1 when the doctor already has an appointment at this slot.
 * 'ignoreId' lets a reassignment skip the appointment being edited. */
static int slotTaken(int doctorId, const char *date, const char *slot,
                     int ignoreId)
{
    FILE *fp = fopen(APPOINTMENT_FILE, "rb");
    Appointment a;
    int taken = 0;

    if (fp == NULL) {
        return 0;
    }
    while (!taken && fread(&a, sizeof a, 1, fp) == 1) {
        if (a.id != ignoreId && a.doctorId == doctorId &&
            strcmp(a.date, date) == 0 && strcmp(a.time, slot) == 0) {
            taken = 1;
        }
    }
    fclose(fp);
    return taken;
}

/* Used by patient.c / doctor.c to protect against orphaned appointments. */
int patientHasAppointments(int patientId)
{
    FILE *fp = fopen(APPOINTMENT_FILE, "rb");
    Appointment a;
    int found = 0;

    if (fp == NULL) {
        return 0;
    }
    while (!found && fread(&a, sizeof a, 1, fp) == 1) {
        found = (a.patientId == patientId);
    }
    fclose(fp);
    return found;
}

int doctorHasAppointments(int doctorId)
{
    FILE *fp = fopen(APPOINTMENT_FILE, "rb");
    Appointment a;
    int found = 0;

    if (fp == NULL) {
        return 0;
    }
    while (!found && fread(&a, sizeof a, 1, fp) == 1) {
        found = (a.doctorId == doctorId);
    }
    fclose(fp);
    return found;
}

/* Ask for a doctor id that exists; returns 0 and fills 'out' on success. */
static int pickDoctor(Doctor *out)
{
    int id = readInt("Doctor ID", 1, MAX_ID);

    if (!storageFind(DOCTOR_FILE, sizeof *out, id, out)) {
        showError("No doctor found with that ID. Add the doctor first.");
        return -1;
    }
    printf("  -> Dr. selected: %s (%s)\n", out->name, out->specialization);
    printf("  -> Availability: %s\n", out->availability);
    return 0;
}

static void printSlip(const Appointment *a, const Patient *p, const Doctor *d)
{
    printf("\n");
    printBorder();
    printCentered("APPOINTMENT SLIP");
    printBorder();
    printBoxLine("Appointment ID : %d", a->id);
    printBoxLine("Patient        : %s (ID %d)", p->name, p->id);
    printBoxLine("Doctor         : %s (ID %d)", d->name, d->id);
    printBoxLine("Specialization : %s", d->specialization);
    printBoxLine("Date / Time    : %s at %s", a->date, a->time);
    printBorder();
}

static void bookAppointment(void)
{
    Appointment a;
    Patient p;
    Doctor d;
    int pid;

    memset(&a, 0, sizeof a);
    printHeader("BOOK APPOINTMENT");
    printf("\n");

    if (storageCount(PATIENT_FILE, sizeof(Patient)) == 0 ||
        storageCount(DOCTOR_FILE, sizeof(Doctor)) == 0) {
        showError("Add at least one patient and one doctor before booking.");
        pauseScreen();
        return;
    }

    pid = readInt("Patient ID", 1, MAX_ID);
    if (!storageFind(PATIENT_FILE, sizeof p, pid, &p)) {
        showError("No patient found with that ID. Register the patient first.");
        pauseScreen();
        return;
    }
    printf("  -> Patient selected: %s\n\n", p.name);

    if (pickDoctor(&d) != 0) {
        pauseScreen();
        return;
    }
    printf("\n");

    readDate("Appointment date (DD-MM-YYYY)", a.date);
    readTime("Appointment time (HH:MM, 24-hour)", a.time);

    if (slotTaken(d.id, a.date, a.time, 0)) {
        showError("That doctor is already booked at this date and time.");
        pauseScreen();
        return;
    }

    a.id = storageNextId(APPOINTMENT_FILE, sizeof a, APPOINTMENT_START_ID);
    a.patientId = p.id;
    a.doctorId = d.id;

    if (storageAppend(APPOINTMENT_FILE, &a, sizeof a)) {
        showSuccess("Appointment booked successfully!");
        printSlip(&a, &p, &d);
    } else {
        showError("Could not write to appointments.dat.");
    }
    pauseScreen();
}

static void viewAppointments(void)
{
    FILE *fp = fopen(APPOINTMENT_FILE, "rb");
    Appointment a;
    Patient p;
    Doctor d;
    const char *patientName;
    const char *doctorName;
    int count = 0;

    printHeader("ALL APPOINTMENTS");
    if (fp == NULL) {
        showInfo("No appointments booked yet.");
        pauseScreen();
        return;
    }

    printf("\n");
    printf(" %-6s %-18s %-18s %-11s %-5s\n",
           "APPT", "PATIENT", "DOCTOR", "DATE", "TIME");
    printRule();
    while (fread(&a, sizeof a, 1, fp) == 1) {
        patientName = storageFind(PATIENT_FILE, sizeof p, a.patientId, &p)
                      ? p.name : "(unknown)";
        doctorName = storageFind(DOCTOR_FILE, sizeof d, a.doctorId, &d)
                     ? d.name : "(unknown)";
        printf(" %-6d %-18.18s %-18.18s %-11s %-5s\n",
               a.id, patientName, doctorName, a.date, a.time);
        count++;
    }
    fclose(fp);
    printRule();

    if (count == 0) {
        showInfo("No appointments booked yet.");
    } else {
        printf("  Total appointments: %d\n", count);
    }
    pauseScreen();
}

/* Assign a different doctor to an existing appointment. */
static void reassignDoctor(void)
{
    Appointment a;
    Patient p;
    Doctor d;
    int id;

    printHeader("ASSIGN / CHANGE DOCTOR");
    printf("\n");
    id = readInt("Appointment ID", 1, MAX_ID);
    if (!storageFind(APPOINTMENT_FILE, sizeof a, id, &a)) {
        showError("No appointment found with that ID.");
        pauseScreen();
        return;
    }
    printf("  -> Current slot: %s at %s (Doctor ID %d)\n\n",
           a.date, a.time, a.doctorId);

    if (pickDoctor(&d) != 0) {
        pauseScreen();
        return;
    }
    if (slotTaken(d.id, a.date, a.time, a.id)) {
        showError("That doctor is already booked at this date and time.");
        pauseScreen();
        return;
    }

    a.doctorId = d.id;
    if (storageUpdate(APPOINTMENT_FILE, sizeof a, a.id, &a)) {
        showSuccess("Doctor assigned successfully!");
        if (storageFind(PATIENT_FILE, sizeof p, a.patientId, &p)) {
            printSlip(&a, &p, &d);
        }
    } else {
        showError("Update failed. Data was not changed.");
    }
    pauseScreen();
}

static void cancelAppointment(void)
{
    Appointment a;
    int id;

    printHeader("CANCEL APPOINTMENT");
    printf("\n");
    id = readInt("Appointment ID to cancel", 1, MAX_ID);
    if (!storageFind(APPOINTMENT_FILE, sizeof a, id, &a)) {
        showError("No appointment found with that ID.");
        pauseScreen();
        return;
    }

    printf("\n  Appointment %d: Patient ID %d, Doctor ID %d, %s at %s\n\n",
           a.id, a.patientId, a.doctorId, a.date, a.time);

    if (confirmAction("Cancel this appointment?")) {
        if (storageDelete(APPOINTMENT_FILE, sizeof a, id)) {
            showSuccess("Appointment cancelled successfully!");
        } else {
            showError("Cancel failed. Data was not changed.");
        }
    } else {
        showInfo("Cancellation aborted. The appointment is still active.");
    }
    pauseScreen();
}

void appointmentMenu(void)
{
    int choice;

    do {
        printHeader("APPOINTMENT MANAGEMENT");
        printBoxLine("");
        printMenuItem(1, "Book New Appointment");
        printMenuItem(2, "View All Appointments");
        printMenuItem(3, "Assign / Change Doctor");
        printMenuItem(4, "Cancel Appointment");
        printMenuItem(0, "Back to Main Menu");
        printBoxLine("");
        printBorder();
        printf("\n");
        choice = readInt("Enter your choice", 0, 4);

        switch (choice) {
        case 1: bookAppointment();    break;
        case 2: viewAppointments();   break;
        case 3: reassignDoctor();     break;
        case 4: cancelAppointment();  break;
        default: break;
        }
    } while (choice != 0);
}
