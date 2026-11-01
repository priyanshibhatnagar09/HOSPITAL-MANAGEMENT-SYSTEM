/*
 * storage.c - Generic file-handling layer.
 *
 * All records are stored as fixed-size binary structures. Because every
 * structure keeps its unique 'id' as the first member, one set of routines
 * can search, update and delete records for patients, doctors,
 * appointments, bills and medical records.
 *
 * A missing file is never an error: it simply means "no records yet".
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hms.h"

/* Read the id stored at the start of a raw record buffer. */
static int idOf(const void *record)
{
    int id;
    memcpy(&id, record, sizeof id);
    return id;
}

/* Number of records in a file (0 when the file does not exist). */
int storageCount(const char *file, size_t recSize)
{
    FILE *fp = fopen(file, "rb");
    long bytes;

    if (fp == NULL) {
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    bytes = ftell(fp);
    fclose(fp);
    if (bytes <= 0) {
        return 0;
    }
    return (int)((size_t)bytes / recSize);
}

/* Next free id: highest existing id + 1, or startId for an empty file. */
int storageNextId(const char *file, size_t recSize, int startId)
{
    FILE *fp = fopen(file, "rb");
    unsigned char *buf;
    int maxId = 0;

    if (fp == NULL) {
        return startId;
    }
    buf = malloc(recSize);
    if (buf == NULL) {
        fclose(fp);
        return startId;
    }
    while (fread(buf, recSize, 1, fp) == 1) {
        if (idOf(buf) > maxId) {
            maxId = idOf(buf);
        }
    }
    free(buf);
    fclose(fp);
    return (maxId >= startId) ? maxId + 1 : startId;
}

/*
 * Look for a record with the given id.
 * Returns 1 if found (copying it into 'out' when out != NULL), else 0.
 */
int storageFind(const char *file, size_t recSize, int id, void *out)
{
    FILE *fp = fopen(file, "rb");
    unsigned char *buf;
    int found = 0;

    if (fp == NULL) {
        return 0;
    }
    buf = malloc(recSize);
    if (buf == NULL) {
        fclose(fp);
        return 0;
    }
    while (!found && fread(buf, recSize, 1, fp) == 1) {
        if (idOf(buf) == id) {
            found = 1;
            if (out != NULL) {
                memcpy(out, buf, recSize);
            }
        }
    }
    free(buf);
    fclose(fp);
    return found;
}

/* Add a new record at the end of the file. Returns 1 on success. */
int storageAppend(const char *file, const void *record, size_t recSize)
{
    FILE *fp = fopen(file, "ab");
    int ok;

    if (fp == NULL) {
        return 0;
    }
    ok = (fwrite(record, recSize, 1, fp) == 1);
    if (fclose(fp) != 0) {
        ok = 0;
    }
    return ok;
}

/*
 * Rewrite the file through a temporary file, either replacing the record
 * with the given id or dropping it. The original is only replaced once the
 * temporary copy has been written completely, so a failure never destroys
 * existing data.
 */
static int rewriteFile(const char *file, size_t recSize, int id,
                       const void *replacement, int removeIt)
{
    FILE *in = fopen(file, "rb");
    FILE *out;
    unsigned char *buf;
    int found = 0;
    int ok = 1;

    if (in == NULL) {
        return 0;
    }
    out = fopen(TEMP_FILE, "wb");
    buf = malloc(recSize);
    if (out == NULL || buf == NULL) {
        if (out != NULL) {
            fclose(out);
            remove(TEMP_FILE);
        }
        free(buf);
        fclose(in);
        return 0;
    }

    while (fread(buf, recSize, 1, in) == 1) {
        if (idOf(buf) == id) {
            found = 1;
            if (removeIt) {
                continue;                       /* skip = delete */
            }
            if (fwrite(replacement, recSize, 1, out) != 1) {
                ok = 0;
            }
        } else if (fwrite(buf, recSize, 1, out) != 1) {
            ok = 0;
        }
    }

    free(buf);
    fclose(in);
    if (fclose(out) != 0) {
        ok = 0;
    }

    if (!found || !ok) {
        remove(TEMP_FILE);
        return 0;
    }
    /* remove() first because rename() fails on Windows if target exists */
    if (remove(file) != 0 || rename(TEMP_FILE, file) != 0) {
        return 0;
    }
    return 1;
}

int storageUpdate(const char *file, size_t recSize, int id, const void *record)
{
    return rewriteFile(file, recSize, id, record, 0);
}

int storageDelete(const char *file, size_t recSize, int id)
{
    return rewriteFile(file, recSize, id, NULL, 1);
}
