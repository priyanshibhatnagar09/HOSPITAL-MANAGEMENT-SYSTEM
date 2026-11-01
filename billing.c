/*
 * billing.c - Billing Management module.
 * Generates bills, calculates the total automatically, stores every bill
 * in bills.dat and also writes a printable copy to bill_<id>.txt.
 */
#include <stdio.h>
#include <string.h>
#include "hms.h"

/* Display one bill as an invoice on screen. */
static void printBill(const Bill *b)
{
    Patient p;
    const char *name = storageFind(PATIENT_FILE, sizeof p, b->patientId, &p)
                       ? p.name : "(patient record removed)";

    printBorder();
    printCentered("HOSPITAL BILL / INVOICE");
    printBorder();
    printBoxLine("Bill No       : %d", b->id);
    printBoxLine("Date          : %s", b->date);
    printBoxLine("Patient ID    : %d", b->patientId);
    printBoxLine("Patient Name  : %s", name);
    printBoxLine("");
    printBoxLine("Consultation Fee   : Rs. %12.2f", b->consultationFee);
    printBoxLine("Medicine Charges   : Rs. %12.2f", b->medicineCharges);
    printBoxLine("Other Charges      : Rs. %12.2f", b->otherCharges);
    printBoxLine("-----------------------------------------");
    printBoxLine("TOTAL AMOUNT       : Rs. %12.2f", b->total);
    printBorder();
}

/* Save a plain-text copy of the bill. Returns 1 on success. */
static int saveBillText(const Bill *b)
{
    char filename[40];
    FILE *fp;
    Patient p;
    const char *name = storageFind(PATIENT_FILE, sizeof p, b->patientId, &p)
                       ? p.name : "(patient record removed)";

    snprintf(filename, sizeof filename, "bill_%d.txt", b->id);
    fp = fopen(filename, "w");
    if (fp == NULL) {
        return 0;
    }
    fprintf(fp, "==========================================\n");
    fprintf(fp, "      %s\n", HOSPITAL_NAME);
    fprintf(fp, "           PATIENT BILL / INVOICE\n");
    fprintf(fp, "==========================================\n");
    fprintf(fp, "Bill No       : %d\n", b->id);
    fprintf(fp, "Date          : %s\n", b->date);
    fprintf(fp, "Patient ID    : %d\n", b->patientId);
    fprintf(fp, "Patient Name  : %s\n\n", name);
    fprintf(fp, "Consultation Fee : Rs. %12.2f\n", b->consultationFee);
    fprintf(fp, "Medicine Charges : Rs. %12.2f\n", b->medicineCharges);
    fprintf(fp, "Other Charges    : Rs. %12.2f\n", b->otherCharges);
    fprintf(fp, "------------------------------------------\n");
    fprintf(fp, "TOTAL AMOUNT     : Rs. %12.2f\n", b->total);
    fprintf(fp, "==========================================\n");
    fprintf(fp, "      Thank you. Get well soon!\n");
    return fclose(fp) == 0;
}

static void generateBill(void)
{
    Bill b;
    Patient p;
    int pid;

    memset(&b, 0, sizeof b);
    printHeader("GENERATE PATIENT BILL");
    printf("\n");

    pid = readInt("Patient ID", 1, MAX_ID);
    if (!storageFind(PATIENT_FILE, sizeof p, pid, &p)) {
        showError("No patient found with that ID.");
        pauseScreen();
        return;
    }
    printf("  -> Billing patient: %s\n\n", p.name);

    b.consultationFee = readMoney("Consultation fee");
    b.medicineCharges = readMoney("Medicine charges");
    b.otherCharges    = readMoney("Other charges");

    /* The total is always calculated by the program, never typed in. */
    b.total = b.consultationFee + b.medicineCharges + b.otherCharges;
    b.id = storageNextId(BILL_FILE, sizeof b, BILL_START_ID);
    b.patientId = pid;
    getToday(b.date);

    if (!storageAppend(BILL_FILE, &b, sizeof b)) {
        showError("Could not write to bills.dat. Bill was not saved.");
        pauseScreen();
        return;
    }

    showSuccess("Bill generated and saved successfully!");
    printf("\n");
    printBill(&b);

    if (saveBillText(&b)) {
        printf("  A printable copy was saved as bill_%d.txt\n", b.id);
    } else {
        showError("The bill is stored, but the text copy could not be created.");
    }
    pauseScreen();
}

static void viewBills(void)
{
    FILE *fp = fopen(BILL_FILE, "rb");
    Bill b;
    double revenue = 0.0;
    int count = 0;

    printHeader("ALL BILLS");
    if (fp == NULL) {
        showInfo("No bills generated yet.");
        pauseScreen();
        return;
    }

    printf("\n");
    printf(" %-7s %-11s %-10s %14s\n", "BILL", "DATE", "PATIENT", "TOTAL (Rs.)");
    printRule();
    while (fread(&b, sizeof b, 1, fp) == 1) {
        printf(" %-7d %-11s %-10d %14.2f\n",
               b.id, b.date, b.patientId, b.total);
        revenue += b.total;
        count++;
    }
    fclose(fp);
    printRule();

    if (count == 0) {
        showInfo("No bills generated yet.");
    } else {
        printf("  Bills: %d      Total revenue: Rs. %.2f\n", count, revenue);
    }
    pauseScreen();
}

static void searchBill(void)
{
    Bill b;
    int id;

    printHeader("SEARCH BILL BY ID");
    printf("\n");
    id = readInt("Enter Bill No", 1, MAX_ID);

    if (storageFind(BILL_FILE, sizeof b, id, &b)) {
        printf("\n");
        printBill(&b);
    } else {
        showError("No bill found with that number.");
    }
    pauseScreen();
}

void billingMenu(void)
{
    int choice;

    do {
        printHeader("BILLING MANAGEMENT");
        printBoxLine("");
        printMenuItem(1, "Generate Patient Bill");
        printMenuItem(2, "View All Bills");
        printMenuItem(3, "Search Bill by Bill No");
        printMenuItem(0, "Back to Main Menu");
        printBoxLine("");
        printBorder();
        printf("\n");
        choice = readInt("Enter your choice", 0, 3);

        switch (choice) {
        case 1: generateBill(); break;
        case 2: viewBills();    break;
        case 3: searchBill();   break;
        default: break;
        }
    } while (choice != 0);
}
