#pragma once

#include <stddef.h>

/*
 * Offset-cursor helpers for Debug: where the next snprintf writes, and how much
 * room is left, so Debug fills the caller's buffer in one pass and returns the
 * would-be length (the buffer-size contract) without intermediate allocations.
 * Defined once in src/cerive.c.
 */

size_t cerive_buf_remaining(size_t cap, int off);
char * cerive_buf_at(char * buf, size_t cap, int off);
