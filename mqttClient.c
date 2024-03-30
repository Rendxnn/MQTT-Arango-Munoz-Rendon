#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_ADDRESS "127.0.0.1"  // Dirección IP del servidor MQTT
#define SERVER_PORT 1883            // Puerto del servidor MQTT

int main(int argc, char* argv[]) {
    int client_socket;
    struct sockaddr_in server_address;
    char message[] = "Hola, servidor MQTT!";
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

    // Enviar mensaje al servidor
    send(client_socket, message, strlen(message), 0);
    printf("Mensaje enviado al servidor: %s\n", message);

    // Recibir respuesta del servidor
    recv(client_socket, buffer, 1024, 0);
    printf("Respuesta del servidor: %s\n", buffer);

    // Cerrar socket
    close(client_socket);

    return 0;
}
