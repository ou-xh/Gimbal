#ifndef RC_PROTOCOL_H
#define RC_PROTOCOL_H

#include <stdint.h>
#include "rc_config.h"

typedef enum
{
    RC_DECODE_OK = 0,
    RC_DECODE_BAD_ARGUMENT,
    RC_DECODE_BAD_LENGTH,
    RC_DECODE_BAD_CHANNEL,
    RC_DECODE_BAD_SWITCH
} RcDecodeResult;

typedef struct
{
    uint16_t channel[RC_CHANNEL_COUNT];
    uint8_t switch_left;
    uint8_t switch_right;
} RcInput;



/* Pure protocol functions: no HAL, clock, UART or global state. */
RcDecodeResult RcDecodeFrame(const uint8_t *data, uint16_t length, RcInput *input);

#endif
