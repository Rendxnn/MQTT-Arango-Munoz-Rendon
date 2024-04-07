#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include "connect.h"
#include "publish.h"
#include "encoder.h"

#define SERVER_ADDRESS "127.0.0.1"  // Dirección IP del servidor MQTT
#define SERVER_PORT 1883            // Puerto del servidor MQTT


int read_instruction(char message[], size_t size, struct fixed_header *message_fixed_header) {
    unsigned char first_byte = message[0];

    message_fixed_header -> type = (first_byte >> 4) & 0x0F;
    message_fixed_header -> flags = first_byte & 0x0F;
    
    
    int current_position = 1;
    message_fixed_header -> remaining_length = decode_variable_byte_integer(message, size, &current_position);

    return current_position;
}


void print_message(char message[], size_t size) {
    for (int i = 0; i < size; i++) {
        for (int j = 7; j >= 0; j--) {
            printf("%d", (message[i] >> j) & 1);
        }
    }
    printf("\n");
}


int main(int argc, char* argv[]) {

    int client_socket;
    struct sockaddr_in server_address;
    
    char buffer[1024] = {0};

    // Crear socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar dirección del servidor
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    
    // Convertir dirección IP de string a formato binario
    if(inet_pton(AF_INET, SERVER_ADDRESS, &server_address.sin_addr) <= 0) {
        perror("Dirección invalida o no soportada");
        exit(EXIT_FAILURE);
    }

    // Conectar al servidor
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error al conectar al servidor");
        exit(EXIT_FAILURE);
    }

    size_t connect_size;
    char* connect = build_connect(&connect_size);

    //print_message(connect, sizeof(connect));

    send(client_socket, connect, connect_size, 0);


    recv(client_socket, buffer, sizeof(buffer), 0);
    //printf("Respuesta del servidor (CONNACK): ");
    //print_message(buffer, sizeof(buffer));

    struct fixed_header response_fixed_header;

    int current_position = read_instruction(buffer, sizeof(buffer), &response_fixed_header);

    if (read_connack(&response_fixed_header)) {
        printf("valid connack received \n");

        printf("BIENVENIDO\n");
        while (1) {
            int opcion;
            printf("Escoja una opcion: \n1. PUBLISH \n2. SUBSCRIBE \n3. DISCONNECT\n");
            scanf("%d", &opcion);

            if (opcion == 1) {
                size_t publish_size;
                char* publish = build_publish(&publish_size);
                printf("publish message: \n");
                print_message(publish, publish_size);
                send(client_socket, publish, publish_size, 0);
            }


            
        }
        
    }
    else {
        printf("Invalid connack or connack not received");
        return 1;
    }


    memset(buffer, 0, sizeof(buffer));
    if (recv(client_socket, buffer, sizeof(buffer), 0) < 0) {
        perror("Error while receiving puback");
        exit(EXIT_FAILURE);
    }
    printf("puback received: \n");
    print_message(buffer, 4);




    // Cerrar socket
    close(client_socket);

    return 0;
}
