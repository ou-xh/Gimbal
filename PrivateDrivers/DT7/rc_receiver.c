#include "rc_receiver.h"
#include "usart.h"

static uint8_t rc_dma_buffer[RC_DMA_BUFFER_LENGTH];
static volatile uint32_t rx_generation;
static volatile uint32_t frame_generation;
static volatile uint8_t rx_recovering;
static uint32_t last_attempt_tick;

volatile uint8_t rc_frame[RC_FRAME_LENGTH];
volatile uint32_t rc_frame_count, rc_frame_tick, rc_processed_count;
volatile uint16_t rc_last_size;
volatile HAL_StatusTypeDef rc_rx_status = HAL_ERROR;
volatile HAL_StatusTypeDef rc_uart_start_status = HAL_ERROR;
volatile HAL_StatusTypeDef rc_uart_abort_status = HAL_OK;
volatile uint32_t rc_uart_error_count, rc_uart_last_error;
volatile uint32_t rc_uart_recovery_count, rc_rx_event_count;
volatile uint32_t rc_length_error_count, rc_skipped_frame_count;
volatile uint32_t rc_uart_last_error_tick;
volatile uint8_t rc_uart_restart_pending;

static void RcRequestRecovery(void)
{
    uint32_t irq_state = __get_PRIMASK();
    __disable_irq();
    rx_generation++;
    rc_uart_restart_pending = 1U;
    __set_PRIMASK(irq_state);
}

void RcReceiverStart(void)
{
    HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle_DMA(
        &huart1, rc_dma_buffer, sizeof(rc_dma_buffer));
    rc_rx_status = rc_uart_start_status = status;
    if (status == HAL_OK)
        __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
    else
        RcRequestRecovery();
}

uint8_t RcReceiverGetState(uint32_t *generation)
{
    uint32_t irq_state = __get_PRIMASK();
    uint8_t ready;
    __disable_irq();
    if (generation != 0) *generation = rx_generation;
    ready = (rc_uart_restart_pending == 0U && rx_recovering == 0U);
    __set_PRIMASK(irq_state);
    return ready;
}

uint8_t RcReceiverIsCurrent(uint32_t generation)
{
    uint32_t current;
    uint8_t ready = RcReceiverGetState(&current);
    return ready && current == generation;
}

uint8_t RcReceiverReadLatest(RcReceivedFrame *frame)
{
    uint32_t irq_state;
    uint32_t count;
    uint32_t unread;
    uint8_t available = 0U;
    if (frame == 0) return 0U;
    irq_state = __get_PRIMASK();
    __disable_irq();
    count = rc_frame_count;
    unread = count - rc_processed_count;
    if (unread != 0U)
    {
        /* This is a latest-value mailbox, not a queue of old commands. */
        if (!rc_uart_restart_pending && !rx_recovering && frame_generation == rx_generation)
        {
            for (uint8_t i = 0; i < RC_FRAME_LENGTH; i++) frame->data[i] = rc_frame[i];
            frame->received_tick = rc_frame_tick;
            frame->generation = frame_generation;
            rc_skipped_frame_count += unread - 1U;
            available = 1U;
        }
        else
        {
            rc_skipped_frame_count += unread;
        }
        rc_processed_count = count;
    }
    __set_PRIMASK(irq_state);
    return available;
}

void RcReceiverServiceRecovery(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t irq_state;
    uint32_t recovery_generation;
    if (!rc_uart_restart_pending || (uint32_t)(now - last_attempt_tick) < RC_RECOVERY_PERIOD_MS)
        return;
    last_attempt_tick = now;

    irq_state = __get_PRIMASK();
    __disable_irq();
    rx_recovering = 1U;
    rx_generation++;
    recovery_generation = rx_generation;
    rc_skipped_frame_count += rc_frame_count - rc_processed_count;
    rc_processed_count = rc_frame_count;
    __set_PRIMASK(irq_state);

    /* HAL operations run with the caller's original interrupt state. */
    rc_rx_status = rc_uart_abort_status = HAL_UART_AbortReceive(&huart1);
    if (rc_uart_abort_status == HAL_OK)
    {
        __HAL_UART_CLEAR_OREFLAG(&huart1);
        RcReceiverStart();
    }

    irq_state = __get_PRIMASK();
    __disable_irq();
    /* Never erase an error raised by an interrupt during this recovery. */
    if (rc_uart_abort_status == HAL_OK && rc_uart_start_status == HAL_OK &&
        rx_generation == recovery_generation)
    {
        rc_uart_restart_pending = 0U;
        rc_uart_recovery_count++;
    }
    rx_recovering = 0U;
    __set_PRIMASK(irq_state);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if (huart != &huart1) return;
    rc_rx_event_count++;
    rc_last_size = size;
    if (rx_recovering || rc_uart_restart_pending) return;
    if (size == RC_FRAME_LENGTH)
    {
        for (uint8_t i = 0; i < RC_FRAME_LENGTH; i++) rc_frame[i] = rc_dma_buffer[i];
        rc_frame_tick = HAL_GetTick();
        frame_generation = rx_generation;
        rc_frame_count++;
    }
    else
    {
        rc_length_error_count++;
    }
    RcReceiverStart();
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart != &huart1) return;
    rc_uart_last_error = huart->ErrorCode;
    rc_uart_last_error_tick = HAL_GetTick();
    rc_uart_error_count++;
    RcRequestRecovery();
}
