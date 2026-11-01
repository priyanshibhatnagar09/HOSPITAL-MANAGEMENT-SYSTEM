/*
 * main.c - Hospital Management System
 * Program entry point: welcome screen, login screen and admin main menu.
 *
 * Build:  gcc -std=c99 -Wall -Wextra -pedantic -o hms *.c
 * Run  :  ./hms          (Windows: hms.exe)
 * Default login:  admin / admin123
 */
#include <stdio.h>
#include "hms.h"

/* Opening splash screen shown once when the program starts. */
static void showWelcomeScreen(void)
{
    clearScreen();
    printBorder();
    printCentered("");
    printCentered("W E L C O M E   T O");
    printCentered("");
    printCentered(HOSPITAL_NAME);
    printCentered("");
    printCentered("+-----+");
    printCentered("|     |");
    printCentered("+--+  +--+");
    printCentered("|        |");
    printCentered("+--+  +--+");
    printCentered("|     |");
    printCentered("+-----+");
    printCentered("");
    printCentered("HOSPITAL MANAGEMENT SYSTEM");
    printCentered("Caring for life, one record at a time");
    printCentered("");
    printBorder();
    pauseScreen();
}

/* Menu shown after a successful login. Returns when the admin logs out. */
static void adminSession(void)
{
    int choice;

    do {
        printHeader("MAIN MENU  -  ADMINISTRATOR");
        printBoxLine("");
        printMenuItem(1, "Patient Management");
        printMenuItem(2, "Doctor Management");
        printMenuItem(3, "Appointment Management");
        printMenuItem(4, "Billing Management");
        printMenuItem(5, "Medical Records");
        printMenuItem(6, "Dashboard & Reports");
        printMenuItem(7, "Change Admin Password");
        printMenuItem(0, "Logout");
        printBoxLine("");
        printBorder();
        printf("\n");
        choice = readInt("Enter your choice", 0, 7);

        switch (choice) {
        case 1: patientMenu();         break;
        case 2: doctorMenu();          break;
        case 3: appointmentMenu();     break;
        case 4: billingMenu();         break;
        case 5: recordsMenu();         break;
        case 6: showDashboard();       break;
        case 7: changeAdminPassword(); break;
        default: break;
        }
    } while (choice != 0);

    printHeader("LOGGED OUT");
    showSuccess("You have been logged out safely. All data is saved.");
    pauseScreen();
}

int main(void)
{
    int choice;

    showWelcomeScreen();

    do {
        printHeader("WELCOME - PLEASE LOG IN");
        printBoxLine("");
        printMenuItem(1, "Admin Login");
        printMenuItem(0, "Exit Program");
        printBoxLine("");
        printBorder();
        printf("\n");
        choice = readInt("Enter your choice", 0, 1);

        if (choice == 1 && adminLogin()) {
            adminSession();
        }
    } while (choice != 0);

    printHeader("THANK YOU");
    printCentered("");
    printCentered("Thank you for using the Hospital Management System.");
    printCentered("Stay healthy!");
    printCentered("");
    printBorder();
    return 0;
}
