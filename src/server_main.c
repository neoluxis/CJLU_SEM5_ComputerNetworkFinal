#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>

#define BUF_SIZE 1024
#define MAX_CLNT 256

void *handle_client(void *arg);
void error_handling(const char *message);
void signal_handler(int sig);

int client_socks[MAX_CLNT];
int client_count = 0;
int serv_sock;

pthread_mutex_t mutex;

int main(int argc, char *argv[]) {
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    pthread_t t_id;

    if (argc != 2) {
        printf("Usage: %s <Port>\n", argv[0]);
        exit(1);
    }

    pthread_mutex_init(&mutex, NULL);

    signal(SIGINT, signal_handler);

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    printf("Server started on port %s\n", argv[1]);

    while (1) {
        clnt_addr_size = sizeof(clnt_addr);
        int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1) {
            perror("accept() error");
            continue;
        }

        pthread_mutex_lock(&mutex);
        client_socks[client_count++] = clnt_sock;
        pthread_mutex_unlock(&mutex);

        pthread_create(&t_id, NULL, handle_client, (void*)&clnt_sock);
        pthread_detach(t_id);

        printf("Connected client IP: %s\n", inet_ntoa(clnt_addr.sin_addr));
    }

    return 0;
}

void *handle_client(void *arg) {
    int clnt_sock = *((int*)arg);
    int str_len;
    char message[BUF_SIZE];

    while ((str_len = read(clnt_sock, message, BUF_SIZE)) != 0) {
        message[str_len] = 0;
        printf("Received from client: %s", message);
	
        write(clnt_sock, message, str_len);  
    }

    pthread_mutex_lock(&mutex);
    for (int i = 0; i < client_count; i++) {
        if (client_socks[i] == clnt_sock) {
            while (i < client_count - 1)
                client_socks[i] = client_socks[i + 1];
            break;
        }
    }
    client_count--;
    pthread_mutex_unlock(&mutex);

    close(clnt_sock);
    printf("Client disconnected.\n");
    return NULL;
}

void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\nShutting down server...\n");

        pthread_mutex_lock(&mutex);
        for (int i = 0; i < client_count; i++) {
            close(client_socks[i]);
        }
        pthread_mutex_unlock(&mutex);

        close(serv_sock);

        pthread_mutex_destroy(&mutex);

        exit(0);
    }
}

void error_handling(const char *message) {
    perror(message);
    exit(1);
}

