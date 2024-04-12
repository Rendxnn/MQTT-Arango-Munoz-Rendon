#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include <sys/select.h>
#include <pthread.h> // Se agrega la librería para el manejo de hilos
#include "connect.h"
#include "publish.h"
#include "encoder.h"
#include "disconnect.h"
#include "subscribe.h"

#define SERVER_ADDRESS "127.0.0.1"  
#define SERVER_PORT 1883            
#define MAX_BUFFER_SIZE 1024        


struct ThreadArgs {
    int client_socket;
};


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




// Función para recibir mensajes en un hilo separado
void* receive_thread(void* arg) {
    int publish_code = 3;
    int connack_code = 2;
    int puback_code = 4;

    int current_position = 0;
    struct fixed_header response_fixed_header;

    struct ThreadArgs* args = (struct ThreadArgs*)arg;
    int client_socket = args->client_socket;
    char buffer[MAX_BUFFER_SIZE] = {0};

    fd_set fds;
    int max_fd = client_socket + 1;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        printf("esperando mensajes \n");
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        printf("bytes_received: %d\n", bytes_received);
        if (bytes_received < 0) {
            perror("Error al recibir mensajes");
            exit(EXIT_FAILURE);
        } else {
            current_position = read_instruction(buffer, bytes_received, &response_fixed_header);
            printf("tipo recibido: %d\n", response_fixed_header.type);
            if (response_fixed_header.type == publish_code) {

                printf("Mensaje publish recibido: \n");
                size_t payload_message_length;
                char payload_message[100];

                size_t topic_length;
                char topic[100];

                char* puback = read_publish(buffer, sizeof(buffer), current_position, response_fixed_header.flags, payload_message, &payload_message_length, topic, &topic_length);
                printf("mensaje: \n");
                for (int i = 0; i < payload_message_length; i++) {
                    printf("%c", payload_message[i]);
                }
                printf("\n");

                printf("topic: \n");

                for (int i = 0; i < topic_length; i++) {
                    printf("%c", topic[i]);
                }

                printf("\n");
            }

            if (response_fixed_header.type == connack_code) {
                printf("CONNACK recibido \n");
            }
            if (response_fixed_header.type == puback_code) {
                printf("PUBACK recibido \n");
            }
        }
    }

    return NULL;

}

int main(int argc, char* argv[]) {
    int connack_code = 2;
    int publish_code = 3;

    int client_socket;
    struct sockaddr_in server_address;
    char buffer[MAX_BUFFER_SIZE] = {0};

    // Crear socket y conectar al servidor
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    if(inet_pton(AF_INET, SERVER_ADDRESS, &server_address.sin_addr) <= 0) {
        perror("Dirección inválida o no soportada");
        exit(EXIT_FAILURE);
    }
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error al conectar al servidor");
        exit(EXIT_FAILURE);
    }

    size_t connect_size;
    char* connect_message = build_connect(&connect_size);
    send(client_socket, connect_message, connect_size, 0);

    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    struct fixed_header response_fixed_header;
    if (bytes_received < 0) {
        perror("Error al recibir mensajes");
        exit(EXIT_FAILURE);
    }

    int current_position = read_instruction(buffer, bytes_received, &response_fixed_header);
    if (response_fixed_header.type == connack_code) {
        printf("connack recibido: ");
        print_message(buffer, bytes_received);
    }

    struct ThreadArgs args;
    args.client_socket = client_socket;

    pthread_t receive_tid;
    if (pthread_create(&receive_tid, NULL, receive_thread, (void*)&args) != 0) {
        perror("Error al crear el hilo para recibir mensajes");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Escoja una opcion: \n1. PUBLISH \n2. SUBSCRIBE \n3. DISCONNECT\n");

        int opcion;
        scanf("%d", &opcion);
        if (opcion == 1) {
            size_t publish_size;
            char* publish = build_publish(&publish_size);
            send(client_socket, publish, publish_size, 0);
        } 

        else if (opcion == 2) {
            char subscribe_example[] = {
                0b10000010, //fixed header
                12, //remaining length
                0b00000000, 0b00000001, //packet identifier

                0b00000000, 0b00000110, 'f', 'u', 't', 'b', 'o', 'l',
                0b00000000,

                0b00000000 };
            size_t subscribe_size;
            send(client_socket, subscribe_example, 14, 0);

        }

        else if (opcion == 3) {
            size_t disconnect_size;
            printf("desconectando...");
            char* disconnect = build_disconnect(&disconnect_size);
            send(client_socket, disconnect, disconnect_size, 0);
            print_message(disconnect, disconnect_size);
            break;
        }
    }

    // Cerrar el socket
    close(client_socket);

    return 0;
}
