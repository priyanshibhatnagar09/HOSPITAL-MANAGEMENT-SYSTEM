# Hospital Management System (C Language)

A complete, console-based Hospital Management System written in **pure C (C99)**
using only the standard library. All records are stored permanently in binary
`.dat` files with `fopen`, `fread`, `fwrite` and `fclose`.

* Language: C99, no external libraries, no databases
* Compiler: GCC (also works in Code::Blocks / MinGW)
* Default login: **admin / admin123**

---

## 1. Project Objectives

1. Replace paper registers with a fast, reliable digital record system.
2. Manage patients, doctors, appointments, bills and medical history in one program.
3. Demonstrate core C concepts: structures, functions, file handling, modular design and input validation.
4. Guarantee data safety: records survive program exit, missing files are handled, duplicate IDs are rejected.
5. Provide a professional, easy-to-use console interface for non-technical staff.

---

## 2. Architecture and Folder Structure

```
HospitalManagementSystem/
|-- hms.h            Shared constants, all structures, all prototypes
|-- main.c           Entry point: welcome screen, login screen, main menu
|-- utils.c          Console UI (boxes, menus) + validated input functions
|-- storage.c        Generic file layer (count, find, append, update, delete)
|-- auth.c           Admin login, logout support, password change
|-- patient.c        Patient Management module
|-- doctor.c         Doctor Management module
|-- appointment.c    Appointment Management module
|-- billing.c        Billing Management module
|-- records.c        Medical Records module
|-- dashboard.c      Dashboard and Reports module
|-- Makefile         Build script
|-- README.md        This documentation
`-- tests/
    `-- test_input.txt   Scripted keyboard input for an automated test run

Created automatically at run time:
    admin.dat  patients.dat  doctors.dat  appointments.dat
    bills.dat  records.dat   bill_<number>.txt (printable bills)
```

### Layered design

```
        +--------------------------------------------+
        |                  main.c                    |   menus and navigation
        +--------------------------------------------+
        | patient | doctor | appointment | billing |  |
        | records | dashboard | auth                   |   feature modules
        +--------------------------------------------+
        |   utils.c (UI + input)   storage.c (files) |   shared services
        +--------------------------------------------+
        |        hms.h  (structures, constants)      |
        +--------------------------------------------+
```

Feature modules never touch the keyboard or screen directly (they use `utils.c`)
and never repeat file-rewriting logic (they use `storage.c`).

### Data structures (hms.h)

| Structure | Fields |
|---|---|
| `Patient` | id, name, age, gender, phone, address, disease |
| `Doctor` | id, name, specialization, phone, availability |
| `Appointment` | id, patientId, doctorId, date, time |
| `Bill` | id, patientId, date, consultationFee, medicineCharges, otherCharges, total |
| `MedicalRecord` | id, patientId, date, diagnosis, treatment |
| `Admin` | username, password |

Every record keeps its unique `id` as the **first member**. This lets one generic
storage layer find, update and delete records of any type.

### How the storage layer works

* **Append** opens the file in `"ab"` mode and writes one structure.
* **Find / count / next ID** read the file record by record. A missing file means "no records", never an error.
* **Update / delete** copy every record into a temporary file, replacing or skipping the target, then swap the files. The original file is replaced only after the new copy is fully written, so a failure cannot destroy existing data.

---

## 3. Module Explanation

### Admin Login (auth.c)
Username and password are checked against `admin.dat` (created with the default
account on first run). Three attempts are allowed. **Logout** returns to the login
screen. The password can be changed from the main menu (current password required,
new one must be 4+ characters and confirmed).

### Patient Management (patient.c)
Add, view all (table), search by ID (card view), update (choose which field to
change) and delete (with confirmation). Patient IDs are unique; the system suggests
the next free ID. A patient who still has appointments cannot be deleted.

### Doctor Management (doctor.c)
Same operations for doctors: name, specialization, contact number and
availability. A doctor with appointments cannot be deleted.

### Appointment Management (appointment.c)
Book an appointment (patient and doctor must exist), store date and time, view all
appointments with patient and doctor names, assign or change the doctor, and cancel
(with confirmation). A doctor cannot be double-booked at the same date and time.
Appointment IDs are generated automatically.

### Billing Management (billing.c)
Enter consultation, medicine and other charges. The total is **calculated by the
program**. The bill is displayed, saved in `bills.dat` and also written as a
printable `bill_<number>.txt`. Bills can be listed (with total revenue) or searched.

### Medical Records (records.c)
Add a diagnosis and treatment for a patient (dated automatically), search the full
history using a patient ID, or list all records.

### Dashboard and Reports (dashboard.c)
Shows total patients, doctors, appointments, today's appointments, medical records,
bills, total revenue, average bill and a text bar chart.

### Utilities (utils.c)
All input goes through validated readers: whole numbers in range, names (letters
only), 10-digit phones, gender (M/F/O), money amounts, real calendar dates
(`DD-MM-YYYY`, leap years handled), 24-hour times and Y/N confirmations. Input is
read with `fgets`, so overflow is impossible.

