/*
 * by Zijie Song
 */

#include "helpers.h"

void trans(char *a, int x) {
    *(int *) a = x;
}

void trans(char *a, float x) {
    *(float *) a = x;
}

void trans(char *a, string x) {
    memcpy(a, x.c_str(), x.length() + 1);
}

int get(char *a, int x) {
    return *(int *) a;
}

float get(char *a, float x) {
    return *(float *) a;
}

string get(char *a, string x) {
    return (string) a;
}

int rtype(int x) {
    return 0;
}

int rtype(float x) {
    return 1;
}

int rtype(string x) {
    return 2;
}