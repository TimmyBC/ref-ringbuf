# Simple Makefile for the reference ring buffer repo.
#
# Targets:
#   make        build app(s)
#   make test   build + run tests
#   make clean  remove build artifacts

CC      ?= cc
CSTD    ?= -std=c11
WARN    ?= -Wall -Wextra -Wpedantic -Werror
OPT     ?= -O0
DBG     ?= -g
SAN     ?=

CPPFLAGS += -Iinclude
CFLAGS   += $(CSTD) $(WARN) $(OPT) $(DBG) $(SAN)
LDFLAGS  += $(SAN)

BUILD_DIR := build
SRC_DIR   := src
APP_DIR   := app
TEST_DIR  := test

LIB_OBJS := $(BUILD_DIR)/ringbuf.o

APP_RINGDUMP := $(BUILD_DIR)/ringdump
TEST_RINGBUF := $(BUILD_DIR)/test_ringbuf

DEPS := $(LIB_OBJS:.o=.d)

.PHONY: all test clean

all: $(APP_RINGDUMP)

test: $(TEST_RINGBUF)
	@$(TEST_RINGBUF)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/ringbuf.o: $(SRC_DIR)/ringbuf.c include/ringbuf.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(APP_RINGDUMP): $(APP_DIR)/ringdump.c $(LIB_OBJS) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_RINGBUF): $(TEST_DIR)/test_ringbuf.c $(LIB_OBJS) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDFLAGS)

-include $(DEPS)

clean:
	@rm -rf $(BUILD_DIR)
