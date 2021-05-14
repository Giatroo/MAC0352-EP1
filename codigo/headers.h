#ifndef HEADERS_H
#define HEADERS_H

#include <sys/types.h>

#include "util.h"

enum PackageType {
    CONNECT_PACKAGE = 0x1,
    CONNACK_PACKAGE = 0x2,
    PUBLISH_PACKAGE = 0x3,
    SUBSCRIBE_PACKAGE = 0x8,
    PINGREQ_PACKAGE = 0xc,
    DISCONNECT_PACKAGE = 0xe
};

typedef struct {
    enum PackageType type;
    u_int64_t remaning_length;
} FixedHeader;

typedef struct {
    u_int16_t msg_id;
    u_int64_t prop_len;
    u_int16_t topic_len;
    string topic_value;
    u_int8_t sub_options;
} SubscribeHeader;

typedef struct {
    u_int16_t topic_len;
    string topic_value;
    u_int64_t prop_len;
    u_int64_t msg_len;
    string msg;
    int recvline_len;
    ustring recvline;
} PublishHeader;

FixedHeader *interpret_fixed_header(ustring recvline);
byte *encode_connack(int *encoded_len);
SubscribeHeader *interpret_subscribe_header(ustring recvline,
                                            int remaning_length);
PublishHeader *interpret_publish_header(ustring recvline, int remaning_length,
                                        int n);

#endif /* ifndef HEADERS_H */
