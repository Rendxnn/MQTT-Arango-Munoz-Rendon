#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "fixedHeader.h"
#include "encoder.h"


char* build_publish(size_t* publish_size) {
    char packet_type_flags = 0b00110000;

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
    while(getchar() != '\n');
    char topic_name[65536];
    printf("topic name: (65536 char max)\n");
    fgets(topic_name, sizeof(topic_name), stdin);
    if (topic_name[strlen(topic_name) - 1] == '\n') {
        topic_name[strlen(topic_name) - 1] = '\0';
    }

    size_t topic_name_size = strlen(topic_name);
    printf("topic_name_size %ld \n", topic_name_size);


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
    while(getchar() != '\n');
    char message[65536];
    printf("message: (65536 char max)\n");
    fgets(message, 65536, stdin);
    if (message[strlen(message) - 1] == '\n') {
        message[strlen(message) - 1] = '\0';
    }

    size_t message_size = strlen(message);
    printf("message_size %ld \n", message_size);

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

    char* built_publish_message = (char*)malloc((remaining_length_int + 1 + remaining_length_length) * sizeof(char));

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


char* build_puback(int packet_identifier) {
    char puback_fixed_header = 0b01000000;
    char puback_remaining_length = 0b00000010;

    char packet_identifier_bytes[2];
    packet_identifier_bytes[0] = (packet_identifier >> 8) & 0xFF;
    packet_identifier_bytes[1] = packet_identifier & 0xFF;

    char* built_puback = (char*)malloc((4) * sizeof(char));

    built_puback[0] = puback_fixed_header;
    built_puback[1] = puback_remaining_length;
    built_puback[2] = packet_identifier_bytes[0];
    built_puback[3] = packet_identifier_bytes[1];

    return built_puback;

}


char* read_publish(char message[], size_t size, int current_position, unsigned char flags, char* payload_message, size_t* payload_message_length, char* topic, size_t* topic_size) {
    unsigned char retain = flags & 0x01;
    unsigned char QoS = (flags >> 1) & 0x03;
    unsigned char DUP = (flags >> 3) & 0x01;

    printf("Retain: %d\n", retain);
    printf("QoS: %d\n", QoS);
    printf("Duplicate: %d\n", DUP);

    char* topic_decoded = decode_UTF8_string(message, &current_position, topic_size);
    for (int i = 0; i < *topic_size; i++) {
        topic[i] = topic_decoded[i];
    }

    int packet_identifier = (message[current_position] << 8) | message[current_position + 1];
    current_position += 2;
    printf("packertidentifier %d\n", packet_identifier);

    char* payload_message_decoded = decode_UTF8_string(message, &current_position, payload_message_length);
    printf("desde read_publish\n");
    for (int i = 0; i < *payload_message_length; i++){
        printf("%c", payload_message_decoded[i]);
        payload_message[i] = payload_message_decoded[i];
    }
    printf("\n");

    char* puback = build_puback(packet_identifier);

    return puback;
}


