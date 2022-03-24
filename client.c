// Client side implementation of UDP client-server model
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT     8080
#define BUFFER_SIZE 1024

void send_file(char *file_dest, long n_o_char, int socket_descriptor, struct sockaddr_in server_address) {
    long pointer = 0;
    int number_of_packets=0;
    unsigned char buffer[BUFFER_SIZE] = {'\0'};
    FILE *read_file = fopen(file_dest,"rb");
    while(1) {
        // clear buffer
        for (int i  = 0; i < BUFFER_SIZE; i++) { buffer[i] = '\0'; }

        // fill buffer
        for (int i  = 0; i < BUFFER_SIZE && pointer != n_o_char; i++) {
            buffer[i] = getc(read_file);
            pointer++;
        }

        // send buffer
        sendto(socket_descriptor, buffer, sizeof(unsigned char)*BUFFER_SIZE,MSG_CONFIRM, (const struct sockaddr *) &server_address,sizeof(server_address));
        number_of_packets++;

        // end if it was last packet
        if (pointer >= n_o_char) { break; }
    }

    printf("%d data packets sent\n",number_of_packets);

    fclose(read_file);
}

int main(int argc, char *argv[]) {

    // ERROR messages
    if (argc < 3) {
        fprintf(stderr, "Enter arguments! (address and file destination)\n");
        return 100;
    }

    printf("Dest: %s:%d\n",argv[1],PORT);

    if ( access(argv[2], F_OK) == 0 ) {
        printf("File: %s\n",argv[2]);
    } else {
        fprintf(stderr, "ERROR: File %s dont exist!\n", argv[2]);
        return 100;
    }

    int socket_descriptor;
    if ( (socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        fprintf(stderr, "ERROR: Socket file descriptor failed\n");
        return 100;
    }

    printf("----------------------\n\n");



    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    // fill server information
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr(argv[1]);

    // send server length of file
    FILE *read_file;
    read_file = fopen(argv[2],"rb");
    fseek(read_file, 0, SEEK_END);
    long n_o_char = ftell(read_file);
    fclose(read_file);
    printf("File length is %ld\n",n_o_char);
    unsigned char str[1024] = {'\0'};
    sprintf((char*)str, "%ld", n_o_char);
    sendto(socket_descriptor, str, sizeof(unsigned char)*BUFFER_SIZE,MSG_CONFIRM, (const struct sockaddr *) &server_address,sizeof(server_address));
    printf("Length info sent\n");

    // send file
    send_file(argv[2], n_o_char, socket_descriptor, server_address);

    close(socket_descriptor);

    return 0;
}