#include "rc_task.h"
#include "rc_protocol.h"
#include "rc_mapping.h"
#include "rc_receiver.h"

volatile uint16_t rc_ch_raw[RC_CHANNEL_COUNT] = {1024, 1024, 1024, 1024};
volatile int16_t rc_yaw_input, rc_pitch_input;
volatile float yaw_target_deg, pitch_target_deg;
volatile uint32_t rc_valid_count, rc_invalid_count;
volatile uint32_t rc_channel_error_count, rc_switch_error_count, rc_stale_frame_count;
volatile uint8_t rc_online;
volatile uint32_t rc_last_valid_tick, rc_timeout_count;
static uint8_t seen_valid_frame;
static uint32_t accepted_generation;

static void RcCheckLink(uint32_t now)
{
    uint32_t generation;
    uint8_t ready = RcReceiverGetState(&generation);
    if (!ready || generation != accepted_generation) seen_valid_frame = 0U;
    if (!seen_valid_frame || (uint32_t)(now - rc_last_valid_tick) >= RC_TIMEOUT_MS)
    {
        if (rc_online && seen_valid_frame) rc_timeout_count++;
        rc_online = 0U;
        rc_yaw_input = 0;
        rc_pitch_input = 0;
        /* Keep the last target. Actual motor fault action belongs to control. */
    }
}

static void RcAcceptFrame(const RcReceivedFrame *frame)
{
    RcInput input;
    RcMappedAngles angles;
    RcDecodeResult result;
    uint32_t now = HAL_GetTick();
    uint32_t irq_state;
    if ((uint32_t)(now - frame->received_tick) >= RC_TIMEOUT_MS)
    {
        rc_stale_frame_count++;
        rc_invalid_count++;
        return;
    }
    result = RcDecodeFrame(frame->data, RC_FRAME_LENGTH, &input);
    if (result != RC_DECODE_OK)
    {
        rc_invalid_count++;
        if (result == RC_DECODE_BAD_CHANNEL) rc_channel_error_count++;
        if (result == RC_DECODE_BAD_SWITCH) rc_switch_error_count++;
        return;
    }

    /* Do floating-point work only once per new valid frame, outside the lock. */
    RcMapAngles(&input, &angles);
    irq_state = __get_PRIMASK();
    __disable_irq();
    now = HAL_GetTick();
    if (!RcReceiverIsCurrent(frame->generation))
    {
        rc_skipped_frame_count++;
        __set_PRIMASK(irq_state);
        return;
    }
    if ((uint32_t)(now - frame->received_tick) >= RC_TIMEOUT_MS)
    {
        rc_stale_frame_count++;
        rc_invalid_count++;
        __set_PRIMASK(irq_state);
        return;
    }
    for (uint8_t i = 0; i < RC_CHANNEL_COUNT; i++) rc_ch_raw[i] = input.channel[i];
    rc_yaw_input = angles.yaw_input;
    rc_pitch_input = angles.pitch_input;
    yaw_target_deg = angles.yaw_deg;
    pitch_target_deg = angles.pitch_deg;
    rc_last_valid_tick = frame->received_tick;
    accepted_generation = frame->generation;
    seen_valid_frame = 1U;
    rc_online = 1U;
    rc_valid_count++;
    __set_PRIMASK(irq_state);
}

void RcTaskInit(void)
{
    RcReceiverStart();
}

void RcTaskProcess(void)
{
    RcReceivedFrame frame;
    RcCheckLink(HAL_GetTick());
    RcReceiverServiceRecovery();
    RcCheckLink(HAL_GetTick());
    if (RcReceiverReadLatest(&frame)) RcAcceptFrame(&frame);
    RcCheckLink(HAL_GetTick());
}

void RcGetSnapshot(RcSnapshot *snapshot)
{
    uint32_t irq_state;
    if (snapshot == 0) return;
    irq_state = __get_PRIMASK();
    __disable_irq();
    for (uint8_t i = 0; i < RC_CHANNEL_COUNT; i++) snapshot->channel[i] = rc_ch_raw[i];
    snapshot->yaw_input = rc_yaw_input;
    snapshot->pitch_input = rc_pitch_input;
    snapshot->yaw_target_deg = yaw_target_deg;
    snapshot->pitch_target_deg = pitch_target_deg;
    snapshot->last_valid_tick = rc_last_valid_tick;
    snapshot->online = rc_online && RcReceiverIsCurrent(accepted_generation) &&
        (uint32_t)(HAL_GetTick() - rc_last_valid_tick) < RC_TIMEOUT_MS;
    if (!snapshot->online)
    {
        snapshot->yaw_input = 0;
        snapshot->pitch_input = 0;
    }
    __set_PRIMASK(irq_state);
}