---

## 4. Compile and Run

### GCC on Linux / macOS / Windows (MinGW)
```
cd HospitalManagementSystem
make                     # or the single command below
gcc -std=c99 -Wall -Wextra -pedantic -o hms *.c
./hms                    # Windows: hms.exe
```

### Code::Blocks
1. File > New > Project > **Console application** > **C**.
2. Remove the default `main.c`.
3. Project > **Add files** and select every `.c` file and `hms.h`.
4. Build and run (F9). Run it from a folder where you are happy for the `.dat` files to be created.

The build produces **no errors and no warnings** with `-Wall -Wextra -pedantic`.

---

## 5. Sample Output (captured from a real run)

### Main menu
```
+----------------------------------------------------------------------------+
|                     CITY CARE MULTISPECIALITY HOSPITAL                     |
+----------------------------------------------------------------------------+
|                        MAIN MENU  -  ADMINISTRATOR                         |
+----------------------------------------------------------------------------+
|                                                                            |
|    [1]  Patient Management                                                 |
|    [2]  Doctor Management                                                  |
|    [3]  Appointment Management                                             |
|    [4]  Billing Management                                                 |
|    [5]  Medical Records                                                    |
|    [6]  Dashboard & Reports                                                |
|    [7]  Change Admin Password                                              |
|    [0]  Logout                                                             |
|                                                                            |
+----------------------------------------------------------------------------+
```

### Adding a patient (with validation errors)
```
  Patient ID (suggested: 1002): 1001
  [ ERROR ]  This Patient ID already exists. Choose another one.
  Patient ID (suggested: 1002): 1002
  Full name: Simran Kaur
  Age (0-120): abc
  [ ERROR ]  Please enter a valid whole number.
  Age (0-120): 28
  Gender (M = Male, F = Female, O = Other): F
  Phone number (10 digits): 123
  [ ERROR ]  Phone number must contain exactly 10 digits.
  Phone number (10 digits): 9812345678
  Address: Sarabha Nagar, Ludhiana
  Disease / Complaint: Migraine

  [ SUCCESS ]  Patient record saved successfully!
```

### Patient card
```
+----------------------------------------------------------------------------+
| Patient ID    : 1001                                                       |
| Name          : Rahul Sharma                                               |
| Age / Gender  : 34 / M                                                     |
| Phone         : 9876543210                                                 |
| Address       : Model Town, Ludhiana                                       |
| Disease       : Fever                                                      |
+----------------------------------------------------------------------------+
```

### View all patients
```

 ID     NAME                 AGE  SEX  PHONE       DISEASE
 ----------------------------------------------------------------------------
 1001   Rahul Sharma         34   M    9876543210  Fever
 1002   Simran Kaur          28   F    9812345678  Migraine
 ----------------------------------------------------------------------------
  Total patients: 2
```

### Appointment slip and validation
```
  Appointment date (DD-MM-YYYY): 31-02-2026
  [ ERROR ]  Invalid date. Use DD-MM-YYYY, e.g. 15-08-2026
  Appointment time (HH:MM, 24-hour): 25:00
  [ ERROR ]  Invalid time. Use 24-hour HH:MM, e.g. 14:30
  [ ERROR ]  That doctor is already booked at this date and time.

+----------------------------------------------------------------------------+
|                              APPOINTMENT SLIP                              |
+----------------------------------------------------------------------------+
| Appointment ID : 7001                                                      |
| Patient        : Rahul Sharma (ID 1001)                                    |
| Doctor         : Dr. Anil Verma (ID 501)                                   |
| Specialization : Cardiology                                                |
| Date / Time    : 15-11-2026 at 10:30                                       |
+----------------------------------------------------------------------------+
```

### Bill
```
+----------------------------------------------------------------------------+
|                          HOSPITAL BILL / INVOICE                           |
+----------------------------------------------------------------------------+
| Bill No       : 9001                                                       |
| Date          : 01-11-2026                                                 |
| Patient ID    : 1001                                                       |
| Patient Name  : Rahul Sharma                                               |
|                                                                            |
| Consultation Fee   : Rs.       500.00                                      |
| Medicine Charges   : Rs.       250.50                                      |
| Other Charges      : Rs.       100.00                                      |
| -----------------------------------------                                  |
| TOTAL AMOUNT       : Rs.       850.50                                      |
+----------------------------------------------------------------------------+
```

### Medical history
```
+----------------------------------------------------------------------------+
| Record ID  : 3001                                                          |
| Date       : 01-11-2026                                                    |
| Diagnosis  : Viral fever                                                   |
| Treatment  : Paracetamol 500mg, rest and plenty of fluids                  |
+----------------------------------------------------------------------------+
```

