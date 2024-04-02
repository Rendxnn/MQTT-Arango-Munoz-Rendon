#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#define SERVER_ADDRESS "127.0.0.1"  // Dirección IP del servidor MQTT
#define SERVER_PORT 1883            // Puerto del servidor MQTT


struct fixed_header {
    unsigned char type: 4;
    unsigned char flags: 4;
    int remaining_length;
};


int decode_variable_byte_integer(char message[], size_t size, int *current_position) {

    int multiplier = 1;
    int value = 0;
    char encoded_byte;
    do {
        encoded_byte = message[*current_position];
        value += (encoded_byte & 127) * multiplier;
        if (multiplier > 128 * 128 * 128) {
            printf("malformed varaible byte integer");
            return -1;
        }
        multiplier *= 128;
        (*current_position)++;
    }
    while ((encoded_byte & 128) != 0);

    return value;  
}

void encode_variable_byte_integer(int number, char result[]) {
    char encoded_byte;
    int position = 0;
    do {
        encoded_byte = number % 128;
        number = number / 128;

        if (number > 0) {
            encoded_byte = encoded_byte | 128;
        }
        result[position] = encoded_byte;
        position++;
    } while (number > 0);
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
        }
    }
    printf("\n");
}


int calculate_variable_byte_length(int remaining_length_int) {
    int remaining_length_length; 
    for (int i = 1; i < 5; i++) {
        if (remaining_length_int < pow(2, 7 * i - 1)) {
            remaining_length_length = i;
            break;
        }
    }

    return remaining_length_length;
}


char* build_connect(size_t* connect_size) {

    char packet_type = 0b00010000;

    char protocol_name[] = {0b00000000, 0b00000100, 'M', 'Q', 'T', 'T'};
    char protocol_level = 0b00000101;
    char connect_flags = 0b11000010;
    char keep_alive[] = {0b00000000, 0b00111100};
    char properties_length = 0b00000000;

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


    printf("username_length %ld\n", username_length);
    printf("password_length %ld\n", password_length);

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

    int remaining_length_int = sizeof(protocol_name) + sizeof(protocol_level) + sizeof(connect_flags) + sizeof(keep_alive) + sizeof(properties_length) + sizeof(username_bytes) + sizeof(password_bytes) + sizeof(client_id_bytes);

    int remaining_length_length = calculate_variable_byte_length(remaining_length_int);

    char remaining_length[remaining_length_length];

    printf("remaining_length connect: %d\n", remaining_length_int);

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

    built_connect_message[current_position] = properties_length;
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

    printf("mensaje construido: \n");
    print_message(built_connect_message, *connect_size);

    return built_connect_message;

}

int main(int argc, char* argv[]) {
    int client_socket;
    struct sockaddr_in server_address;
    char connect_message[] = {
    //FIXED HEADER
    0b00010000, 0b00100110,   // Control Packet Type (CONNECT), Remaining Length

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

    size_t connect_size;
    char* connect = build_connect(&connect_size);

    //print_message(connect, sizeof(connect));

    // Enviar mensaje al servidor
    send(client_socket, connect, connect_size, 0);


    // Recibir respuesta del servidor
    recv(client_socket, buffer, sizeof(buffer), 0);
    //printf("Respuesta del servidor (CONNACK): ");
    //print_message(buffer, sizeof(buffer));

    struct fixed_header response_fixed_header;

    int current_position = read_instruction(buffer, sizeof(buffer), &response_fixed_header);

    printf("type: %d\n", response_fixed_header.type);

    printf("remaining_length: %d\n", response_fixed_header.remaining_length);


    // Cerrar socket
    close(client_socket);

    return 0;
}
