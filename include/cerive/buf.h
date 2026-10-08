#pragma once

#include <stddef.h>

size_t cerive_buf_remaining(size_t cap, int off);

char * cerive_buf_at(char * buf, size_t cap, int off);
