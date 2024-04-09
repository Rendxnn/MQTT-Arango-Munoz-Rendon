#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <arpa/inet.h> //internamente incluye las funciones para Sockets de Berkeley
#include <pthread.h> // Agrega la biblioteca pthread.h para trabajar con hilos
#include "connect.h"
#include "publish.h"
#include "encoder.h"
#include "disconnect.h"
#include "fixedHeader.h"
#include "subscribe.h"


#define SERVER_PORT 1883

struct ThreadArgs {
    int client_socket;
};

struct Subscription {
    char topic[1024];
    int client_socket;
};

struct Subscription subscriptions[100];
int subscription_count = 0;
pthread_mutex_t subscription_mutex = PTHREAD_MUTEX_INITIALIZER;

void add_subscription(const char *topic, int client_socket) {
    pthread_mutex_lock(&subscription_mutex);
    if (subscription_count < 100) {
        strcpy(subscriptions[subscription_count].topic, topic);
        subscriptions[subscription_count].client_socket = client_socket;
        subscription_count++;
    }
    pthread_mutex_unlock(&subscription_mutex);
}


void send_message_to_subscribers(const char *topic, const char *message, size_t message_size) {
    printf("dentro de funcion\n");
    pthread_mutex_lock(&subscription_mutex);
    printf("despues de funcion rara\n");
    for (int i = 0; i < subscription_count; ++i) {
        printf("%s", subscriptions[i].topic);
        if (strcmp(subscriptions[i].topic, topic) == 0) {
            printf("enviando a un cliente\n");
            for (int i = 0; i < message_size; i++) {
                printf("%c", message[i]);
            }
            send(subscriptions[i].client_socket, message, message_size, 0);
        }
    }
    pthread_mutex_unlock(&subscription_mutex);
}


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


void *handle_client(void *args) {

    int connect_code = 1;
    int publish_code = 3;
    int subscribe_code = 8;
    int unsubscribe_code = 10;
    int disconnect_code = 14;

    struct ThreadArgs *thread_args = (struct ThreadArgs *)args;
    int client_socket = thread_args->client_socket;
    char buffer[1024] = {0};


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

        char default_topic[1024];
        strcpy(default_topic, "global");
        add_subscription(default_topic, client_socket);
        printf("Succesful default subscription creation \n");
    }

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (client_socket < 0) {
            perror("Error al aceptar la conexión");
            exit(EXIT_FAILURE);
        }
        current_message_position = 0;
        current_message_position = read_instruction(buffer, bytes_received, &message_fixed_header);
        printf("message type recieved %d\n", message_fixed_header.type);

        if (message_fixed_header.type == publish_code) {
            printf("publish recieved\n");
            //print_message(buffer, bytes_received);

            size_t payload_message_length;
            char payload_message[100];

            size_t topic_length;
            char topic[100];

            char* puback = read_publish(buffer, sizeof(buffer), current_message_position, message_fixed_header.flags, payload_message, &payload_message_length, topic, &topic_length);
            printf("payload_message_length %ld\n", payload_message_length);
            printf("payload_message from main \n");
            for (int i = 0; i < payload_message_length; i++) {
                printf("%c", payload_message[i]);
            }
            printf("\n");

            printf("topic from main \n");
            for (int i = 0; i < topic_length; i++) {
                printf("%c", topic[i]);
            }
            printf("holaquepasa");

            printf("enviar mensaje a suscriptiores");
            send_message_to_subscribers(topic, payload_message, payload_message_length);
            printf("message Succesfully sent");

        }

        else if (message_fixed_header.type == subscribe_code) {
            char client_id[] = {'c', 'l', 'i', 'e', 'n', 't'};
            printf("subscribe recieved\n");
            print_message(buffer, bytes_received);
            printf("ya");
            size_t suback_size;

            char* suback = read_subscribe(buffer, current_message_position, message_fixed_header.remaining_length, client_id, &suback_size);
            //printf("SUBACK: \n");
            //print_message(suback, suback_size);

            if (send(client_socket, suback, 4, 0) < 0) {
                perror("Error al enviar SUBACK");
                exit(EXIT_FAILURE);
            }


        }

        else if (message_fixed_header.type == disconnect_code) {
            printf("cliente desconectado\n");
            break;
        }
    }

    close(client_socket);
    free(thread_args);
    return NULL;

}

int main() {
    int server_socket;
    pthread_t threads[5];
    int thread_count = 0;
    server_socket = initialize_socket(); // Aquí inicializamos el socket

    while (1) {
        int client_socket;
        struct sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);

        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
        if (client_socket < 0) {
            perror("Error al aceptar la conexión");
            exit(EXIT_FAILURE);
        }

        printf("Cliente conectado\n");
        struct ThreadArgs *args = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
        args->client_socket = client_socket;
        if (pthread_create(&threads[thread_count++], NULL, handle_client, (void *)args) < 0) {
            perror("Error al crear el hilo");
            exit(EXIT_FAILURE);
        }

        if (thread_count >= 5) {
            // Esperar a que los hilos terminen para aceptar nuevas conexiones
            for (int i = 0; i < thread_count; ++i) {
                pthread_join(threads[i], NULL);
            }
            thread_count = 0;
        }
    }
    
    close(server_socket);

    return 0;
}


