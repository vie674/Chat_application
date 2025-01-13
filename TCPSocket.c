#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "TCPSocket.h"

// Hàm tạo socket
int create_socket_server(int port) {
    int server_fd, opt = 1;
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in));

    // Tao Socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        handle_error("Socket creation failed");

    /* Ngăn lỗi : “address already in use” */
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        handle_error("setsockopt()");  

    // Khởi tạo địa chỉ cho server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    //Gắn socket với địa chỉ server
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        handle_error("Bind failed");

    if (listen(server_fd, MAX_CLIENTS) < 0)
        handle_error("Listen failed");

    printf("Server listening on port %d\n", port);
    return server_fd;
}

int accept_client_connection(int server_fd, char* client_ip, int *client_port) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Chấp nhận kết nối
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("Accept failed");
        return -1;
    }

    // Lấy địa chỉ IP của client
    if (inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN) == NULL) {
        perror("inet_ntop() failed");
        close(client_fd);
        return -1;
    }

    // Lấy số port của client
    *client_port = ntohs(client_addr.sin_port);

    printf("Accepted connection from %s:%d\n", client_ip, *client_port);
    return client_fd;
}

int create_socket_client(const char *server_ip, int server_port) {
    int client_fd;
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in));

    // Tao Socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
        handle_error("Socket creation failed");

    // Khởi tạo địa chỉ cho server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) == -1) 
        handle_error("inet_pton()");

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_fd);
        return -1;
    }

    printf("Connected to server %s:%d\n", server_ip, server_port);
    return client_fd;
}

int get_client_info(int socket_fd, char *client_ip, int *client_port) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if (getpeername(socket_fd, (struct sockaddr *)&client_addr, &addr_len) == -1) {
        perror("getpeername() failed");
        return -1;
    }

    // Lấy địa chỉ IP
    if (inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN) == NULL) {
        perror("get ip failed");
        return -1;
    }

    // Lấy số port
    *client_port = ntohs(client_addr.sin_port);
    return 0;
}


