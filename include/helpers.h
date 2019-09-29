/*
 * by Zijie Song
 */

#ifndef MINISQL_HELPERS_H
#define MINISQL_HELPERS_H


#include <cstring>
#include <string>

using namespace std;

void trans(char *a, int x);

void trans(char *a, float x);

void trans(char *a, string x);

int get(char *a, int x);

float get(char *a, float x);

string get(char *a, string x);

int rtype(int x);

int rtype(float x);

int rtype(string x);

#endif //MINISQL_HELPERS_H
