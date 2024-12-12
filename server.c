#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080
#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024

// Function declarations
void initializeBoard(char board[3][3]);
void displayBoard(char board[3][3]);
int checkWin(char board[3][3]);

int main() {
    WSADATA wsaData;
    SOCKET server_fd, new_socket[MAX_CLIENTS];
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char board[3][3], buffer[BUFFER_SIZE];
    int current_player = 0; // 0 or 1
    int client_count = 0;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        exit(EXIT_FAILURE);
    }

    // Initialize the game board
    initializeBoard(board);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("Socket failed");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        perror("Bind failed");
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Listen for clients
    if (listen(server_fd, MAX_CLIENTS) == SOCKET_ERROR) {
        perror("Listen failed");
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    // Accept connections from clients
    while (client_count < MAX_CLIENTS) {
        if ((new_socket[client_count] = accept(server_fd, (struct sockaddr *)&address, &addrlen)) == INVALID_SOCKET) {
            perror("Accept failed");
            closesocket(server_fd);
            WSACleanup();
            exit(EXIT_FAILURE);
        }
        printf("Player %d connected.\n", client_count + 1);
        client_count++;
    }

    // Send initial board to both players
    for (int i = 0; i < MAX_CLIENTS; i++) {
        send(new_socket[i], (const char *)board, sizeof(board), 0);
    }

    // Game loop
    while (1) {
        // Notify the current player to make a move
        snprintf(buffer, sizeof(buffer), "Your turn, Player %d:\n", current_player + 1);
        send(new_socket[current_player], buffer, strlen(buffer), 0);

        // Receive the player's move
        recv(new_socket[current_player], buffer, BUFFER_SIZE, 0);
        int row = buffer[0] - '0';
        int col = buffer[1] - '0';

        // Validate the move and update the board
        if (row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == ' ') {
            board[row][col] = (current_player == 0) ? 'X' : 'O';

            // Check for a winner
            if (checkWin(board)) {
                snprintf(buffer, sizeof(buffer), "Player %d wins!\n", current_player + 1);
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    send(new_socket[i], buffer, strlen(buffer), 0);
                }
                break;
            }

            // Check for a draw
            int draw = 1;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (board[i][j] == ' ') {
                        draw = 0;
                        break;
                    }
                }
                if (!draw) break;
            }

            if (draw) {
                snprintf(buffer, sizeof(buffer), "It's a draw!\n");
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    send(new_socket[i], buffer, strlen(buffer), 0);
                }
                break;
            }

            // Switch to the next player
            current_player = 1 - current_player;

            // Send updated board to both players
            for (int i = 0; i < MAX_CLIENTS; i++) {
                send(new_socket[i], (const char *)board, sizeof(board), 0);
            }
        } else {
            // Invalid move
            snprintf(buffer, sizeof(buffer), "Invalid move. Try again.\n");
            send(new_socket[current_player], buffer, strlen(buffer), 0);
        }
    }

    // Close connections
    for (int i = 0; i < MAX_CLIENTS; i++) {
        closesocket(new_socket[i]);
    }
    closesocket(server_fd);
    WSACleanup();

    return 0;
}

void initializeBoard(char board[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            board[i][j] = ' ';
        }
    }
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

int checkWin(char board[3][3]) {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != ' ') return 1;
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != ' ') return 1;
    }
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != ' ') return 1;
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != ' ') return 1;
    return 0;
}
