#ifndef TCP_H
#define TCP_H_H

#include <netinet/in.h>

#define MAX_CLIENTS 10

// Macro xử lý lỗi
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

// Hàm tạo socket server
int create_socket_server(int port);

// Hàm chấp nhận kết nối từ client
int accept_client_connection(int server_fd, char *client_ip, int *client_port);

// Hàm tạo socket client và kết nối tới server
int create_socket_client(const char *server_ip, int server_port);

// Hàm lấy thông tin IP/Port của client
int get_client_info(int socket_fd, char *client_ip, int *client_port);

#endif
