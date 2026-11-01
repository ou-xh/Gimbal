#include "rc_mapping.h"

_Static_assert(RC_YAW_CHANNEL < RC_CHANNEL_COUNT, "Invalid yaw channel");
_Static_assert(RC_PITCH_CHANNEL < RC_CHANNEL_COUNT, "Invalid pitch channel");
_Static_assert(RC_YAW_DIRECTION == 1 || RC_YAW_DIRECTION == -1, "Invalid yaw direction");
_Static_assert(RC_PITCH_DIRECTION == 1 || RC_PITCH_DIRECTION == -1, "Invalid pitch direction");

static int16_t RcCenterChannel(uint16_t raw)
{
    int16_t value = (int16_t)raw - RC_CHANNEL_CENTER;
    return (value > -RC_DEADZONE && value < RC_DEADZONE) ? 0 : value;
}

static float RcClamp(float value, float limit)
{
    if (value > limit) return limit;
    if (value < -limit) return -limit;
    return value;
}

void RcMapAngles(const RcInput *input, RcMappedAngles *angles)
{
    if (input == 0 || angles == 0) return;
    angles->yaw_input = RcCenterChannel(input->channel[RC_YAW_CHANNEL]) * RC_YAW_DIRECTION;
    angles->pitch_input = RcCenterChannel(input->channel[RC_PITCH_CHANNEL]) * RC_PITCH_DIRECTION;
    angles->yaw_deg = RcClamp(((float)angles->yaw_input / RC_CHANNEL_SPAN) * RC_YAW_LIMIT_DEG,
                             RC_YAW_LIMIT_DEG);
    angles->pitch_deg = RcClamp(((float)angles->pitch_input / RC_CHANNEL_SPAN) * RC_PITCH_LIMIT_DEG,
                               RC_PITCH_LIMIT_DEG);
}
