#include "headers.h"

#include <stdio.h>
#include <unistd.h>

FixedHeader *interpret_fixed_header(ustring recvline, int *start_idx) {
    byte type_flags;
    byte type;
    u_int64_t remaning_length;
    FixedHeader *header;

    header = malloc(sizeof(FixedHeader));

    type_flags = recvline[(*start_idx)++];
    type = type_flags >> 4;

    header->type = type;

    remaning_length = read_var_byte_integer(recvline, start_idx);
    header->remaning_length = remaning_length;
    return header;
}

byte *encode_connack(ConnackVarHeader *connack_header, int *encoded_len) {
    byte *encoded_str;
    int i = 0;

    /* first let's calculate the lengths of some properties */

    int remaning_length = 0;
    remaning_length += 1; // ack flags
    remaning_length += 1; // reason code
    remaning_length += 1; // prop length  (hardcoding 1 byte)
    remaning_length += 3; // topic_alias_maximum (id and value)
    remaning_length += 3; // Assigned Client ID (id and length)
    remaning_length += connack_header->client_id_len;

    connack_header->property_length = remaning_length - 3;

    FixedHeader *fixed_header = malloc(sizeof(FixedHeader));
    fixed_header->type = CONNACK_PACKAGE << 4;
    fixed_header->remaning_length = remaning_length;

    /* now let's create the encoded string */
    encoded_str = malloc((1 + remaning_length) * sizeof(byte));

    /* the first byte is the type and flags */
    encoded_str[i++] = fixed_header->type;

    /* the next bytes are the bytes for the remaning length */
    encoded_str[i++] = fixed_header->remaning_length;

    /* before the fixed header, we have the acknowledge flags */
    encoded_str[i++] = connack_header->ack_flags;

    /* the reason code */
    encoded_str[i++] = connack_header->reason_code;

    /* the properties length */
    encoded_str[i++] = connack_header->property_length;
    fprintf(stdout, "property_length: %ld\n", connack_header->property_length);
    fprintf(stdout, "encoded_prop_len: %02x\n", encoded_str[i - 1]);

    /* now we write the topic alias maximum id */
    encoded_str[i++] = 0x22;
    /* and the value */
    encoded_str[i++] = 0x00;
    encoded_str[i++] = 0x0a;

    /* now we write the assigned client identifier */
    encoded_str[i++] = 0x12;
    /* the length: */
    encoded_str[i++] = 0x00;
    encoded_str[i++] = connack_header->client_id_len;
    /* and the id */
    for (int j = 0; j < connack_header->client_id_len; j++)
        encoded_str[i++] = connack_header->client_id[j];

    fprintf(stdout, "encoded: '");
    for (int j = 0; j < i; j++) { fprintf(stdout, "%02x ", encoded_str[j]); }
    fprintf(stdout, "'\n");

    *encoded_len = i;
    return encoded_str;
}

SubscribeHeader *interpret_subscribe_header(ustring recvline, int *start_idx,
                                            int remaning_length) {
    SubscribeHeader *sub_header = malloc(sizeof(SubscribeHeader));
    int i = *start_idx;

    sub_header->msg_id = (1 << 8) * recvline[i] + recvline[i + 1];
    i += 2;

    sub_header->prop_len = read_var_byte_integer(recvline, &i);

    sub_header->topic_len = (1 << 8) * recvline[i] + recvline[i + 1];
    i += 2;

    sub_header->topic_value =
        malloc((sub_header->topic_len + 1) * sizeof(char));
    memcpy(sub_header->topic_value, &recvline[i], sub_header->topic_len);
    sub_header->topic_value[sub_header->topic_len] = 0;
    i += sub_header->topic_len;

    sub_header->sub_options = recvline[i++];

    return sub_header;
}

PublishHeader *interpret_publish_header(ustring recvline, int *start_idx,
                                        int remaning_length) {
    PublishHeader *pub_header = malloc(sizeof(PublishHeader));
    int i = *start_idx;

    pub_header->topic_len = (1 << 8) * recvline[i] + recvline[i + 1];
    i += 2;

    pub_header->topic_value =
        malloc((pub_header->topic_len + 1) * sizeof(char));
    memcpy(pub_header->topic_value, &recvline[i], pub_header->topic_len);
    pub_header->topic_value[pub_header->topic_len] = 0;
    i += pub_header->topic_len;

    pub_header->prop_len = read_var_byte_integer(recvline, &i);

    remaning_length -= (i - *start_idx);
    pub_header->msg_len = remaning_length;

    pub_header->msg =
        malloc((pub_header->msg_len + 1) * sizeof(char));
    memcpy(pub_header->msg, &recvline[i], pub_header->msg_len);
    pub_header->msg[pub_header->msg_len] = 0;
    i += pub_header->msg_len;

    return pub_header;
}
