#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include "fixedHeader.h"
#include "encoder.h"
#include <string.h>

char* build_subscribe(size_t* subscribe_size) {
    char fixed_header_type = 0b10000010;


    //variable header
    int packet_identifier_int;
    printf("packet identifier: (65535 max)\n");
    scanf("%d", &packet_identifier_int);


    char packet_identifier[2];
    packet_identifier[0] = (packet_identifier_int >> 8) & 0xFF;
    packet_identifier[1] = packet_identifier_int & 0xFF;


    //payload
    
    char topic[101];
    int QoS;
    size_t topic_size;    

    printf("topic (100 max size): ");
    scanf("%100s", topic);

    topic_size = strlen(topic);
    char* encoded_topic = encode_UTF8_string(topic, &topic_size);

    printf("QoS: ");
    scanf("%d", &QoS);

    int remaining_length_int = 2 + strlen(topic) + 2;
    int remaining_length_length = calculate_variable_byte_length(remaining_length_int);
    char remaining_length[remaining_length_length];

    encode_variable_byte_integer(remaining_length_int, remaining_length);

    char* built_subscribe = (char*)malloc((remaining_length_int + 1 + remaining_length_length) * sizeof(char));

    built_subscribe[0] = fixed_header_type;
    int current_position = 1;

    for (int i = 0; i < remaining_length_length; i++) {
        built_subscribe[current_position + i] = remaining_length[i];
    }
    current_position += remaining_length_length;

    for (int i = 0; i < strlen(topic); i++) {
        built_subscribe[current_position] = topic[i];
        current_position++;
    }

    built_subscribe[current_position] = QoS;

    *subscribe_size = remaining_length_int + remaining_length_length + 1;
    return built_subscribe;
}



char* build_suback(size_t* suback_size, int packet_identifier, int subscriptions) {
    char fixed_header_type = 0b10010000;
    
    //VARIABLE HEADER
    char packet_identifier_bytes[2];
    packet_identifier_bytes[0] = (packet_identifier >> 8) & 0xFF;
    packet_identifier_bytes[1] = packet_identifier & 0xFF;
    
    //PAYLOAD
    int remaining_length_int = subscriptions + 2;

    int remaining_length_length = calculate_variable_byte_length(remaining_length_int);

    char remaining_length[remaining_length_length];

    encode_variable_byte_integer(remaining_length_int, remaining_length);

    char* built_suback = (char*)malloc((remaining_length_int + 1) * sizeof(char));

    built_suback[0] = fixed_header_type;

    int current_position = 1;

    for (int i = 0; i < remaining_length_length; i++) {
        built_suback[current_position + i] = remaining_length[i];
    }
    current_position += remaining_length_length;

    built_suback[current_position] = packet_identifier_bytes[0];
    built_suback[current_position + 1] = packet_identifier_bytes[1];
    current_position += 2;

    for (int i = 0; i < subscriptions; i++) {
        built_suback[current_position] = 0;
        current_position++;
    }

    return built_suback;
    
}


char* read_subscribe(char message[], int current_position, int remaining_length, char clientid[], size_t* suback_size){
    int subscriptions = 0;
    int original_position = current_position;
    int packet_identifier = (message[current_position] << 8 ) | message[current_position + 1];
    current_position += 2;
    printf("topic: \n");
    while (current_position < remaining_length + original_position) {
        subscriptions++;
        int topic_name_size = (message[current_position] << 8 ) | message[current_position + 1];
        current_position += 2;
        char current_topic[topic_name_size];

        for (int i = 0; i < topic_name_size; i++) {
            current_topic[i] = message[current_position];
            printf("%c", current_topic[i]);
            current_position++;
        }
        printf("\n");
        int requested_QoS = message[current_position];
        current_position++;
        //subscribe_topic(current_topic, clientid);
        
    }
    char* suback;
    suback = build_suback(suback_size, packet_identifier, subscriptions);
    return suback;
    
}
  
  
  
  
  
  
  
  
