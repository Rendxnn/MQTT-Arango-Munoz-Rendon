#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> //internamente incluye las funciones para Sockets de Berkeley
#include <math.h>

#define SERVER_PORT 1883


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


    char properties_length = 0b00000000; // working with packages with no properties for now

    //PAYLOAD
    // no payload xd


    int remaining_length_int = sizeof(connack_flags) + sizeof(connack_reason_code) + sizeof(properties_length);

    printf("remaining_length_int %d\n", remaining_length_int);  

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

    printf("built_connack_message recien creado %ld\n", sizeof(built_connack_message));

    built_connack_message[0] = connack_fixed_header;
    for (int i = 0; i < remaining_length_length; i++) {
        built_connack_message[i + 1] = remaining_length[i]; 
    }

    printf("built_connack_message despues de header %ld\n", sizeof(built_connack_message));

    built_connack_message[remaining_length_length + 2] = connack_flags;
    built_connack_message[remaining_length_length + 3] = connack_reason_code;
    built_connack_message[remaining_length_length + 4] = properties_length;

    *size = remaining_length_int + 2;


    return built_connack_message;

}


char* read_connect_message(char message[], size_t size, int current_position, size_t* connack_size) {
    // READ VARIABLE HEADER
    char protocol_name[] = {message[current_position], message[current_position + 1], message[current_position + 2], 
    message[current_position + 3], message[current_position + 4], message[current_position + 5]};
    printf("%ld\n", sizeof(protocol_name));
    for (int i = 0; i < sizeof(protocol_name); i++) {
        printf("%c", protocol_name[i]);
    }
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

    //READ PROPERTIES
    char properties_length = decode_variable_byte_integer(message, size, &current_position);

    //READ PAYLOAD
    char client_id_length = (message[current_position] << 8) | message[current_position + 1];
    current_position += 2;

    printf("client_id_length %d\n", client_id_length);

    char client_id[client_id_length];

    for (int i = 0; i < client_id_length; i++) {
        client_id[i] = message[current_position + i];
    }

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
        printf("password_length %c\n", password_length);
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


    printf("username_flag %d\n", username_flag);
    printf("password_flag %d\n", password_flag);
    printf("will_retain_flag %d\n", will_retain_flag);
    printf("keep_alive_duratiion %d\n", keep_alive_duration);
    printf("properties_length %d\n", properties_length);
    printf("client_id_length %d\n", client_id_length);

    char* connack_message;
    connack_message = build_connack(connack_size, clean_start_flag);

    printf("connack_size: %ld\n", *connack_size);
    printf("connack_message: \n");
    for (int i = 0; i < *connack_size; i++) {
        printf("%d\n", connack_message[i]);
    }

    return connack_message;

}




void print_message(char message[], size_t size) {
    for (int i = 0; i < size; i++) {
        for (int j = 7; j >= 0; j--) {
            printf("%d", (message[i] >> j) & 1);
        }
    }
    printf("\n");
}

//tempora
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
        char* connack = read_connect_message(buffer, sizeof(buffer), current_message_position, &connack_size);
        printf("connack from main: \n");
        print_message(connack, sizeof(connack));
        if (send(client_socket, connack, connack_size, 0) < 0) {
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


