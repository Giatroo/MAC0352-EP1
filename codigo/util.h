#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef char *string;
typedef unsigned char uchar;
typedef uchar *ustring;
typedef u_int8_t byte;

void print_in_hex(ustring s, int len);
void *global_malloc(size_t size);
void global_free(void *addr, size_t size);

#endif /* ifndef UTIL_H */
