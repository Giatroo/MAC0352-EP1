#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

typedef char* string;
typedef u_int8_t byte;
typedef u_int64_t var_byte;

u_int64_t read_var_byte_integer(string buffer, int *start_idx);
string write_var_byte_integer(u_int64_t value);

#endif /* ifndef UTIL_H */
