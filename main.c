#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h> // Để sử dụng inet_ntop
#include <netdb.h>     // Để sử dụng getaddrinfo và cấu trúc addrinfo
#include <unistd.h>    // Để sử dụng gethostname
#include "ConnectionManager.h"

#define BUFFER_SIZE 1024 
extern int selected_connection;  // Connection ID được chọn bởi main thread


void display_help() {
    printf("Available commands:\n");
    printf("1. help: Display available commands.\n");
    printf("2. myip: Display the IP address of this process.\n");
    printf("3. myport: Display the port on which this process is listening.\n");
    printf("4. connect: Establish a new TCP connection.\n");
    printf("5. list: Display all active connections.\n");
    printf("6. terminate: Terminate a specific connection.\n");
    printf("7. send: Send a message to a connection.\n");
    printf("8. exit: Close all connections and terminate the program.\n");
}

void displayMyIP() {

    char hostname[256];
    struct addrinfo hints, *res, *p;
    char ipstr[INET_ADDRSTRLEN];

    if (gethostname(hostname, sizeof(hostname)) != 0) {
        perror("gethostname");
        return;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, NULL, &hints, &res) != 0) {
        perror("getaddrinfo");
        return;
    }

    printf("IP address of this process:\n");

    for (p = res; p != NULL; p = p->ai_next) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
        inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
        printf("- %s\n", ipstr);
    }

    freeaddrinfo(res);
}

void displayMyPort(int port) {
    printf("My port is %d \n",port);
}

void connect_to_peer(const char *ip, int port) {
    add_connection(ip, port);
    printf("Connecting to %s:%d\n", ip, port);
}

void list_connections() {
    printf("Listing all active connections\n");
    display_active_connections();
}

void terminate_connection(int connection_id) {
    remove_connection(connection_id);
    printf("Terminating connection with ID %d\n", connection_id);
}

void send_message(int connection_id, const char *message) {
    printf("Sending message to connection %d: %s\n", connection_id, message);
}

int main(int argc, char *argv[]) {

    char dest_ip[INET_ADDRSTRLEN];
    int dest_port;
    int choice, conn_id;
    if (argc != 2) {
    fprintf(stderr, "Leak of input port or too much input port\n");
    exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    init_connection_manager(port);
    printf("Welcome to the Chat Application!\n");
    printf("Chat Application started on port %d\n", port);
    display_help();

    while (1) {
        
        printf("\n> Select an option: ");
        scanf("%d", &choice);
        getchar(); // Xóa ký tự xuống dòng còn lại trong bộ đệm

        switch (choice) {
            case 1:
                display_help();
                break;
            case 2:
                displayMyIP();
                break;
            case 3:
                displayMyPort(port);
                break;
            case 4:
                printf("Enter destination IP: ");
                fgets(dest_ip, sizeof(dest_ip), stdin);
                dest_ip[strcspn(dest_ip, "\n")] = 0;

                printf("Enter port: ");
                scanf("%d", &dest_port);
                getchar(); // Xóa ký tự xuống dòng còn lại trong bộ đệm
                connect_to_peer(dest_ip, dest_port);
                break;
            case 5:
                list_connections();
                break;
            case 6:
                printf("Enter connection ID to terminate: ");
                scanf("%d", &conn_id);
                getchar(); // Xóa ký tự xuống dòng còn lại trong bộ đệm
                terminate_connection(conn_id);
                break;
            case 7:
                printf("Enter connection ID: ");
                if (scanf("%d", &conn_id) != 1 || conn_id < 0 || conn_id >= connection_count || !connections[conn_id].active) {
                    printf("Invalid connection ID.\n");
                    while (getchar() != '\n'); // Xóa ký tự xuống dòng còn lại
                    break;
                }
                while (getchar() != '\n'); // Xóa ký tự xuống dòng còn lại
                // Đọc tin nhắn từ người dùng
                printf("Enter the message: ");
                if (!fgets(sendbuff, BUFF_SIZE, stdin)) {
                    printf("Failed to read message.\n");
                    break;
                }

                // Loại bỏ ký tự xuống dòng
                sendbuff[strcspn(sendbuff, "\n")] = 0;

                // Kiểm tra tin nhắn rỗng
                if (strlen(sendbuff) == 0) {
                    printf("Cannot send an empty message.\n");
                    break;
                }

                // Gửi tin nhắn
                Connection *conn = &connections[conn_id];
                int numb_write = write(conn->socket_fd, sendbuff, strlen(sendbuff));
                if (numb_write <= 0) {
                    perror("write() failed");
                } else {
                    printf("Message sent to %s:%d\n", conn->ip, conn->port);
                }

            case 8:
                break;
            default:
                printf("Invalid choice. Type '1' for available commands.\n");
                break;
        }
    }

    return 0;
}
