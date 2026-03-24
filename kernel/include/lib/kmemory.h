#ifndef KMEMORY_LIB_H
#define KMEMORY_LIB_H

#include <stddef.h>

void *kmemset(void *dest, int value, size_t count);
void *kmemcpy(void *dest, const void *src, size_t count);

#endif
