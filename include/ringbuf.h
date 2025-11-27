#ifndef RINGBUF_H
#define RINGBUF_H

// A tiny fixed-capacity ring buffer for bytes.
//
// Design goals:
// - No heap allocation (caller supplies storage)
// - O(1) push/pop
// - Easy to reason about: explicit size tracking (no "keep one slot empty" trick)
//
// Typical embedded use: UART RX/TX buffering, log capture, producer/consumer queues.
//
// Thread-safety: none. If you use this between an ISR and main,
// ensure you protect shared state appropriately (e.g., disable interrupts
// briefly, or use atomics / critical sections).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *data;   // backing storage (owned by caller)
    size_t cap;      // capacity in bytes
    size_t size;     // number of bytes currently stored
    size_t head;     // read index (next byte returned by pop)
    size_t tail;     // write index (next slot written by push)
} RingBuf;

// ringbuf_init
// Initialize a ring buffer over caller-supplied storage.
// Returns false if inputs are invalid or cap == 0.
bool ringbuf_init(RingBuf *rb, uint8_t *storage, size_t cap);

// ringbuf_reset
// Clears the buffer (does not wipe storage contents).
void ringbuf_reset(RingBuf *rb);

// ringbuf_capacity
// Returns capacity in bytes (0 if rb is NULL or uninitialized).
size_t ringbuf_capacity(const RingBuf *rb);

// ringbuf_size
// Returns number of bytes currently stored (0 if rb is NULL).
size_t ringbuf_size(const RingBuf *rb);

// ringbuf_free_space
// Returns how many bytes can still be pushed.
size_t ringbuf_free_space(const RingBuf *rb);

// ringbuf_is_empty
bool ringbuf_is_empty(const RingBuf *rb);

// ringbuf_is_full
bool ringbuf_is_full(const RingBuf *rb);

// ringbuf_push
// Adds one byte to the tail.
// Returns false if the buffer is full or inputs are invalid.
bool ringbuf_push(RingBuf *rb, uint8_t byte);

// ringbuf_pop
// Removes one byte from the head and writes it to *out.
// Returns false if the buffer is empty or inputs are invalid.
bool ringbuf_pop(RingBuf *rb, uint8_t *out);

// ringbuf_peek
// Reads the next byte (head) without removing it.
// Returns false if empty or inputs invalid.
bool ringbuf_peek(const RingBuf *rb, uint8_t *out);

// ringbuf_push_many
// Push up to n bytes. Returns number of bytes actually pushed.
// Safe to call with n == 0.
size_t ringbuf_push_many(RingBuf *rb, const uint8_t *src, size_t n);

// ringbuf_pop_many
// Pop up to n bytes. Returns number of bytes actually popped.
// Safe to call with n == 0.
size_t ringbuf_pop_many(RingBuf *rb, uint8_t *dst, size_t n);

#ifdef __cplusplus
}
#endif

#endif // RINGBUF_H
