#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void displayBoard(char board[3][3]);

int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];
    char board[3][3];
    int row, col;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        printf("Connection to server failed. Error Code: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    printf("Connected to the server.\n");

    while (1) {
        if (recv(client_socket, (char *)board, sizeof(board), 0) <= 0) {
            printf("Connection lost with server.\n");
            break;
        }

        displayBoard(board);

        if (recv(client_socket, buffer, BUFFER_SIZE, 0) <= 0) {
            printf("Connection lost with server.\n");
            break;
        }
        printf("%s", buffer);

        if (strncmp(buffer, "Player", 6) == 0 || strncmp(buffer, "It's a draw", 11) == 0) {
            break;
        }

        printf("Enter your move (row and column): ");
        scanf("%d %d", &row, &col);
        snprintf(buffer, BUFFER_SIZE, "%d%d", row, col);
        send(client_socket, buffer, strlen(buffer), 0);
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}

void displayBoard(char board[3][3]) {
    printf("\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf(" %c ", board[i][j]);
            if (j < 2) printf("|");
        }
        printf("\n");
        if (i < 2) printf("---+---+---\n");
    }
    printf("\n");
}
