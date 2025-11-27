# reference-ringbuf

A small, dependency-free reference project showing how to structure a tiny C module with:
- clean header/source separation
- simple, explicit function contracts
- a tiny test suite (no external frameworks)
- a small example application

The actual module is a fixed-capacity **ring buffer for bytes**. This is a common building block in embedded work (UART RX/TX buffering, simple producer/consumer queues), and it’s a great excuse to practice “professional C” without needing any hardware.

This repo is meant as a *reference implementation* you can read and imitate. It’s intentionally boring. Boring code tends to survive longer.

---

## Repository layout

```
include/     Public headers (the module's public surface)
src/         Implementations
app/         Example program(s)
test/        Small unit tests (no external frameworks)
build/       Build outputs (generated)
Makefile     Build entry point
```

- **The module**
  - `include/ringbuf.h`
  - `src/ringbuf.c`

- **Example app**
  - `app/ringdump.c`

- **Tests**
  - `test/test_ringbuf.c`

---

## What the ring buffer does

`RingBuf` stores bytes in a fixed-size circular array.

- **No heap allocation**: the caller supplies the storage.
- **O(1)** push/pop operations.
- Tracks `size` explicitly, so **empty** and **full** are unambiguous.
- **Not thread-safe** by default. If you use it between an ISR and main (or between threads), you must add proper synchronization.

This module is intentionally byte-oriented (`uint8_t`) to keep the example focused.

---

## Building

Requirements:
- Any C11 compiler (GCC/Clang/etc.)
- `make`

Build the example app:

```bash
make
```

This produces:

- `build/ringdump`

Run:

```bash
echo -n 'hello' | ./build/ringdump
```

Choose a capacity (number of bytes to retain):

```bash
cat file.bin | ./build/ringdump -c 256
```

> `ringdump` keeps only the **last N bytes** of the input. If input length exceeds N, older bytes are dropped. That overwrite policy is implemented in the *app*, not in the ring buffer module.

---

## Running tests

```bash
make test
```

This builds and runs:

- `build/test_ringbuf`

If everything is correct, it prints `All tests passed.`

---

## Optional: build with sanitizers

Sanitizers are not required, but they’re excellent during learning. They catch a lot of mistakes early.

```bash
make clean
make SAN='-fsanitize=address,undefined'
make test SAN='-fsanitize=address,undefined'
```

---

## API summary

Full contracts are in `include/ringbuf.h`. The key operations are:

- Initialize and reset:
  - `bool ringbuf_init(RingBuf *rb, uint8_t *storage, size_t cap);`
  - `void ringbuf_reset(RingBuf *rb);`

- Single-element operations:
  - `bool ringbuf_push(RingBuf *rb, uint8_t byte);`
  - `bool ringbuf_pop(RingBuf *rb, uint8_t *out);`
  - `bool ringbuf_peek(const RingBuf *rb, uint8_t *out);`

- Bulk operations:
  - `size_t ringbuf_push_many(RingBuf *rb, const uint8_t *src, size_t n);`
  - `size_t ringbuf_pop_many(RingBuf *rb, uint8_t *dst, size_t n);`

- Helpers:
  - `size_t ringbuf_size(const RingBuf *rb);`
  - `size_t ringbuf_capacity(const RingBuf *rb);`
  - `size_t ringbuf_free_space(const RingBuf *rb);`
  - `bool ringbuf_is_empty(const RingBuf *rb);`
  - `bool ringbuf_is_full(const RingBuf *rb);`

Return/value conventions:
- Functions returning `bool` return `true` on success, `false` on invalid inputs or when the operation cannot be performed (e.g., push into a full buffer).
- Bulk functions return the number of bytes actually pushed/popped.

---

## Design notes (why it’s written this way)

- **No modulo arithmetic**: index wrapping uses a simple increment + compare. It’s easier to understand and debug than `% cap` while learning.
- **Explicit `size` tracking**: avoids the classic “keep one slot empty” trick and makes full/empty checks straightforward.
- **Strong-ish behavior on failure**: `ringbuf_push` fails cleanly when full without changing the buffer.
- **No hidden dependencies**: plain C, no third-party frameworks.

---

## License

MIT License is recommended if you want this to be straightforward to reuse. If you publish this repo, add a `LICENSE` file with MIT text and put your name in it.
