#include "rc_protocol.h"

RcDecodeResult RcDecodeFrame(const uint8_t *data, uint16_t length, RcInput *input)
{
    RcInput decoded;
    if (data == 0 || input == 0) return RC_DECODE_BAD_ARGUMENT;
    if (length != RC_FRAME_LENGTH) return RC_DECODE_BAD_LENGTH;

    decoded.channel[0] = (data[0] | ((uint16_t)data[1] << 8)) & 0x07FFU;
    decoded.channel[1] = ((data[1] >> 3) | ((uint16_t)data[2] << 5)) & 0x07FFU;
    decoded.channel[2] = ((data[2] >> 6) | ((uint16_t)data[3] << 2) |
                          ((uint16_t)data[4] << 10)) & 0x07FFU;
    decoded.channel[3] = ((data[4] >> 1) | ((uint16_t)data[5] << 7)) & 0x07FFU;
    for (uint8_t i = 0; i < RC_CHANNEL_COUNT; i++)
    {
        if (decoded.channel[i] < RC_CHANNEL_MIN || decoded.channel[i] > RC_CHANNEL_MAX)
            return RC_DECODE_BAD_CHANNEL;
    }
    decoded.switch_left = (data[5] >> 6) & 0x03U;
    decoded.switch_right = (data[5] >> 4) & 0x03U;
    if (decoded.switch_left == 0U || decoded.switch_right == 0U)
        return RC_DECODE_BAD_SWITCH;

    *input = decoded;
    return RC_DECODE_OK;
}
