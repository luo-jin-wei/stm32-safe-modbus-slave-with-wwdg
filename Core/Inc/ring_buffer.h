#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stddef.h>

#define RB_SIZE 256

typedef struct {
		uint8_t buffer[RB_SIZE];
		volatile uint16_t head;
		volatile uint16_t tail;
} ring_buffer_t;

void rb_init(ring_buffer_t *rb);
uint8_t rb_put(ring_buffer_t *rb, uint8_t data);
uint8_t rb_get(ring_buffer_t *rb, uint8_t *data);
uint16_t rb_used(ring_buffer_t *rb);
uint16_t rb_free(ring_buffer_t *rb);
void rb_reset(ring_buffer_t *rb);

#endif

