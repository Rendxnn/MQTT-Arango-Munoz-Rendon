#include <stddef.h>
#ifndef PUBLISH_H
#define PUBLISH_H

char* build_publish(size_t* publish_size);
char* read_publish(char message[], size_t size, int current_position, unsigned char flags);

#endif