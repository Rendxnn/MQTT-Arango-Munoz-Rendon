#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "fixedHeader.h"
#include "encoder.h"


char* build_publish(size_t* publish_size) {
    char packet_type_flags = 0b0011000;

    int retain_flag;
    printf("retain flag: \n");
    scanf("%d", &retain_flag);

    int QoS_flag;
    printf("Qos flag: \n");
    scanf("%d", &QoS_flag);
    if (QoS_flag == 1) {
        packet_type_flags = packet_type_flags | 0b00000010;
    }
    else if (QoS_flag == 2) {
        packet_type_flags = packet_type_flags | 0b00000100;
    }

    if (retain_flag) {
        packet_type_flags = packet_type_flags | 0b00000001;
    }

    //pending remaining length
    //VARIABLE HEADER

    char topic_name[65536];
    printf("topic name: (65536 char max)\n");
    scanf("%s", topic_name);

    size_t topic_name_size = strlen(topic_name);

    char* encoded_topic_name = encode_UTF8_string(topic_name, &topic_name_size);
    
    unsigned char packet_identifier[2];

    if (QoS_flag != 0) {
        int packet_identifier_int;
        printf("packet identifier: (65535 max)\n");
        scanf("%d", &packet_identifier_int);

        packet_identifier[0] = (packet_identifier_int >> 8) & 0xFF;
        packet_identifier[1] = packet_identifier_int & 0xFF;

    }

    //PAYLOAD

    char message[65536];
    printf("message: (65536 char max)\n");
    scanf("%s", message);

    size_t message_size = strlen(message);

    char* encoded_message = encode_UTF8_string(message, &message_size);

    int remaining_length_int;

    if (QoS_flag) {
        remaining_length_int = topic_name_size + sizeof(packet_identifier) + message_size;
    }
    else {
        remaining_length_int = topic_name_size  + message_size;
    }

    int remaining_length_length = calculate_variable_byte_length(remaining_length_int);

    char remaining_length[remaining_length_length];

    encode_variable_byte_integer(remaining_length_int, remaining_length);

    char* built_publish_message = (char*)malloc((remaining_length_int + 1) * sizeof(char));

    built_publish_message[0] = packet_type_flags;

    int current_position = 1;

    for (int i = 0; i < remaining_length_length; i++) {
        built_publish_message[current_position + i] = remaining_length[i];
    }
    current_position += remaining_length_length;

    for (int i = 0; i < topic_name_size; i++) {
        built_publish_message[current_position + i] = encoded_topic_name[i];
    }
    current_position += topic_name_size;

    if (QoS_flag) {
        for (int i = 0; i < 2; i++) {
            built_publish_message[current_position + i] = packet_identifier[i];
        }
        current_position += 2;
    }

    for (int i = 0; i < message_size; i++) {
        built_publish_message[current_position + i] = encoded_message[i];
    }
    current_position += message_size;

    *publish_size = remaining_length_int + remaining_length_length + 1;

    return built_publish_message;
}
