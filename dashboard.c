/*
 * dashboard.c - Dashboard and Reports module.
 * Reads every data file and shows hospital statistics with a text bar chart.
 */
#include <stdio.h>
#include <string.h>
#include "hms.h"

#define BAR_WIDTH 30

/* One row of the text bar chart:  Label | ######       | 12 */
static void printBar(const char *label, int value, int maxValue)
{
    char bar[BAR_WIDTH + 1];
    int filled = (maxValue > 0) ? (value * BAR_WIDTH) / maxValue : 0;

    memset(bar, ' ', BAR_WIDTH);
    memset(bar, '#', (size_t)filled);
    bar[BAR_WIDTH] = '\0';
    printBoxLine("  %-14s |%s| %d", label, bar, value);
}

/* Sum of every bill total (0 when there are no bills). */
static double calculateRevenue(void)
{
    FILE *fp = fopen(BILL_FILE, "rb");
    Bill b;
    double revenue = 0.0;

    if (fp == NULL) {
        return 0.0;
    }
    while (fread(&b, sizeof b, 1, fp) == 1) {
        revenue += b.total;
    }
    fclose(fp);
    return revenue;
}

/* Number of appointments scheduled for today's date. */
static int countTodayAppointments(void)
{
    FILE *fp = fopen(APPOINTMENT_FILE, "rb");
    Appointment a;
    char today[DATE_LEN];
    int count = 0;

    if (fp == NULL) {
        return 0;
    }
    getToday(today);
    while (fread(&a, sizeof a, 1, fp) == 1) {
        if (strcmp(a.date, today) == 0) {
            count++;
        }
    }
    fclose(fp);
    return count;
}

void showDashboard(void)
{
    int patients     = storageCount(PATIENT_FILE, sizeof(Patient));
    int doctors      = storageCount(DOCTOR_FILE, sizeof(Doctor));
    int appointments = storageCount(APPOINTMENT_FILE, sizeof(Appointment));
    int bills        = storageCount(BILL_FILE, sizeof(Bill));
    int records      = storageCount(RECORD_FILE, sizeof(MedicalRecord));
    int today        = countTodayAppointments();
    double revenue   = calculateRevenue();
    char date[DATE_LEN];
    int maxValue = patients;

    if (doctors > maxValue)      maxValue = doctors;
    if (appointments > maxValue) maxValue = appointments;
    if (bills > maxValue)        maxValue = bills;
    if (records > maxValue)      maxValue = records;

    getToday(date);
    printHeader("DASHBOARD & REPORTS");
    printBoxLine("");
    printBoxLine("  Report date            : %s", date);
    printBoxLine("");
    printBorder();
    printCentered("HOSPITAL STATISTICS");
    printBorder();
    printBoxLine("  Total Patients         : %d", patients);
    printBoxLine("  Total Doctors          : %d", doctors);
    printBoxLine("  Total Appointments     : %d", appointments);
    printBoxLine("  Appointments Today     : %d", today);
    printBoxLine("  Medical Records        : %d", records);
    printBoxLine("  Bills Generated        : %d", bills);
    printBoxLine("  Total Revenue          : Rs. %.2f", revenue);
    if (bills > 0) {
        printBoxLine("  Average Bill Amount    : Rs. %.2f", revenue / bills);
    }
    printBorder();
    printCentered("OVERVIEW CHART");
    printBorder();
    printBar("Patients", patients, maxValue);
    printBar("Doctors", doctors, maxValue);
    printBar("Appointments", appointments, maxValue);
    printBar("Records", records, maxValue);
    printBar("Bills", bills, maxValue);
    printBorder();

    pauseScreen();
}
