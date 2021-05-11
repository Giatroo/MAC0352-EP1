#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef char *string;
typedef unsigned char *ustring;
typedef u_int8_t byte;
typedef u_int64_t var_byte;

u_int64_t read_var_byte_integer(ustring buffer, int *start_idx);
byte *write_var_byte_integer(u_int64_t value, u_int16_t *len);

#endif /* ifndef UTIL_H */
