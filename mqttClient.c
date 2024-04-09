#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include <sys/select.h> 
#include "connect.h"
#include "publish.h"
#include "encoder.h"
#include "disconnect.h"
#include "subscribe.h"


#define SERVER_ADDRESS "127.0.0.1"  // Dirección IP del servidor MQTT
#define SERVER_PORT 1883            // Puerto del servidor MQTT
#define MAX_BUFFER_SIZE 1024        // Tamaño máximo del búfer


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

    // Construir y enviar mensaje CONNECT
    size_t connect_size;
    char* connect_message = build_connect(&connect_size);
    send(client_socket, connect_message, connect_size, 0);

    // Esperar a recibir CONNACK del servidor
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



    fd_set fds;
    int max_fd = client_socket + 1;
    printf("BIENVENIDO\n");
    while (1) {
        printf("Escoja una opcion: \n1. PUBLISH \n2. SUBSCRIBE \n3. DISCONNECT\n");
        // Limpiar el conjunto de descriptores de archivo y configurar los descriptores a vigilar
        FD_ZERO(&fds);
        FD_SET(client_socket, &fds);
        FD_SET(STDIN_FILENO, &fds);

        // Utilizar select() para esperar en los descriptores de archivo
        if (select(max_fd, &fds, NULL, NULL, NULL) < 0) {
            perror("Error en select");
            exit(EXIT_FAILURE);
        }


        // Comprobar si hay datos disponibles en el socket del cliente
        if (FD_ISSET(client_socket, &fds)) {
            printf("algo se recibio");
            //memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            printf("bytes_received: %d\n", bytes_received);
            if (bytes_received < 0) {
                perror("Error al recibir mensajes");
                exit(EXIT_FAILURE);
            } else if (bytes_received == 0) {
                // El servidor cerró la conexión
                printf("El servidor cerró la conexión\n");
                break;
            } else {
                // Decodificar y manejar el mensaje recibido
                current_position = read_instruction(buffer, bytes_received, &response_fixed_header);
                if (response_fixed_header.type == publish_code) {
                    // Este mensaje es un mensaje de publicación, imprímelo o procesa según sea necesario
                    printf("Mensaje publish recibido: \n");
                    print_message(buffer, bytes_received);
                }
            }
        }

        // Comprobar si hay entrada disponible en stdin (entrada estándar del usuario)
        if (FD_ISSET(STDIN_FILENO, &fds)) {
            int opcion;
            scanf("%d", &opcion);
            if (opcion == 1) {
                // Lógica para enviar un mensaje de publicación al servidor MQTT
                size_t publish_size;
                char* publish = build_publish(&publish_size);
                send(client_socket, publish, publish_size, 0);
            } else if (opcion == 2) {
                char subscribe_example[] = {
                    0b10000010, //fixed header
                    12, //remaining length
                    0b00000000, 0b00000001, //packet identifier

                    0b00000000, 0b00000110, 'f', 'u', 't', 'b', 'o', 'l',
                    0b00000000,

                    0b00000000 };
                size_t subscribe_size;
                //char* subscribe = build_subscribe(&subscribe_size);
                send(client_socket, subscribe_example, 14, 0);

                int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

                printf("suback received: \n");
                print_message(buffer, bytes_received);


            } else if (opcion == 3) {
                size_t disconnect_size;
                char* disconnect = build_disconnect(&disconnect_size);
                send(client_socket, disconnect, disconnect_size, 0);
                print_message(disconnect, disconnect_size);
                break;
            }
        }
    }

    // Cerrar el socket
    close(client_socket);

    return 0;
}
