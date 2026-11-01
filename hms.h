/*
 * hms.h - Hospital Management System
 * Shared constants, data structures and function prototypes.
 *
 * NOTE: Every record structure stores its unique ID as the FIRST member.
 *       The generic storage layer (storage.c) relies on this rule.
 */
#ifndef HMS_H
#define HMS_H

#include <stddef.h>

/* ------------------------------------------------------------------ */
/* Configuration                                                       */
/* ------------------------------------------------------------------ */
#define HOSPITAL_NAME   "CITY CARE MULTISPECIALITY HOSPITAL"
#define UI_WIDTH        78          /* total width of every console box */

#define PATIENT_FILE     "patients.dat"
#define DOCTOR_FILE      "doctors.dat"
#define APPOINTMENT_FILE "appointments.dat"
#define BILL_FILE        "bills.dat"
#define RECORD_FILE      "records.dat"
#define ADMIN_FILE       "admin.dat"
#define TEMP_FILE        "hms_temp.dat"

#define PATIENT_START_ID      1001
#define DOCTOR_START_ID        501
#define APPOINTMENT_START_ID  7001
#define RECORD_START_ID       3001
#define BILL_START_ID         9001
#define MAX_ID              999999

#define DATE_LEN   11               /* DD-MM-YYYY + '\0' */
#define TIME_LEN    6               /* HH:MM + '\0'      */
#define PHONE_LEN  11               /* 10 digits + '\0'  */

/* ------------------------------------------------------------------ */
/* Data structures                                                     */
/* ------------------------------------------------------------------ */
typedef struct {
    int  id;                        /* MUST stay the first member */
    char name[41];
    int  age;
    char gender;                    /* M, F or O */
    char phone[PHONE_LEN];
    char address[80];
    char disease[60];
} Patient;

typedef struct {
    int  id;
    char name[41];
    char specialization[31];
    char phone[PHONE_LEN];
    char availability[31];
} Doctor;

typedef struct {
    int  id;
    int  patientId;
    int  doctorId;
    char date[DATE_LEN];
    char time[TIME_LEN];
} Appointment;

typedef struct {
    int    id;
    int    patientId;
    char   date[DATE_LEN];
    double consultationFee;
    double medicineCharges;
    double otherCharges;
    double total;
} Bill;

typedef struct {
    int  id;
    int  patientId;
    char date[DATE_LEN];
    char diagnosis[121];
    char treatment[121];
} MedicalRecord;

typedef struct {
    char username[21];              /* the id-first rule is not needed here */
    char password[21];
} Admin;

/* ------------------------------------------------------------------ */
/* utils.c - console UI and validated input                            */
/* ------------------------------------------------------------------ */
void   clearScreen(void);
void   pauseScreen(void);
void   printBorder(void);
void   printCentered(const char *text);
void   printBoxLine(const char *fmt, ...);
void   printBoxWrapped(const char *label, const char *text);
void   printMenuItem(int number, const char *text);
void   printHeader(const char *title);
void   printRule(void);
void   showSuccess(const char *message);
void   showError(const char *message);
void   showInfo(const char *message);
void   readText(const char *prompt, char *out, size_t size);
void   readName(const char *prompt, char *out, size_t size);
void   readPhone(char *out, size_t size);
char   readGender(void);
void   readDate(const char *prompt, char *out);
void   readTime(const char *prompt, char *out);
int    readInt(const char *prompt, int min, int max);
double readMoney(const char *prompt);
int    confirmAction(const char *question);
void   getToday(char *out);

/* ------------------------------------------------------------------ */
/* storage.c - generic binary file helpers (first member must be id)   */
/* ------------------------------------------------------------------ */
int storageCount(const char *file, size_t recSize);
int storageNextId(const char *file, size_t recSize, int startId);
int storageFind(const char *file, size_t recSize, int id, void *out);
int storageAppend(const char *file, const void *record, size_t recSize);
int storageUpdate(const char *file, size_t recSize, int id, const void *record);
int storageDelete(const char *file, size_t recSize, int id);

/* ------------------------------------------------------------------ */
/* Modules                                                             */
/* ------------------------------------------------------------------ */
/* auth.c */
int  adminLogin(void);
void changeAdminPassword(void);

/* patient.c */
void patientMenu(void);

/* doctor.c */
void doctorMenu(void);

/* appointment.c */
void appointmentMenu(void);
int  patientHasAppointments(int patientId);
int  doctorHasAppointments(int doctorId);

/* billing.c */
void billingMenu(void);

/* records.c */
void recordsMenu(void);

/* dashboard.c */
void showDashboard(void);

#endif /* HMS_H */
