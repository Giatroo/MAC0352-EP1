#include "util.h"

#include <stdio.h>

u_int64_t read_var_byte_integer(string buffer, int *start_idx) {
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

string write_var_byte_integer(u_int64_t value) {
    u_int8_t encoded_byte;
    int i = 0;
    char output[4];
    do {
        encoded_byte = value % 128;
        value /= 128;
        if (value > 0) encoded_byte = encoded_byte | 128;
        output[i++] = (char) encoded_byte;
        fprintf(stdout, "Encoding: %d\n", encoded_byte);
    } while (value > 0);

    fprintf(stdout, "i=%d\n", i);
    string ret = malloc((i+1) * sizeof(char));
    for (int j = 0; j < i; j++) {
        ret[j] = output[j];
        fprintf(stdout, "ret[%d] = %02x\n", j, ret[j]);
    }
    ret[i] = 0;
    return ret;
}
