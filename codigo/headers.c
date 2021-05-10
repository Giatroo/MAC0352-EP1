#include "headers.h"

FixedHeader *interpret_fixed_header(string recvline, int *start_idx) {
    byte type_flags;
    byte type;
    var_byte remaning_length;
    FixedHeader *header;

    header = malloc(sizeof(FixedHeader));

    type_flags = recvline[(*start_idx)++];
    type = type_flags >> 4;

    header->type = type;

    remaning_length = read_var_byte_integer(recvline, start_idx);
    header->remaning_length = remaning_length;
    return header;
}
