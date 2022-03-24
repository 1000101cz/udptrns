
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT     8080
#define BUFFER_SIZE 1024


void receive_message(char *file_dest, int socket_descriptor, struct sockaddr_in client_address, int len, long message_length) {
    unsigned char buffer[BUFFER_SIZE] = {'\0'};
    long counter = 0;
    FILE *new_file;
    new_file = fopen(file_dest,"wb");

    while (1) {
        recvfrom(socket_descriptor, buffer, sizeof(unsigned char)*BUFFER_SIZE,MSG_WAITALL, ( struct sockaddr *) &client_address,(unsigned int*)&len);

        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (counter == message_length) { break; } // reached end of message
            fputc(buffer[i],new_file);
            counter++;
        }
        if (counter == message_length) { break; }
    }

    fclose(new_file);
}


int main(int argc, char *argv[]) {

    // ERROR messages
    if (argc < 2) {
        fprintf(stderr,"ERROR: destination for new file required!\n");
        return 100;
    }
    char *file_dest = argv[1];
    if ( access(file_dest, F_OK) == 0 ) {
        fprintf(stderr,"ERROR: File exists!\n");
        return 100;
    }

    // Create socket file descriptor
    int socket_descriptor;
    if ( (socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { return 100; }

    struct sockaddr_in server_address, client_address;
    memset(&server_address, 0, sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));
    int len = sizeof(client_address);

    // fill server information
    server_address.sin_family    = AF_INET; // IPv4
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // bind the socket with the server address
    if ( bind(socket_descriptor, (const struct sockaddr *)&server_address,sizeof(server_address)) < 0 ) { return 100; }

    // get total length of message from first packet
    unsigned char buffer[BUFFER_SIZE] = {'\0'};
    recvfrom(socket_descriptor, buffer, sizeof(unsigned char)*BUFFER_SIZE,MSG_WAITALL, ( struct sockaddr *) &client_address,(unsigned int*)&len);
    long message_length = atol(buffer);
    printf("msg length: %ld\n",message_length);

    // receive message
    receive_message(file_dest, socket_descriptor, client_address, len, message_length);

    return 0;
}