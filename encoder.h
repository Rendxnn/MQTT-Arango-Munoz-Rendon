#include <stddef.h>
#ifndef ENCODER_H
#define ENCODER_H

char* encode_UTF8_string(char string[], size_t* string_size);

char* decode_UTF8_string(char message[], int* current_position, size_t* topic_size);

int decode_variable_byte_integer(char message[], size_t size, int *current_position);

void encode_variable_byte_integer(int number, char result[]);

int calculate_variable_byte_length(int remaining_length_int);

#endif