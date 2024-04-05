#include <stddef.h>
#include "fixedHeader.h"
#ifndef CONNECT_H
#define CONNECT_H

char* build_connect();
char* read_connect();
char* build_connack(size_t* size, char clean_start_flag);
int read_connack(struct fixed_header *response_fixed_header);

#endif