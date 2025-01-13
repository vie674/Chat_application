#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <pthread.h>
#include <netinet/in.h>
#include "TCPSocket.h"
#include <pthread.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CONNECTIONS 100    // Số kết nối tối đa
#define BUFF_SIZE 1024         // Kích thước bộ đệm

// Cấu trúc Connection lưu thông tin mỗi kết nối
typedef struct {
    int socket_fd;              // Socket của client
    char ip[INET_ADDRSTRLEN];   // Địa chỉ IP của client/server
    int id;
    int port;                   // Cổng của client/server
    int active;                 // Trạng thái kết nối (1: active, 0: inactive)
} Connection;

// Các biến toàn cục được khai báo
extern pthread_cond_t cond;         // Điều kiện đồng bộ
extern pthread_mutex_t mutex;       // Mutex đồng bộ
extern int condition;               // Biến tín hiệu
extern Connection connections[];    // Mảng lưu các kết nối
extern int connection_count;        // Số lượng kết nối hiện tại
extern char sendbuff[BUFF_SIZE];        // Số lượng kết nối hiện tại
// Khởi tạo ConnectionManager
void init_connection_manager(int port);

// Thêm một kết nối client
int add_connection(const char *server_ip, int server_port);

// Xóa một kết nối client
int remove_connection(int index);

// Hiển thị danh sách kết nối
void display_active_connections();

// Khởi chạy server trên cổng được chỉ định
void start_server(int port);

// Hàm xử lý thread client
void *client_thread(void *arg);

// Hàm xử lý thread server
void *server_thread(void *arg);

#endif // CONNECTION_MANAGER_H
