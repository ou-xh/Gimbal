#ifndef RC_CONFIG_H
#define RC_CONFIG_H

#define RC_FRAME_LENGTH        18U
#define RC_DMA_BUFFER_LENGTH   50U
#define RC_CHANNEL_COUNT       4U
#define RC_CHANNEL_CENTER      1024
#define RC_CHANNEL_SPAN        660
#define RC_CHANNEL_MIN         364U
#define RC_CHANNEL_MAX         1684U
#define RC_DEADZONE            10
#define RC_TIMEOUT_MS          100U
#define RC_RECOVERY_PERIOD_MS  20U

/* Channel assignment and direction do not change the wire protocol. */
#define RC_YAW_CHANNEL         0U
#define RC_PITCH_CHANNEL       1U
#define RC_YAW_DIRECTION       1
#define RC_PITCH_DIRECTION     1
#define RC_YAW_LIMIT_DEG        180.0f
#define RC_PITCH_LIMIT_DEG      30.0f

#endif
