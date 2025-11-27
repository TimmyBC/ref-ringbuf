#include "ringbuf.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAP 128u
#define MAX_CAP 4096u

static void usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s [-c CAP]\n\n"
            "Reads bytes from stdin and prints the last CAP bytes captured.\n"
            "If input exceeds CAP, old bytes are dropped (overwrite-oldest behavior).\n\n"
            "Examples:\n"
            "  echo -n 'hello' | %s\n"
            "  cat file.bin | %s -c 256\n",
            prog, prog, prog);
}

static bool parse_cap(const char *s, size_t *out_cap) {
    if (!s || !out_cap) {
        return false;
    }

    errno = 0;
    char *end = NULL;
    unsigned long v = strtoul(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') {
        return false;
    }
    if (v == 0 || v > MAX_CAP) {
        return false;
    }

    *out_cap = (size_t)v;
    return true;
}

static void hexdump_line(size_t base, const uint8_t *buf, size_t n) {
    // Print: offset  hex bytes  ascii
    printf("%08zx  ", base);

    for (size_t i = 0; i < 16; i++) {
        if (i < n) {
            printf("%02x ", buf[i]);
        } else {
            printf("   ");
        }
        if (i == 7) {
            printf(" ");
        }
    }

    printf(" |");
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)buf[i];
        printf("%c", isprint(c) ? (char)c : '.');
    }
    for (size_t i = n; i < 16; i++) {
        printf(" ");
    }
    printf("|\n");
}

int main(int argc, char **argv) {
    size_t cap = DEFAULT_CAP;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "-c") == 0) {
            if (i + 1 >= argc || !parse_cap(argv[i + 1], &cap)) {
                fprintf(stderr, "Invalid capacity. Must be 1..%u\n", MAX_CAP);
                return 2;
            }
            i++;
            continue;
        }

        fprintf(stderr, "Unknown arg: %s\n", argv[i]);
        usage(argv[0]);
        return 2;
    }

    static uint8_t storage[MAX_CAP];
    RingBuf rb;
    if (!ringbuf_init(&rb, storage, cap)) {
        fprintf(stderr, "Failed to init ring buffer\n");
        return 2;
    }

    // Read stdin and keep only the last CAP bytes.
    int ch;
    while ((ch = getchar()) != EOF) {
        uint8_t b = (uint8_t)(unsigned char)ch;

        if (ringbuf_is_full(&rb)) {
            // Overwrite-oldest policy: drop one byte then push.
            uint8_t dropped;
            (void)ringbuf_pop(&rb, &dropped);
        }
        (void)ringbuf_push(&rb, b);
    }

    const size_t n = ringbuf_size(&rb);
    printf("Captured %zu byte(s) (cap=%zu)\n", n, ringbuf_capacity(&rb));

    size_t offset = 0;
    uint8_t line[16];

    while (!ringbuf_is_empty(&rb)) {
        const size_t want = (n - offset >= sizeof line) ? sizeof line : (n - offset);
        const size_t got = ringbuf_pop_many(&rb, line, want);
        if (got == 0) {
            break;
        }

        hexdump_line(offset, line, got);
        offset += got;
    }

    return 0;
}
