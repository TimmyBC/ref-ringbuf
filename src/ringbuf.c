#include "ringbuf.h"

static size_t wrap_inc(size_t idx, size_t cap) {
    // cap is assumed > 0
    idx++;
    if (idx == cap) {
        idx = 0;
    }
    return idx;
}

static bool is_initialized(const RingBuf *rb) {
    return rb && rb->data && rb->cap > 0;
}

bool ringbuf_init(RingBuf *rb, uint8_t *storage, size_t cap) {
    if (!rb || !storage || cap == 0) {
        return false;
    }

    rb->data = storage;
    rb->cap = cap;
    rb->size = 0;
    rb->head = 0;
    rb->tail = 0;
    return true;
}

void ringbuf_reset(RingBuf *rb) {
    if (!rb) {
        return;
    }
    rb->size = 0;
    rb->head = 0;
    rb->tail = 0;
}

size_t ringbuf_capacity(const RingBuf *rb) {
    return is_initialized(rb) ? rb->cap : 0;
}

size_t ringbuf_size(const RingBuf *rb) {
    return (rb ? rb->size : 0);
}

size_t ringbuf_free_space(const RingBuf *rb) {
    if (!is_initialized(rb)) {
        return 0;
    }
    return rb->cap - rb->size;
}

bool ringbuf_is_empty(const RingBuf *rb) {
    return (!rb || rb->size == 0);
}

bool ringbuf_is_full(const RingBuf *rb) {
    return (is_initialized(rb) && rb->size == rb->cap);
}

bool ringbuf_push(RingBuf *rb, uint8_t byte) {
    if (!is_initialized(rb) || rb->size == rb->cap) {
        return false;
    }

    rb->data[rb->tail] = byte;
    rb->tail = wrap_inc(rb->tail, rb->cap);
    rb->size++;
    return true;
}

bool ringbuf_pop(RingBuf *rb, uint8_t *out) {
    if (!is_initialized(rb) || !out || rb->size == 0) {
        return false;
    }

    *out = rb->data[rb->head];
    rb->head = wrap_inc(rb->head, rb->cap);
    rb->size--;
    return true;
}

bool ringbuf_peek(const RingBuf *rb, uint8_t *out) {
    if (!is_initialized(rb) || !out || rb->size == 0) {
        return false;
    }

    *out = rb->data[rb->head];
    return true;
}

size_t ringbuf_push_many(RingBuf *rb, const uint8_t *src, size_t n) {
    if (!is_initialized(rb) || (!src && n != 0)) {
        return 0;
    }

    size_t pushed = 0;
    while (pushed < n && rb->size < rb->cap) {
        rb->data[rb->tail] = src[pushed];
        rb->tail = wrap_inc(rb->tail, rb->cap);
        rb->size++;
        pushed++;
    }
    return pushed;
}

size_t ringbuf_pop_many(RingBuf *rb, uint8_t *dst, size_t n) {
    if (!is_initialized(rb) || (!dst && n != 0)) {
        return 0;
    }

    size_t popped = 0;
    while (popped < n && rb->size > 0) {
        dst[popped] = rb->data[rb->head];
        rb->head = wrap_inc(rb->head, rb->cap);
        rb->size--;
        popped++;
    }
    return popped;
}
