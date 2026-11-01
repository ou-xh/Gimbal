#ifndef RC_RECEIVER_H
#define RC_RECEIVER_H

#include "main.h"
#include "rc_config.h"

typedef struct
{
    uint8_t data[RC_FRAME_LENGTH];
    uint32_t received_tick;
    uint32_t generation;
} RcReceivedFrame;

/* Retain existing diagnostic symbol names for debugger watch expressions. */
extern volatile uint8_t rc_frame[RC_FRAME_LENGTH];
extern volatile uint32_t rc_frame_count, rc_frame_tick, rc_processed_count;
extern volatile uint16_t rc_last_size;
extern volatile HAL_StatusTypeDef rc_rx_status;
extern volatile HAL_StatusTypeDef rc_uart_start_status, rc_uart_abort_status;
extern volatile uint32_t rc_uart_error_count, rc_uart_last_error;
extern volatile uint32_t rc_uart_recovery_count, rc_rx_event_count;
extern volatile uint32_t rc_length_error_count, rc_skipped_frame_count;
extern volatile uint32_t rc_uart_last_error_tick;
extern volatile uint8_t rc_uart_restart_pending;

void RcReceiverStart(void);
void RcReceiverServiceRecovery(void);
uint8_t RcReceiverReadLatest(RcReceivedFrame *frame);
uint8_t RcReceiverGetState(uint32_t *generation);
uint8_t RcReceiverIsCurrent(uint32_t generation);

#endif
