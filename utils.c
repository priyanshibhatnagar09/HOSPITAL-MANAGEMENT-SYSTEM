/*
 * utils.c - Console interface helpers and validated input routines.
 * Every piece of user input in the project goes through this file, so
 * validation rules live in exactly one place.
 */
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hms.h"

/* ================================================================== */
/* Low-level input                                                     */
/* ================================================================== */

/* Remove leading and trailing white space from a string in place. */
static void trimInPlace(char *s)
{
    size_t len = strlen(s);
    size_t start = 0;

    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }
    while (s[start] != '\0' && isspace((unsigned char)s[start])) {
        start++;
    }
    if (start > 0) {
        memmove(s, s + start, strlen(s + start) + 1);
    }
}

/*
 * Read one line from the keyboard safely.
 * - Uses fgets so the buffer can never overflow.
 * - Discards any extra characters when the line is too long.
 * - Exits cleanly if the input stream is closed (EOF).
 */
static void readLine(char *buf, size_t size)
{
    size_t len;

    if (fgets(buf, (int)size, stdin) == NULL) {
        printf("\n  Input closed. Exiting program.\n");
        exit(0);
    }
    len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* discard the rest of an over-long line */
        }
    }
    trimInPlace(buf);
}

/* ================================================================== */
/* Screen helpers                                                      */
/* ================================================================== */

void clearScreen(void)
{
    int status;
#ifdef _WIN32
    status = system("cls");
#else
    status = system("clear");
#endif
    if (status != 0) {
        /* Terminal could not be cleared; just separate screens instead. */
        printf("\n\n");
    }
}

void pauseScreen(void)
{
    char tmp[8];
    printf("\n  Press ENTER to continue...");
    readLine(tmp, sizeof tmp);
}

/* +----------------------------------------------------+ */
void printBorder(void)
{
    int i;
    putchar('+');
    for (i = 0; i < UI_WIDTH - 2; i++) {
        putchar('-');
    }
    printf("+\n");
}

/* Plain horizontal rule used under table headings. */
void printRule(void)
{
    int i;
    putchar(' ');
    for (i = 0; i < UI_WIDTH - 2; i++) {
        putchar('-');
    }
    putchar('\n');
}

/* |            centred text            | */
void printCentered(const char *text)
{
    int inner = UI_WIDTH - 2;
    int len = (int)strlen(text);
    int left;
    int right;

    if (len > inner) {
        len = inner;
    }
    left = (inner - len) / 2;
    right = inner - len - left;
    printf("|%*s%.*s%*s|\n", left, "", len, text, right, "");
}

/* | formatted left-aligned text (printf style)                | */
void printBoxLine(const char *fmt, ...)
{
    char text[256];
    va_list args;

    va_start(args, fmt);
    vsnprintf(text, sizeof text, fmt, args);
    va_end(args);
    printf("| %-*.*s|\n", UI_WIDTH - 3, UI_WIDTH - 3, text);
}

/* Print "Label : long text" and wrap onto extra lines when needed. */
void printBoxWrapped(const char *label, const char *text)
{
    int labelLen = (int)strlen(label);
    int avail = UI_WIDTH - 3 - labelLen;
    size_t pos = 0;
    size_t len = strlen(text);

    do {
        printBoxLine("%-*s%.*s", labelLen, pos == 0 ? label : "",
                     avail, text + pos);
        pos += (size_t)avail;
    } while (pos < len);
}

void printMenuItem(int number, const char *text)
{
    printBoxLine("   [%d]  %s", number, text);
}

/* Clears the screen and prints the standard hospital banner + title. */
void printHeader(const char *title)
{
    clearScreen();
    printBorder();
    printCentered(HOSPITAL_NAME);
    printBorder();
    printCentered(title);
    printBorder();
}

void showSuccess(const char *message)
{
    printf("\n  [ SUCCESS ]  %s\n", message);
}

void showError(const char *message)
{
    printf("  [ ERROR ]  %s\n", message);
}

void showInfo(const char *message)
{
    printf("\n  [ INFO ]  %s\n", message);
}

/* ================================================================== */
/* Validated input                                                     */
/* ================================================================== */

/* Read a non-empty line that fits in 'out' (size includes the '\0'). */
void readText(const char *prompt, char *out, size_t size)
{
    char line[256];

    for (;;) {
        printf("  %s: ", prompt);
        readLine(line, sizeof line);
        if (line[0] == '\0') {
            showError("This field cannot be empty.");
            continue;
        }
        if (strlen(line) >= size) {
            printf("  [ ERROR ]  Too long. Maximum %u characters.\n",
                   (unsigned)(size - 1));
            continue;
        }
        strcpy(out, line);
        return;
    }
}

/* A name may contain letters, spaces, dots, hyphens and apostrophes. */
void readName(const char *prompt, char *out, size_t size)
{
    char line[256];
    size_t i;
    int valid;

    for (;;) {
        readText(prompt, line, sizeof line);
        valid = (strlen(line) >= 2);
        for (i = 0; valid && line[i] != '\0'; i++) {
            if (!isalpha((unsigned char)line[i]) && line[i] != ' ' &&
                line[i] != '.' && line[i] != '-' && line[i] != '\'') {
                valid = 0;
            }
        }
        if (!valid) {
            showError("Use letters only (at least 2 characters).");
            continue;
        }
        if (strlen(line) >= size) {
            printf("  [ ERROR ]  Too long. Maximum %u characters.\n",
                   (unsigned)(size - 1));
            continue;
        }
        strcpy(out, line);
        return;
    }
}

