#include "ringbuf.h"

#include <stdio.h>
#include <string.h>

#define TEST_ASSERT(expr)                                                                  \
    do {                                                                                   \
        if (!(expr)) {                                                                     \
            fprintf(stderr, "[FAIL] %s:%d: %s\n", __FILE__, __LINE__, #expr);             \
            return 1;                                                                      \
        }                                                                                  \
    } while (0)

#define RUN_TEST(fn)                                                                       \
    do {                                                                                   \
        int rc = fn();                                                                     \
        if (rc != 0) {                                                                     \
            return rc;                                                                     \
        }                                                                                  \
        fprintf(stdout, "[OK] %s\n", #fn);                                                 \
    } while (0)

static int test_init_validation(void) {
    uint8_t storage[8] = {0};
    RingBuf rb;

    TEST_ASSERT(!ringbuf_init(NULL, storage, sizeof storage));
    TEST_ASSERT(!ringbuf_init(&rb, NULL, sizeof storage));
    TEST_ASSERT(!ringbuf_init(&rb, storage, 0));
    TEST_ASSERT(ringbuf_init(&rb, storage, sizeof storage));
    TEST_ASSERT(ringbuf_capacity(&rb) == sizeof storage);
    TEST_ASSERT(ringbuf_size(&rb) == 0);
    return 0;
}

static int test_push_pop_order(void) {
    uint8_t storage[4] = {0};
    RingBuf rb;
    TEST_ASSERT(ringbuf_init(&rb, storage, sizeof storage));

    TEST_ASSERT(ringbuf_push(&rb, 0x11));
    TEST_ASSERT(ringbuf_push(&rb, 0x22));
    TEST_ASSERT(ringbuf_push(&rb, 0x33));

    uint8_t x = 0;
    TEST_ASSERT(ringbuf_peek(&rb, &x));
    TEST_ASSERT(x == 0x11);

    TEST_ASSERT(ringbuf_pop(&rb, &x));
    TEST_ASSERT(x == 0x11);
    TEST_ASSERT(ringbuf_pop(&rb, &x));
    TEST_ASSERT(x == 0x22);
    TEST_ASSERT(ringbuf_pop(&rb, &x));
    TEST_ASSERT(x == 0x33);

    TEST_ASSERT(ringbuf_is_empty(&rb));
    TEST_ASSERT(!ringbuf_pop(&rb, &x));
    TEST_ASSERT(!ringbuf_peek(&rb, &x));
    return 0;
}

static int test_wraparound_behavior(void) {
    uint8_t storage[4] = {0};
    RingBuf rb;
    TEST_ASSERT(ringbuf_init(&rb, storage, sizeof storage));

    // Fill 3, pop 2, then push 2 more to force wrap.
    TEST_ASSERT(ringbuf_push(&rb, 1));
    TEST_ASSERT(ringbuf_push(&rb, 2));
    TEST_ASSERT(ringbuf_push(&rb, 3));

    uint8_t x;
    TEST_ASSERT(ringbuf_pop(&rb, &x) && x == 1);
    TEST_ASSERT(ringbuf_pop(&rb, &x) && x == 2);

    TEST_ASSERT(ringbuf_push(&rb, 4));
    TEST_ASSERT(ringbuf_push(&rb, 5));

    TEST_ASSERT(ringbuf_pop(&rb, &x) && x == 3);
    TEST_ASSERT(ringbuf_pop(&rb, &x) && x == 4);
    TEST_ASSERT(ringbuf_pop(&rb, &x) && x == 5);

    TEST_ASSERT(ringbuf_is_empty(&rb));
    return 0;
}

static int test_full_behavior_does_not_mutate(void) {
    uint8_t storage[2] = {0};
    RingBuf rb;
    TEST_ASSERT(ringbuf_init(&rb, storage, sizeof storage));

    TEST_ASSERT(ringbuf_push(&rb, 0xA0));
    TEST_ASSERT(ringbuf_push(&rb, 0xB0));
    TEST_ASSERT(ringbuf_is_full(&rb));

    // Snapshot state.
    RingBuf before = rb;

    TEST_ASSERT(!ringbuf_push(&rb, 0xC0));

    // State should be unchanged.
    TEST_ASSERT(rb.data == before.data);
    TEST_ASSERT(rb.cap == before.cap);
    TEST_ASSERT(rb.size == before.size);
    TEST_ASSERT(rb.head == before.head);
    TEST_ASSERT(rb.tail == before.tail);

    uint8_t x;
    TEST_ASSERT(ringbuf_pop(&rb, &x) && x == 0xA0);
    TEST_ASSERT(ringbuf_pop(&rb, &x) && x == 0xB0);
    return 0;
}

static int test_push_pop_many(void) {
    uint8_t storage[8] = {0};
    RingBuf rb;
    TEST_ASSERT(ringbuf_init(&rb, storage, sizeof storage));

    uint8_t in[] = {10, 11, 12, 13, 14};
    TEST_ASSERT(ringbuf_push_many(&rb, in, sizeof in) == sizeof in);
    TEST_ASSERT(ringbuf_size(&rb) == sizeof in);

    uint8_t out[5] = {0};
    TEST_ASSERT(ringbuf_pop_many(&rb, out, sizeof out) == sizeof out);
    TEST_ASSERT(memcmp(in, out, sizeof in) == 0);
    TEST_ASSERT(ringbuf_is_empty(&rb));

    // pop_many on empty should return 0
    TEST_ASSERT(ringbuf_pop_many(&rb, out, sizeof out) == 0);
    return 0;
}

int main(void) {
    RUN_TEST(test_init_validation);
    RUN_TEST(test_push_pop_order);
    RUN_TEST(test_wraparound_behavior);
    RUN_TEST(test_full_behavior_does_not_mutate);
    RUN_TEST(test_push_pop_many);

    fprintf(stdout, "All tests passed.\n");
    return 0;
}
