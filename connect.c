#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "fixedHeader.h"
#include "encoder.h"


char* build_connack(size_t* size, char clean_start_flag){
    char connack_fixed_header = 0b00100000; 

    // variable header

    char connack_flags = 0;
    if (clean_start_flag) {
        connack_flags = 0b00000000;
    }
    else {
        connack_flags = 0b00000001;
    }

    char connack_reason_code = 0x00; // success


    //PAYLOAD
    // no payload xd


    int remaining_length_int = sizeof(connack_flags) + sizeof(connack_reason_code);

    int remaining_length_length; 
    for (int i = 1; i < 5; i++) {
        if (remaining_length_int < pow(2, 7 * i - 1)) {
            remaining_length_length = i;
            break;
        }
    }


    char remaining_length[remaining_length_length];



    encode_variable_byte_integer(remaining_length_int, remaining_length);

    char* built_connack_message = (char*)malloc((remaining_length_int + 1) * sizeof(char));


    built_connack_message[0] = connack_fixed_header;
    for (int i = 0; i < remaining_length_length; i++) {
        built_connack_message[i + 1] = remaining_length[i]; 
    }


    built_connack_message[remaining_length_length + 2] = connack_flags;
    built_connack_message[remaining_length_length + 3] = connack_reason_code;

    *size = remaining_length_int + remaining_length_length + 1;


    return built_connack_message;

}

char* build_connect(size_t* connect_size) {

    char packet_type = 0b00010000;

    char protocol_name[] = {0b00000000, 0b00000100, 'M', 'Q', 'T', 'T'};
    char protocol_level = 0b00000101;
    char connect_flags = 0b11000010;
    char keep_alive[] = {0b00000000, 0b00111100};

    char client_id[65535];
    char username[65535];
    char password[65535];

    printf("Ingrese el client_id: ");
    scanf("%s", client_id); 
    
    printf("Ingrese su nombre de usuario: ");
    scanf("%s", username); 
    
    printf("Ingrese su contraseña: ");
    scanf("%s", password); 

    size_t username_length = strlen(username);
    size_t password_length = strlen(password);
    size_t client_id_length = strlen(client_id);

    unsigned char username_length_bytes[2];
    unsigned char password_length_bytes[2];
    unsigned char client_id_length_bytes[2];

    username_length_bytes[0] = (username_length >> 8) & 0xFF;
    username_length_bytes[1] = username_length & 0xFF;

    password_length_bytes[0] = (password_length >> 8) & 0xFF;
    password_length_bytes[1] = password_length & 0xFF;

    client_id_length_bytes[0] = (client_id_length >> 8) & 0xFF;
    client_id_length_bytes[1] = client_id_length & 0xFF;

    char username_bytes[username_length + 2];
    char password_bytes[password_length + 2];
    char client_id_bytes[client_id_length + 2];

    username_bytes[0] = username_length_bytes[0];
    username_bytes[1] = username_length_bytes[1];

    for(size_t i = 0; i < username_length; i++) {
        username_bytes[i + 2] = username[i];
    }

    password_bytes[0] = password_length_bytes[0];
    password_bytes[1] = password_length_bytes[1];

    for(size_t i = 0; i < password_length; i++) {
        password_bytes[i + 2] = password[i];
    }

    client_id_bytes[0] = client_id_length_bytes[0];
    client_id_bytes[1] = client_id_length_bytes[1];

    for(size_t i = 0; i < client_id_length; i++) {
        client_id_bytes[i + 2] = client_id[i];
    }

    int remaining_length_int = sizeof(protocol_name) + sizeof(protocol_level) + sizeof(connect_flags) + sizeof(keep_alive)  + sizeof(username_bytes) + sizeof(password_bytes) + sizeof(client_id_bytes);

    int remaining_length_length = calculate_variable_byte_length(remaining_length_int);

    char remaining_length[remaining_length_length];

    encode_variable_byte_integer(remaining_length_int, remaining_length);

    char* built_connect_message = (char*)malloc((remaining_length_int + 1) * sizeof(char));

    built_connect_message[0] = packet_type;

    for (int i = 0; i < remaining_length_length; i++) {
        built_connect_message[i + 1] = remaining_length[i]; 
    }

    int current_position = remaining_length_length + 2;

    for (int i = 0; i < sizeof(protocol_name); i++) {
        built_connect_message[current_position + i] = protocol_name[i];
    }
    current_position += sizeof(protocol_name);

    built_connect_message[current_position] = protocol_level;
    current_position++;

    built_connect_message[current_position] = connect_flags;
    current_position++;

    built_connect_message[current_position] = keep_alive[0];
    current_position++;

    built_connect_message[current_position] = keep_alive[1];
    current_position++;

    for (int i = 0; i < sizeof(client_id_bytes); i++) {
        built_connect_message[current_position + i] = client_id_bytes[i];
    }
    current_position += sizeof(client_id_bytes);

    for (int i = 0; i < sizeof(username_bytes); i++) {
        built_connect_message[current_position + i] = username_bytes[i];
    }
    current_position += sizeof(username_bytes);

    for (int i = 0; i < sizeof(password_bytes); i++) {
        built_connect_message[current_position + i] = password_bytes[i];
    }
    current_position += sizeof(password_bytes);

    *connect_size = remaining_length_int + remaining_length_length + 2;

    return built_connect_message;

}


