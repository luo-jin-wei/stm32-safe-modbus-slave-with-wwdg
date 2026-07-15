#include "ring_buffer.h"

void rb_init(ring_buffer_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
}

uint8_t rb_put(ring_buffer_t *rb, uint8_t data)
{
    uint16_t next_head = (rb->head + 1) % RB_SIZE;
    if (next_head == rb->tail) {
        return 0;  // 缓冲区满，丢包
    }
    rb->buffer[rb->head] = data;
    rb->head = next_head;
    return 1;  // 成功
}

uint8_t rb_get(ring_buffer_t *rb, uint8_t *data)
{
    if (rb->head == rb->tail) {
        return 0;  // 缓冲区空
    }
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % RB_SIZE;
    return 1;  // 成功
}

uint16_t rb_used(ring_buffer_t *rb)
{
    return (rb->head - rb->tail + RB_SIZE) % RB_SIZE;
}

uint16_t rb_free(ring_buffer_t *rb)
{
    return RB_SIZE - 1 - rb_used(rb);
}

void rb_reset(ring_buffer_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
}

