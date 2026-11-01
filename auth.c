/*
 * auth.c - Admin login and password change.
 * Credentials live in admin.dat. On the first run the file is created
 * with the default account:  username = admin   password = admin123
 */
#include <stdio.h>
#include <string.h>
#include "hms.h"

#define MAX_LOGIN_ATTEMPTS 3

static int saveAdmin(const Admin *admin)
{
    FILE *fp = fopen(ADMIN_FILE, "wb");
    int ok;

    if (fp == NULL) {
        return 0;
    }
    ok = (fwrite(admin, sizeof *admin, 1, fp) == 1);
    if (fclose(fp) != 0) {
        ok = 0;
    }
    return ok;
}

/* Load the admin account, creating the default one if needed. */
static void loadAdmin(Admin *admin)
{
    FILE *fp = fopen(ADMIN_FILE, "rb");
    int ok = 0;

    if (fp != NULL) {
        ok = (fread(admin, sizeof *admin, 1, fp) == 1);
        fclose(fp);
    }
    if (!ok) {
        strcpy(admin->username, "admin");
        strcpy(admin->password, "admin123");
        saveAdmin(admin);
    }
}

/* Returns 1 when the admin logs in successfully, 0 otherwise. */
int adminLogin(void)
{
    Admin stored;
    char username[21];
    char password[21];
    int attempt;

    loadAdmin(&stored);

    for (attempt = 1; attempt <= MAX_LOGIN_ATTEMPTS; attempt++) {
        printHeader("ADMIN LOGIN");
        printBoxLine("");
        printBoxLine(" Please enter your administrator credentials.");
        printBoxLine(" Attempt %d of %d", attempt, MAX_LOGIN_ATTEMPTS);
        printBoxLine("");
        printBorder();
        printf("\n");

        readText("Username", username, sizeof username);
        readText("Password", password, sizeof password);

        if (strcmp(username, stored.username) == 0 &&
            strcmp(password, stored.password) == 0) {
            showSuccess("Login successful. Welcome, Administrator!");
            pauseScreen();
            return 1;
        }
        showError("Invalid username or password.");
        if (attempt < MAX_LOGIN_ATTEMPTS) {
            pauseScreen();
        }
    }
    showError("Too many failed attempts. Returning to the main screen.");
    pauseScreen();
    return 0;
}

void changeAdminPassword(void)
{
    Admin stored;
    char current[21];
    char newPass[21];
    char confirm[21];

    loadAdmin(&stored);
    printHeader("CHANGE ADMIN PASSWORD");
    printf("\n");

    readText("Current password", current, sizeof current);
    if (strcmp(current, stored.password) != 0) {
        showError("Current password is incorrect.");
        pauseScreen();
        return;
    }

    readText("New password (4-20 characters)", newPass, sizeof newPass);
    if (strlen(newPass) < 4) {
        showError("Password must be at least 4 characters long.");
        pauseScreen();
        return;
    }
    readText("Confirm new password", confirm, sizeof confirm);
    if (strcmp(newPass, confirm) != 0) {
        showError("Passwords do not match. Nothing was changed.");
        pauseScreen();
        return;
    }

    strcpy(stored.password, newPass);
    if (saveAdmin(&stored)) {
        showSuccess("Password changed successfully!");
    } else {
        showError("Could not save the new password.");
    }
    pauseScreen();
}
