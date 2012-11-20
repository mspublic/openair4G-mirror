#ifndef TIMER_MESSAGES_TYPES_H_
#define TIMER_MESSAGES_TYPES_H_

typedef struct {
    uint32_t expiry_ms;
    uint32_t timer_id;
} TimerNewRequest, TimerHasExpired;

typedef struct {
    uint32_t timer_id;
} TimerRemove;

#endif /* TIMER_MESSAGES_TYPES_H_ */
