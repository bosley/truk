#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <stdio.h>
#include <sys/stat.h>

static inline long read_file_size(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return (long)st.st_size;
    }
    return -1;
}

static inline int read_first_byte(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        return -1;
    }
    int c = fgetc(f);
    fclose(f);
    return c;
}

static inline int read_file_as_number(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        return -1;
    }
    int num = 0;
    if (fscanf(f, "%d", &num) != 1) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return num;
}

#endif
