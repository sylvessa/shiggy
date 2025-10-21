#ifndef STRING_H
#define STRING_H

#include "types.h"

void dec2str(int32 n, char *des);
void hex2str(nat32 n, char *des);
void str_bin(int32 n, byte *des);
nat32 strlen(const char *s);
int32 strcmp(char *s1, char *s2);
void strcpy(char *des, const char *src);
void str_reverse(char *s);
void strlower(char *s);
void strncpy(char *des, const char *src, nat32 n);
int32 strncmp(const char *s1, const char *s2, nat32 n);

#endif