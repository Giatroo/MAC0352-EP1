#include "util.h"

#include <stdio.h>

u_int64_t read_var_byte_integer(ustring buffer, int *start_idx) {
    u_int64_t multiplier = 1;
    u_int64_t value = 0;
    u_int8_t encoded_byte;
    do {
        encoded_byte = buffer[(*start_idx)++];
        value += (encoded_byte & 127) * multiplier;
        multiplier *= 128;
    } while ((encoded_byte & 128) != 0);

    return value;
}

byte *write_var_byte_integer(u_int64_t value, u_int16_t *i) {
    /* fprintf(stdout, "\nEntering write var byte integer\n"); */
    u_int8_t encoded_byte;
    *i = 0;
    byte output[4];

    /* fprintf(stdout, "value: %ld\n", value); */

    do {
        encoded_byte = value % 128U;
        /* fprintf(stdout, "Encoding: %d\n", encoded_byte); */
        value /= 128U;
        /* fprintf(stdout, "value: %ld\n", value); */
        if (value > 0) encoded_byte = encoded_byte | 128U;
        output[(*i)++] = (unsigned char) encoded_byte;
        /* fprintf(stdout, "Encoding: %d\n", encoded_byte); */
        /* fprintf(stdout, "output: %02x\n\n", output[*i - 1]); */
    } while (value > 0);

    /* fprintf(stdout, "i=%d\n", *i); */
    byte *ret = malloc(*i * sizeof(byte));
    for (int j = 0; j < *i; j++) {
        ret[j] = output[j];
        /* fprintf(stdout, "ret[%d] = %02x\n", j, ret[j]); */
    }
    return ret;
}
