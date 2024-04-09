#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>


char* encode_UTF8_string(char string[], size_t* string_size) {

    char* encoded_string = (char*)malloc((*string_size + 2) * sizeof(char));

    encoded_string[0] = (*string_size >> 8) & 0xFF;
    encoded_string[1] = *string_size & 0xFF;


    for (int i = 0; i < *string_size; i++) {
        encoded_string[i + 2] = string[i];
    }
    *string_size += 2;

    return encoded_string;
}

char* decode_UTF8_string(char message[], int* current_position, size_t* topic_size) {
    int string_length = (message[*current_position] << 8) | message[*current_position + 1];
    char* string = (char*)malloc((string_length + 1) * sizeof(char));

    *current_position +=  2;

    for (int i = 0; i < string_length; i++) {
        string[i] = message[i + *current_position];
        //printf("%c", message[i + *current_position]);
    }
    printf("\n");

    *topic_size = string_length;
    *current_position += string_length;

    return string;
}

int decode_variable_byte_integer(char message[], size_t size, int *current_position) {

    int multiplier = 1;
    int value = 0;
    char encoded_byte;
    do {
        encoded_byte = message[*current_position];
        value += (encoded_byte & 127) * multiplier;
        if (multiplier > 128 * 128 * 128) {
            printf("malformed varaible byte integer");
            return -1;
        }
        multiplier *= 128;
        (*current_position)++;
    }
    while ((encoded_byte & 128) != 0);

    return value;  
}

void encode_variable_byte_integer(int number, char result[]) {
    char encoded_byte;
    int position = 0;
    do {
        encoded_byte = number % 128;
        number = number / 128;

        if (number > 0) {
            encoded_byte = encoded_byte | 128;
        }
        result[position] = encoded_byte;
        position++;
    } while (number > 0);
}

int calculate_variable_byte_length(int remaining_length_int) {
    int remaining_length_length; 
    for (int i = 1; i < 5; i++) {
        if (remaining_length_int <= pow(2, 7 * i - 1)) {
            remaining_length_length = i;
            break;
        }
    }

    return remaining_length_length;
}