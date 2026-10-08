#pragma once

#include <stddef.h>

size_t cerive_buf_remaining(size_t const cap, int const off);

char * cerive_buf_at(size_t const cap, char buf[const cap], int const off);
