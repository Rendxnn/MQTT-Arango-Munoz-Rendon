#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_PORT 1883


struct fixed_header {
	unsigned char type: 4;
	unsigned char flags: 4;
	unsigned char remaining_length;
};


void read_instruction(char message[], size_t size, struct fixed_header *message_fixed_header) {
    unsigned char first_byte = message[0];
    unsigned char second_byte = message[1];

    message_fixed_header -> type = (first_byte >> 4) & 0x0F;
    message_fixed_header -> flags = first_byte & 0x0F;
    message_fixed_header -> remaining_length = second_byte;
}


void read_connect_message(char message[], size_t size) {
    // READ VARIABLE HEADER
    char protocol_name[] = {message[2], message[3], message[4], message[5], message[6], message[7]};
    char protocol_level = message[8];
    char connect_flags = message[9];


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
    char keep_alive_duration = (message[10] << 8) | message[11];

    //READ PROPERTIES
    char properties_length = message[12];

    //READ PAYLOAD
    char client_id_length = (message[13] << 8) | message[14];
    char client_id[client_id_length];
    int current_position = 15;

    for (int i = 0; i < client_id_length; i++) {
        client_id[i] = message[current_position + i];
    }

    current_position += client_id_length;

    if (username_flag) {
        char username_length = (message[current_position] << 8) | message[current_position + 1];
        current_position += 2;
        char username[username_length];

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

        for (int i = 0; i < password_length; i++) {
            password[i] = message[current_position + i];
            printf("%c", password[i]);
        }
        printf("\n");

        current_position += password_length;
    }





    printf("username_flag %d\n", username_flag);
    printf("password_flag %d\n", password_flag);
    printf("will_retain_flag %d\n", will_retain_flag);
    printf("keep_alive_duratiion %d\n", keep_alive_duration);
    printf("properties_length %d\n", properties_length);
    printf("client_id_length %d\n", client_id_length);




}


void print_message(char message[], size_t size) {
    for (int i = 0; i < size; i++) {
        for (int j = 7; j >= 0; j--) {
            printf("%d", (message[i] >> j) & 1);
        }
    }
    printf("\n");
}

//temporal
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

int main() {

    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    char buffer[1024] = {0};

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

    // Aceptar conexiones entrantes
    socklen_t client_address_length = sizeof(client_address);
    client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
    if (client_socket < 0) {
        perror("Error al aceptar la conexión");
        exit(EXIT_FAILURE);
    }

    printf("Cliente conectado\n");

    // Recibir datos del cliente
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received < 0) {
        perror("Error al recibir datos");
        exit(EXIT_FAILURE);
    }

    // Imprimir el mensaje recibido
    printf("Mensaje recibido del cliente: ");
    //print_message(buffer, sizeof(buffer));

    struct fixed_header message_fixed_header;

    read_instruction(buffer, sizeof(buffer), &message_fixed_header);

    printf("Type: %d\n", message_fixed_header.type);
    printf("Flags: %d\n", message_fixed_header.flags);
    printf("Remaining length: %d\n", message_fixed_header.remaining_length);

    if (message_fixed_header.type == 1) {
        read_connect_message(buffer, sizeof(buffer));
    }

    // Cerrar sockets
    close(client_socket);
    close(server_socket);

    return 0;
}

