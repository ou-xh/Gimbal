#ifndef RC_TASK_H
#define RC_TASK_H

#include <stdint.h>
#include "rc_config.h"

typedef struct
{
    uint16_t channel[RC_CHANNEL_COUNT];
    int16_t yaw_input;
    int16_t pitch_input;
    float yaw_target_deg;
    float pitch_target_deg;
    uint32_t last_valid_tick;
    uint8_t online; /* Recent valid UART data; not a verified RF failsafe signal. */
} RcSnapshot;

extern volatile uint16_t rc_ch_raw[RC_CHANNEL_COUNT];
extern volatile int16_t rc_yaw_input, rc_pitch_input;
extern volatile float yaw_target_deg, pitch_target_deg;
extern volatile uint32_t rc_valid_count, rc_invalid_count;
extern volatile uint32_t rc_channel_error_count, rc_switch_error_count, rc_stale_frame_count;
extern volatile uint8_t rc_online;
extern volatile uint32_t rc_last_valid_tick, rc_timeout_count;

/* Call Init once after peripheral initialization; call Process from one owner.
 * This module is currently serviced by main(), not by an RTOS task.
 */
void RcTaskInit(void);
void RcTaskProcess(void);
void RcGetSnapshot(RcSnapshot *snapshot);

#endif
