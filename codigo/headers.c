#include "headers.h"

#include <stdio.h>
#include <unistd.h>

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

byte *encode_connack(ConnackVarHeader *connack_header) {
    byte *encoded_str;

    /* first let's calculate the lengths of some properties */

    // flags, reason code, topic alias, id len, receive maximum
    int total_length = 11;
    total_length += connack_header->client_id_len;

    int prop_length = total_length - 2;

    u_int16_t len_prop_length;
    byte *str_prop_length =
        write_var_byte_integer(prop_length, &len_prop_length);
    free(str_prop_length);

    total_length += len_prop_length;

    FixedHeader *fixed_header = malloc(sizeof(FixedHeader));
    fixed_header->type = CONNACK_PACKAGE << 4;
    fixed_header->remaning_length = total_length;

    /* now let's create the encoded string */
    encoded_str = malloc((1 + 2 + total_length) * sizeof(char));

    /* the first byte is the type and flags */
    encoded_str[0] = fixed_header->type;

    /* the next bytes are the bytes for the remaning length  */
    /* since we're using the PID as user ID, it will never pass 1 byte */
    encoded_str[1] = 0x0c;

    /* before the fixed header, we have the acknowledge flags */
    encoded_str[2] = connack_header->ack_flags;

    /* the reason code */
    encoded_str[3] = 0x00;

    /* the properties length */
    encoded_str[4] = 0x09;

    /* now we write the topic alias maximum id */
    encoded_str[5] = 0x22;
    /* and the value */
    encoded_str[6] = 0x00;
    encoded_str[7] = 0x0a;

    /* now we write the assigned client identifier */
    encoded_str[8] = 0x12;
    /* the length: */
    encoded_str[9] = 0x00;
    /* encoded_str[8] = 0x03; */
    encoded_str[10] = connack_header->client_id_len;
    /* and the id */
    /* encoded_str[11] = connack_header->client_id[0]; */
    /* encoded_str[12] = connack_header->client_id[1]; */
    /* encoded_str[13] = connack_header->client_id[2]; */
    encoded_str[11] = 'a';
    encoded_str[12] = 'a';
    encoded_str[13] = 'a';

    /* ending with the receive maximum */
    /* the id */
    /* encoded_str[14] = 0x21; */
    /* and the value */
    /* encoded_str[15] = 0x00; */
    /* encoded_str[16] = 0x14; */

    fprintf(stdout, "encoded: '");
    for (int i = 0; i < 14; i++) { fprintf(stdout, "%02x ", encoded_str[i]); }
    fprintf(stdout, "'\n");

    return encoded_str;
}
