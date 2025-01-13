
#include "ConnectionManager.h"

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;  // Điều kiện
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex
int condition = 0;  // Biến tín hiệu để kiểm tra
int selected_connection = -1;
char sendbuff[BUFF_SIZE];
Connection connections[MAX_CONNECTIONS];
int connection_count = 0;

void *server_thread(void *arg);
void *client_thread(void *arg);

// Khởi tạo ConnectionManager
void init_connection_manager(int port) {
    connection_count = 0;
    memset(connections, 0, sizeof(connections));
    start_server(port);
}

// Thêm một kết nối client
int add_connection(const char *server_ip, int server_port) {
    if (connection_count >= MAX_CONNECTIONS) {
        printf("Connection limit reached. Cannot add more connections.\n");
        return -1;
    }

    for (int i = 0; i < connection_count; i++) {
        if (strcmp(server_ip, connections[i].ip) == 0 && connections[i].port == server_port) {
            printf("Connection already exists: IP=%s, Port=%d\n", server_ip, server_port);
            return -1;
        }
    }

    Connection *conn = &connections[connection_count];
    conn->id = connection_count;  // Gán ID duy nhất
    strncpy(conn->ip, server_ip, INET_ADDRSTRLEN);
    conn->port = server_port;
    conn->active = 1;

    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, client_thread, conn) != 0) {
        perror("Failed to create client thread");
        conn->active = 0;
        return -1;
    }

    pthread_detach(thread_id);
    connection_count++;
    printf("Added client connection: IP=%s, Port=%d\n", server_ip, server_port);
    return 0;
}

// void *client_thread(void *arg) {
//     Connection *conn = (Connection *)arg;

//     conn->socket_fd = create_socket_client(conn->ip, conn->port);
//     if (conn->socket_fd < 0) {
//         printf("Failed to connect to server %s:%d\n", conn->ip, conn->port);
//         conn->active = 0;
//         return NULL;
//     }

//     char sendbuff[BUFF_SIZE];
//     int numb_write;

//     while (1) {
//         pthread_mutex_lock(&mutex);
//         while (condition == 0 || selected_connection != conn->id) {
//             pthread_cond_wait(&cond, &mutex);
//         }
//         condition = 0;  // Reset điều kiện
//         pthread_mutex_unlock(&mutex);

//         memset(sendbuff, 0, BUFF_SIZE);
//         printf("Please enter the message: ");
//         if (!fgets(sendbuff, BUFF_SIZE, stdin)) {
//             printf("Failed to read input. Exiting thread.\n");
//             break;
//         }

//         // Loại bỏ ký tự xuống dòng
//         sendbuff[strcspn(sendbuff, "\n")] = 0;

//         numb_write = write(conn->socket_fd, sendbuff, strlen(sendbuff));
//         if (numb_write <= 0) {
//             perror("write() failed");
//             break;
//         }

//         if (strncmp("exit", sendbuff, 4) == 0) {
//             printf("Closing connection to server %s:%d.\n", conn->ip, conn->port);
//             break;
//         }

//         sleep(1);
//     }

//     close(conn->socket_fd);
//     conn->active = 0;
//     printf("Client thread for %s:%d terminated.\n", conn->ip, conn->port);
//     return NULL;
// }

void *client_thread(void *arg) {
    Connection *conn = (Connection *)arg;

    conn->socket_fd = create_socket_client(conn->ip, conn->port);
    if (conn->socket_fd < 0) {
        printf("Failed to connect to server %s:%d\n", conn->ip, conn->port);
        conn->active = 0;
        return NULL;
    }

    char recvbuff[BUFF_SIZE];
    int numb_read;

    while (1) {
        // Đọc tin nhắn từ server
        memset(recvbuff, 0, BUFF_SIZE);
        numb_read = read(conn->socket_fd, recvbuff, BUFF_SIZE);
        if (numb_read <= 0) {
            if (numb_read == 0) {
                printf("Server %s:%d closed the connection.\n", conn->ip, conn->port);
            } else {
                perror("read() failed");
            }
            break;
        }

        // Hiển thị tin nhắn nhận được
        printf("Message from %s:%d: %s\n", conn->ip, conn->port, recvbuff);
    }
    return NULL;
}

// Xóa một kết nối
int remove_connection(int index) {
    if (index < 0 || index >= connection_count) {
        printf("Invalid index. No connection removed.\n");
        return -1;
    }

    close(connections[index].socket_fd);
    for (int i = index; i < connection_count - 1; i++) {
        connections[i] = connections[i + 1];
    }

    connection_count--;
    printf("Removed connection at index %d\n", index);
    return 0;
}

// Hiển thị danh sách kết nối
void display_active_connections() {
    printf("Active connections:\n");
    for (int i = 0; i < connection_count; i++) {
        printf("Index=%d, IP=%s, Port=%d\n",
               i, connections[i].ip, connections[i].port);
    }
}

// Thread server: Lắng nghe và chấp nhận kết nối
void start_server(int port) {
    pthread_t server_thread_id;
    int *server_port = malloc(sizeof(int));
    *server_port = port;

    if (pthread_create(&server_thread_id, NULL, server_thread, server_port) != 0) {
        perror("Failed to create server thread");
        free(server_port);
        return;
    }

    pthread_detach(server_thread_id);
    printf("Server thread started on port %d\n", port);
}

// Thread xử lý server
void *server_thread(void *arg) {
    int port = *((int *)arg);
    free(arg);

    int server_fd = create_socket_server(port);
    if (server_fd < 0) {
        perror("Failed to start server");
        return NULL;
    }

    while (1) {
        char client_ip[INET_ADDRSTRLEN];
        int client_port;

        int client_fd = accept_client_connection(server_fd, client_ip, &client_port);
        if (client_fd >= 0) {
            printf("New connection from %s:%d\n", client_ip, client_port);

            int numb_read;
            char recvbuff[BUFF_SIZE];

            while (1) {
                memset(recvbuff, 0, BUFF_SIZE);  // Đảm bảo buffer được xóa

                /* Đọc dữ liệu từ socket */
                numb_read = read(client_fd, recvbuff, BUFF_SIZE - 1);  // Trừ 1 để chừa chỗ cho '\0'
                if (numb_read < 0) {
                    perror("read() failed");
                    break;
                }
                if (numb_read == 0) {
                    printf("Client %s:%d has closed the connection.\n", client_ip, client_port);
                    break;
                }

                recvbuff[numb_read] = '\0';  // Đảm bảo chuỗi kết thúc
                printf("Received message from %s:%d: %s\n", client_ip, client_port, recvbuff);

                if (strncmp("exit", recvbuff, 4) == 0) {
                    printf("Client %s:%d has exited the connection.\n", client_ip, client_port);
                    break;
                }
            }

            close(client_fd);
            printf("Connection with %s:%d closed.\n", client_ip, client_port);
        }
    }

    close(server_fd);
    return NULL;
}


