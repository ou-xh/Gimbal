#ifndef RC_MAPPING_H
#define RC_MAPPING_H

#include "rc_protocol.h"

typedef struct
{
    int16_t yaw_input;
    int16_t pitch_input;
    float yaw_deg;
    float pitch_deg;
} RcMappedAngles;

/* Application mapping: raw stick input to target angles in degrees. */
void RcMapAngles(const RcInput *input, RcMappedAngles *angles);

#endif
