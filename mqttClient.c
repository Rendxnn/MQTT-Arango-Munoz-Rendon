#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_ADDRESS "127.0.0.1"  // Dirección IP del servidor MQTT
#define SERVER_PORT 1883            // Puerto del servidor MQTT


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
    char connect_message[] = {
    //FIXED HEADER
    0b00010010, 0b00100101,   // Control Packet Type (CONNECT), Remaining Length

    // VARIABLE HEADER
    0b00000000, 0b00000100, 'M', 'Q', 'T', 'T',   // Protocol Name (MQTT)
    0b00000101,   // Protocol Level
    0b11000010,   // Connect Flags: Clean Session = 1
    0b00000000, 0b00111100,   // Keep Alive (60 segundos)

    // VARIABLE HEADER PROPERTIES
    0b00000000, // properties length
    // 0b00010001, // Session Expiry Interval Identifier (17) 
    // 0b00000000, 0b00000000,
    // 0b00000000, 0b00001010, //Sesion expiry interval (10)

    //PAYLOAD
    0b00000000, 0b00001001, 'c', 'l', 'i', 'e', 'n', 't', '1', '2', '3',   // Client Identifier length // Client Identifier (client123)
    0b00000000, 0b00000100, 'r', 'e', 'n', 'd',   // Username length // User Name (user)
    0b00000000, 0b00001000, 'p', 'a', 's', 's', 'w', 'o', 'r', 'd'   // Password length // Password (password)
};
    
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
    send(client_socket, connect_message, sizeof(connect_message), 0);
    printf("Mensaje enviado al servidor: " );
    print_message(connect_message, sizeof(connect_message));



    // Recibir respuesta del servidor
    recv(client_socket, buffer, 1024, 0);
    printf("Respuesta del servidor: %s\n", buffer);

    // Cerrar socket
    close(client_socket);

    return 0;
}
