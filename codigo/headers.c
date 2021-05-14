#include "headers.h"

#include <stdio.h>
#include <unistd.h>

FixedHeader *interpret_fixed_header(ustring recvline) {
    FixedHeader *header;
    header = malloc(sizeof(FixedHeader));
    header->type = recvline[0] >> 4;
    header->remaning_length = recvline[1];
    return header;
}

byte *encode_connack(int *encoded_len) {
    byte *encoded_str;
    int i = 0;
    char client_id[10];

    sprintf(client_id, "%d", getpid());

    /* first let's calculate the lengths of some properties */

    int remaning_length = 0;
    remaning_length += 1; // ack flags
    remaning_length += 1; // reason code
    remaning_length += 1; // prop length  (hardcoding 1 byte)
    remaning_length += 3; // topic_alias_maximum (id and value)
    remaning_length += 3; // Assigned Client ID (id and length)
    remaning_length += strlen(client_id);

    /* now let's create the encoded string */
    encoded_str = malloc((2 + remaning_length) * sizeof(byte));

    /* the first byte is the type and flags */
    encoded_str[i++] = CONNACK_PACKAGE << 4;

    /* the next byte is the byte for the remaning length */
    encoded_str[i++] = remaning_length;

    /* before the fixed header, we have the acknowledge flags */
    encoded_str[i++] = 0x00;

    /* the reason code */
    encoded_str[i++] = 0x00;

    /* the properties length */
    encoded_str[i++] = remaning_length - 3;

    /* now we write the topic alias maximum id */
    encoded_str[i++] = 0x22;
    /* and the value */
    encoded_str[i++] = 0x00;
    encoded_str[i++] = 0x0a;

    /* now we write the assigned client identifier */
    encoded_str[i++] = 0x12;
    /* the length: */
    encoded_str[i++] = 0x00;
    encoded_str[i++] = strlen(client_id);
    /* and the id */
    for (int j = 0; j < strlen(client_id); j++) encoded_str[i++] = client_id[j];

    *encoded_len = i;

    fprintf(stdout, "encoded_str: ");
    print_in_hex(encoded_str, *encoded_len);

    return encoded_str;
}

SubscribeHeader *interpret_subscribe_header(ustring recvline,
                                            int remaning_length) {
    SubscribeHeader *sub_header = malloc(sizeof(SubscribeHeader));
    int i = 2;

    sub_header->msg_id = (1 << 8) * recvline[i] + recvline[i + 1];
    i += 2;

    sub_header->prop_len = recvline[i++];

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

PublishHeader *interpret_publish_header(ustring recvline,
                                        int remaning_length, int n) {
    PublishHeader *pub_header = malloc(sizeof(PublishHeader));
    int i = 2;

    pub_header->topic_len = (1 << 8) * recvline[i] + recvline[i + 1];
    i += 2;

    pub_header->topic_value =
        malloc((pub_header->topic_len + 1) * sizeof(char));
    memcpy(pub_header->topic_value, &recvline[i], pub_header->topic_len);
    pub_header->topic_value[pub_header->topic_len] = 0;
    i += pub_header->topic_len;

    pub_header->prop_len = recvline[i++];

    pub_header->msg_len = remaning_length - (i-2);

    pub_header->msg = malloc((pub_header->msg_len + 1) * sizeof(char));
    memcpy(pub_header->msg, &recvline[i], pub_header->msg_len);
    pub_header->msg[pub_header->msg_len] = 0;
    i += pub_header->msg_len;

    pub_header->recvline_len = 2 + remaning_length;
    pub_header->recvline = malloc(pub_header->recvline_len * sizeof(uchar));
    memcpy(pub_header->recvline, recvline, pub_header->recvline_len);

    fprintf(stdout, "encoded_str: ");
    print_in_hex(pub_header->recvline, pub_header->recvline_len);

    return pub_header;
}