/* Phone numbers must be exactly 10 digits. */
void readPhone(char *out, size_t size)
{
    char line[64];
    size_t i;
    int valid;

    for (;;) {
        readText("Phone number (10 digits)", line, sizeof line);
        valid = (strlen(line) == 10 && strlen(line) < size);
        for (i = 0; valid && line[i] != '\0'; i++) {
            if (!isdigit((unsigned char)line[i])) {
                valid = 0;
            }
        }
        if (!valid) {
            showError("Phone number must contain exactly 10 digits.");
            continue;
        }
        strcpy(out, line);
        return;
    }
}

char readGender(void)
{
    char line[16];
    char c;

    for (;;) {
        printf("  Gender (M = Male, F = Female, O = Other): ");
        readLine(line, sizeof line);
        if (strlen(line) == 1) {
            c = (char)toupper((unsigned char)line[0]);
            if (c == 'M' || c == 'F' || c == 'O') {
                return c;
            }
        }
        showError("Please enter M, F or O.");
    }
}

/* Integer within [min, max]; rejects letters, blanks and overflow. */
int readInt(const char *prompt, int min, int max)
{
    char line[64];
    char *end;
    long value;

    for (;;) {
        printf("  %s: ", prompt);
        readLine(line, sizeof line);
        if (line[0] == '\0') {
            showError("Input cannot be empty.");
            continue;
        }
        errno = 0;
        value = strtol(line, &end, 10);
        if (*end != '\0' || errno != 0) {
            showError("Please enter a valid whole number.");
            continue;
        }
        if (value < min || value > max) {
            printf("  [ ERROR ]  Value must be between %d and %d.\n", min, max);
            continue;
        }
        return (int)value;
    }
}

/* Non-negative money amount such as 500 or 249.50 (max 10,000,000). */
double readMoney(const char *prompt)
{
    char line[64];
    size_t i;
    int digits;
    int dots;
    int valid;
    double value;

    for (;;) {
        printf("  %s (Rs.): ", prompt);
        readLine(line, sizeof line);
        digits = 0;
        dots = 0;
        valid = (line[0] != '\0');
        for (i = 0; valid && line[i] != '\0'; i++) {
            if (isdigit((unsigned char)line[i])) {
                digits++;
            } else if (line[i] == '.') {
                dots++;
            } else {
                valid = 0;
            }
        }
        if (!valid || digits == 0 || dots > 1) {
            showError("Enter a valid amount, for example 500 or 249.50");
            continue;
        }
        value = strtod(line, NULL);
        if (value > 10000000.0) {
            showError("Amount is too large.");
            continue;
        }
        return value;
    }
}

int confirmAction(const char *question)
{
    char line[16];

    for (;;) {
        printf("  %s (Y/N): ", question);
        readLine(line, sizeof line);
        if (line[1] == '\0' && (line[0] == 'Y' || line[0] == 'y')) {
            return 1;
        }
        if (line[1] == '\0' && (line[0] == 'N' || line[0] == 'n')) {
            return 0;
        }
        showError("Please answer Y or N.");
    }
}

/* ================================================================== */
/* Date and time                                                       */
/* ================================================================== */

static int isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/* Checks the text "DD-MM-YYYY" and that the calendar date really exists. */
static int isValidDate(const char *s)
{
    static const int daysInMonth[12] = {31, 28, 31, 30, 31, 30,
                                        31, 31, 30, 31, 30, 31};
    int i, day, month, year, maxDay;

    if (strlen(s) != 10 || s[2] != '-' || s[5] != '-') {
        return 0;
    }
    for (i = 0; i < 10; i++) {
        if (i != 2 && i != 5 && !isdigit((unsigned char)s[i])) {
            return 0;
        }
    }
    day   = (s[0] - '0') * 10 + (s[1] - '0');
    month = (s[3] - '0') * 10 + (s[4] - '0');
    year  = (s[6] - '0') * 1000 + (s[7] - '0') * 100 +
            (s[8] - '0') * 10 + (s[9] - '0');

    if (year < 1900 || year > 2100 || month < 1 || month > 12) {
        return 0;
    }
    maxDay = daysInMonth[month - 1];
    if (month == 2 && isLeapYear(year)) {
        maxDay = 29;
    }
    return day >= 1 && day <= maxDay;
}

/* 'out' must hold at least DATE_LEN characters. */
void readDate(const char *prompt, char *out)
{
    char line[64];

    for (;;) {
        printf("  %s: ", prompt);
        readLine(line, sizeof line);
        if (isValidDate(line)) {
            strcpy(out, line);
            return;
        }
        showError("Invalid date. Use DD-MM-YYYY, e.g. 15-08-2026");
    }
}

/* 'out' must hold at least TIME_LEN characters. 24-hour HH:MM. */
void readTime(const char *prompt, char *out)
{
    char line[64];
    int hour, minute;

    for (;;) {
        printf("  %s: ", prompt);
        readLine(line, sizeof line);
        if (strlen(line) == 5 && line[2] == ':' &&
            isdigit((unsigned char)line[0]) && isdigit((unsigned char)line[1]) &&
            isdigit((unsigned char)line[3]) && isdigit((unsigned char)line[4])) {
            hour   = (line[0] - '0') * 10 + (line[1] - '0');
            minute = (line[3] - '0') * 10 + (line[4] - '0');
            if (hour <= 23 && minute <= 59) {
                strcpy(out, line);
                return;
            }
        }
        showError("Invalid time. Use 24-hour HH:MM, e.g. 14:30");
    }
}

/* Today's date as DD-MM-YYYY; 'out' needs DATE_LEN characters. */
void getToday(char *out)
{
    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    if (local == NULL ||
        strftime(out, DATE_LEN, "%d-%m-%Y", local) == 0) {
        strcpy(out, "01-01-2026");
    }
}
