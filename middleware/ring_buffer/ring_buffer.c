#include "ring_buffer.h"

void rb_init(ring_buffer_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
}

uint8_t rb_put(ring_buffer_t *rb, uint8_t data)
{
    uint16_t next_head = (rb->head + 1) & (RB_SIZE - 1);  // 位运算替代%256
    if (next_head == rb->tail) {
        return 0;  // 缓冲区满
    }
    rb->buffer[rb->head] = data;  // 先写数据
    rb->head = next_head;          // 再更新指针（顺序至关重要）
    return 1;
}

uint8_t rb_get(ring_buffer_t *rb, uint8_t *data)
{
    if (rb->head == rb->tail) {
        return 0;  // 缓冲区空
    }
    *data = rb->buffer[rb->tail];  // 先读数据
    rb->tail = (rb->tail + 1) & (RB_SIZE - 1);  // 再更新指针
    return 1;
}

uint16_t rb_used(ring_buffer_t *rb)
{
    return (rb->head - rb->tail) & (RB_SIZE - 1);
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
