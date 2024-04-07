#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <arpa/inet.h> //internamente incluye las funciones para Sockets de Berkeley
#include "connect.h"
#include "publish.h"
#include "encoder.h"
#include "fixedHeader.h"


#define SERVER_PORT 1883


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
            //printf("%c", message[i]);
        }
    }
    printf("\n");
}


int initialize_socket() {
    int server_socket;
    struct sockaddr_in server_address;

    // Crear socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar dirección del servidor
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);

    // Enlazar socket al puerto
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error al enlazar el socket");
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(server_socket, 5) < 0) {
        perror("Error al escuchar");
        exit(EXIT_FAILURE);
    }

    printf("Esperando clientes...\n");

    return server_socket;
}

int main() {
    int connect_code = 1;
    int publish_code = 3;
    int subscribe_code = 8;
    int unsubscribe_code = 10;
    int disconnect = 14;
    int server_socket, client_socket;
    struct sockaddr_in client_address;
    char buffer[1024] = {0};

    // Inicializar el socket
    server_socket = initialize_socket();

    // Aceptar conexiones entrantes
    socklen_t client_address_length = sizeof(client_address);
    client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
    if (client_socket < 0) {
        perror("Error al aceptar la conexión");
        exit(EXIT_FAILURE);
    }

    printf("Cliente conectado\n");

    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received < 0) {
        perror("Error al recibir datos");
        exit(EXIT_FAILURE);
    }

    // Imprimir el mensaje recibido
    printf("Mensaje recibido del cliente: ");
    //print_message(buffer, sizeof(buffer));

    struct fixed_header message_fixed_header;

    int current_message_position = read_instruction(buffer, sizeof(buffer), &message_fixed_header) + 1;

    printf("Type: %d\n", message_fixed_header.type);
    printf("Flags: %d\n", message_fixed_header.flags);
    printf("Remaining length: %d\n", message_fixed_header.remaining_length);

    if (message_fixed_header.type == 1) {
        size_t connack_size;
        char* connack = read_connect(buffer, sizeof(buffer), current_message_position, &connack_size);
        printf("connack from main: \n");
        print_message(connack, connack_size);
        if (send(client_socket, connack, connack_size, 0) < 0) {
            perror("Error al enviar CONNACK");
            exit(EXIT_FAILURE);
        }
    }

    memset(buffer, 0, sizeof(buffer));
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (client_socket < 0) {
        perror("Error al aceptar la conexión");
        exit(EXIT_FAILURE);
    }
    current_message_position = 0;
    current_message_position = read_instruction(buffer, bytes_received, &message_fixed_header);
    printf("message[current_position] %d\n", buffer[current_message_position]);

    if (message_fixed_header.type == publish_code) {
                printf("publish recieved\n");
                print_message(buffer, bytes_received);
                char* puback = read_publish(buffer, sizeof(buffer), current_message_position, message_fixed_header.flags);
                print_message(puback, 4);
                if (send(client_socket, puback, 4, 0) < 0) {
                    perror("Error al enviar CONNACK");
                    exit(EXIT_FAILURE);
                }
    }

    // Resto del código para manejar la comunicación con el cliente
    // Recibir datos, procesarlos, etc.

    // Cerrar sockets
    close(client_socket);
    close(server_socket);

    return 0;
}

