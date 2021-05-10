#ifndef HEADERS_H
#define HEADERS_H

#include <sys/types.h>
#include "util.h"

enum PackageType {
    CONNECT_PACKAGE = 0x1,
    CONNACK_PACKAGE = 0x2,
    PUBLISH_PACKAGE = 0x3,
    PUBACK_PACKAGE = 0x4,
    PUBREC_PACKAGE = 0x5,
    PUBREL_PACKAGE = 0x6,
    PUBCOMP_PACKAGE = 0x7,
    SUBSCRIBE_PACKAGE = 0x8,
    SUBACK_PACKAGE = 0x9,
    UNSUBSCRIBE_PACKAGE = 0xa,
    UNSUBACK_PACKAGE = 0xb,
    PINGREQ_PACKAGE = 0xc,
    PINGRESP_PACKAGE = 0xd,
    DISCONNECT_PACKAGE = 0xe
};

typedef struct {
    enum PackageType type;
    var_byte remaning_length;
} FixedHeader;

typedef struct {
    u_int8_t ack_flags;
    u_int8_t reason_code;
    var_byte property_length;
    u_int16_t topic_alias_maximum_value;
    u_int16_t client_id_len;
    u_int64_t client_id;
    u_int16_t receive_maximum_value;
} ConnackVarHeader;

FixedHeader *interpret_fixed_header(string recvline, int *start_idx);

#endif /* ifndef HEADERS_H */
