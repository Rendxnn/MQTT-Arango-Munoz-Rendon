#include <stddef.h>
#ifndef PUBLISH_H
#define PUBLISH_H

char* build_publish(size_t* publish_size);
char* read_publish(char message[], size_t size, int current_position, unsigned char flags, char* payload_message, size_t* payload_message_length, char* topic, size_t* topic_size);


#endif