char* read_connect(char message[], size_t size, int current_position, size_t* connack_size) {
    // READ VARIABLE HEADER
    char protocol_name[] = {message[current_position], message[current_position + 1], message[current_position + 2], 
    message[current_position + 3], message[current_position + 4], message[current_position + 5]};
    printf("protocol name: ");
    for (int i = 0; i < sizeof(protocol_name); i++) {
        printf("%c", protocol_name[i]);
    }
    printf("\n");


    current_position += 6;

    char protocol_level = message[current_position];
    printf("protocol_level %d\n", protocol_level);
    current_position++;

    char connect_flags = message[current_position];
    current_position++;


    // READ CONNECTION FLAGS
    char username_flag = (connect_flags >> 7) & 1;
    char password_flag = (connect_flags >> 6) & 1;
    char will_retain_flag = (connect_flags >> 5) & 1;
    char will_qos_flag1 = (connect_flags >> 4) & 1;
    char will_qos_flag2 = (connect_flags >> 3) & 1;
    char will_qos_flag = (will_qos_flag1 << 8) | will_qos_flag2; 
    char will_flag = (connect_flags >> 2) & 1;
    char clean_start_flag = (connect_flags >> 1) & 1;

    // KEEP ALIVE
    char keep_alive_duration = (message[current_position] << 8) | message[current_position + 1];
    current_position += 2;

    //READ PAYLOAD
    char client_id_length = (message[current_position] << 8) | message[current_position + 1];
    current_position += 2;

    char client_id[client_id_length];

    printf("clientid: ");
    for (int i = 0; i < client_id_length; i++) {
        client_id[i] = message[current_position + i];
        printf("%c", client_id[i]);
    }
    printf("\n");

    current_position += client_id_length;

    if (username_flag) {
        char username_length = (message[current_position] << 8) | message[current_position + 1];
        current_position += 2;
        char username[username_length];
        printf("username: ");
        for (int i = 0; i < username_length; i++) {
            username[i] = message[current_position + i];
            printf("%c", username[i]);
        }
        printf("\n");

        current_position += username_length;
    }

    if (password_flag) {
        char password_length = (message[current_position] << 8) | message[current_position + 1];
        current_position += 2;
        char password[password_length];
        printf("password: ");
        for (int i = 0; i < password_length; i++) {
            password[i] = message[current_position + i];
            printf("%c", password[i]);
        }
        printf("\n");

        current_position += password_length;
    }

    char* connack_message;
    connack_message = build_connack(connack_size, clean_start_flag);

    return connack_message;

}


int read_connack(struct fixed_header *response_fixed_header) {
    return (response_fixed_header -> type == 2);
}