### Dashboard
```
+----------------------------------------------------------------------------+
|                     CITY CARE MULTISPECIALITY HOSPITAL                     |
+----------------------------------------------------------------------------+
|                            DASHBOARD & REPORTS                             |
+----------------------------------------------------------------------------+
|                                                                            |
|   Report date            : 01-11-2026                                      |
|                                                                            |
+----------------------------------------------------------------------------+
|                            HOSPITAL STATISTICS                             |
+----------------------------------------------------------------------------+
|   Total Patients         : 1                                               |
|   Total Doctors          : 2                                               |
|   Total Appointments     : 1                                               |
|   Appointments Today     : 0                                               |
|   Medical Records        : 1                                               |
|   Bills Generated        : 1                                               |
|   Total Revenue          : Rs. 850.50                                      |
|   Average Bill Amount    : Rs. 850.50                                      |
+----------------------------------------------------------------------------+
|                               OVERVIEW CHART                               |
+----------------------------------------------------------------------------+
|   Patients       |###############               | 1                        |
|   Doctors        |##############################| 2                        |
|   Appointments   |###############               | 1                        |
|   Records        |###############               | 1                        |
|   Bills          |###############               | 1                        |
+----------------------------------------------------------------------------+
```

---

## 6. Testing Instructions

### A. Automated scripted run
`tests/test_input.txt` contains the keystrokes for a full session (login failure,
login, add/update patients, add doctors, book/reassign/cancel appointments, delete,
billing, records, dashboard, logout).

```
mkdir testrun && cd testrun
cp ../hms .                       # copy the compiled program
rm -f *.dat                       # start with empty data
./hms < ../tests/test_input.txt   # replay the session
```
After the run you should see `patients.dat`, `doctors.dat`, `appointments.dat`,
`bills.dat`, `records.dat` and `bill_9001.txt`. Run it from an empty folder, as the
script assumes patient ID 1001 and the default password.

### B. Manual test cases

| # | Test | Steps | Expected result |
|---|---|---|---|
| 1 | Wrong login | admin / wrong | "Invalid username or password"; 3 attempts then back |
| 2 | Correct login | admin / admin123 | Main menu opens |
| 3 | Duplicate patient ID | Add patient 1001 twice | Second attempt rejected |
| 4 | Invalid age | Enter `abc` or `200` | Error, asks again |
| 5 | Invalid phone | Enter `123` | Error, asks again |
| 6 | Search missing ID | Search patient 9999 | "No patient found" |
| 7 | Update | Change a patient's age | New value visible in View All |
| 8 | Appointment, unknown patient | Book with ID 9999 | Rejected |
| 9 | Double booking | Same doctor, date and time twice | Second booking rejected |
| 10 | Impossible date | `31-02-2026` | Rejected (leap years handled too) |
| 11 | Delete protection | Delete a patient with appointments | Blocked with message |
| 12 | Cancel confirmation | Answer `N` | Appointment kept |
| 13 | Bill total | 500 + 250.50 + 100 | Total 850.50, `bill_9001.txt` created |
| 14 | Empty files | Run in an empty folder, open each list | "No ... found yet" messages, no crash |
| 15 | Persistence | Add data, exit, start again | Data still present |
| 16 | Dashboard | After the steps above | Counts and revenue match the data |

### C. Memory-safety check (optional)
```
gcc -std=c99 -g -fsanitize=address,undefined -o hms_san *.c
./hms_san < tests/test_input.txt
```
No sanitizer messages were reported during development.

---

## 7. Safety and Quality Features

* Input read with `fgets` (no `scanf` overflow); over-long lines are discarded.
* Every numeric, date, time, phone and name field is validated before use.
* Unique IDs for every record type; duplicates rejected.
* Missing or empty data files are treated as "no records".
* Safe update/delete through a temporary file.
* Referential checks: appointments need an existing patient and doctor; people with appointments cannot be deleted.
* Closed input (EOF) exits cleanly instead of looping.
* Comments explain every module and important block.

---

## 8. Known Limitations

* Passwords are visible while typing, because standard C has no portable hidden-input function.
* Passwords are stored as plain text in `admin.dat`.
* `.dat` files are binary dumps of structures, so they are not portable between different compilers/platforms.
* Single admin account; single user at a time.
* Searches scan the file sequentially (fine for college-scale data).

---

## 9. Future Scope

1. Password hiding and hashing; multiple user roles (admin, receptionist, doctor, accountant).
2. Search by name, phone or specialization; sorting and filtering.
3. Appointment reminders, doctor shift calendars and available-slot suggestions.
4. Inpatient features: wards, beds, admission and discharge.
5. Pharmacy and inventory management with stock alerts.
6. Insurance claims, discounts, taxes and part-payment tracking.
7. Export reports to CSV and print-ready summaries; daily/monthly revenue reports.
8. Automatic data backup and restore.
9. Replace flat files with an indexed file format or SQLite.
10. Graphical or web front end, and a networked multi-user version.

---

## 10. Conclusion

This project delivers a complete, working Hospital Management System in standard
C. It covers the whole patient journey: registration, doctor assignment,
appointment booking, medical history, billing and management reporting. The code
is modular, validated, commented and compiles cleanly with strict warnings.
Because all data is stored in files, nothing is lost when the program closes.
The project shows how structures, functions and file handling can be combined into
a practical application, and the layered design makes it easy to extend with the
features listed under Future Scope.
