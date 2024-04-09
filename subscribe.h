#include <stddef.h>
#ifndef SUBSCRIBE_H
#define SUBSCRIBE_H

char* build_subscribe(size_t* subscribe_size);
char* build_suback(size_t* suback_size, int packet_identifier, int subscriptions);
char* read_subscribe(char message[], int current_position, int remaining_length, char clientid[], size_t* suback_size);

#endif