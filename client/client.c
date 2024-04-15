/*! @file client.c
File implementing the client side of the battleship game.
The client connects to the server, sends the player's name, and then plays the game.
The player is prompted to enter a move, which is then sent to the server.
The game board is displayed after each move, showing the player's hits and misses.
The game continues until all ships have been sunk or the server disconnects.
@author Gavrish A.A.
@date 13.04.2024 */

#include <arpa/inet.h>
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "../shared/shared.h"

/**
 * @brief Configuration structure for the client.
 *
 * @see ClientConfig
 */
ClientConfig config;

/**
 * @brief Configuration options for the client.
 *
 * @see ConfigOption
 */
ConfigOption options[] = {
    {"h", &config.server_address, parse_string},
    {"p", &config.server_port, parse_int},
    {"n", &config.client_name, parse_string},
};

char** playing_field;

void display_game_status(char** playing_field, int field_size, char* prev_move, char* answer, int ships_left);
void init_configuration(int argc, char* argv[]);
void send_player_name(int client_socket, char* name);
void connect_to_server(int* client_socket);
bool make_move(char* move);

/**
 * @brief Main function for the client. Connects to the server, sends the player's name, and plays the game.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return EXIT_SUCCESS if the program exits successfully, EXIT_FAILURE otherwise.
 */
int main(int argc, char* argv[]) {
    init_configuration(argc, argv);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    connect_to_server(&client_socket);

    send_player_name(client_socket, config.client_name);

    char buffer[BUF_MESSAGE_SIZE];
    recv(client_socket, buffer, BUF_MESSAGE_SIZE, 0);

    int field_size, global_number_of_ships;
    sscanf(buffer, "f=%d,n=%d", &field_size, &global_number_of_ships);
    create_game_board(&playing_field, field_size);

    int local_numbers_of_ships = 0;
    char prev_move[BUF_MESSAGE_SIZE], answer[BUF_MESSAGE_SIZE];

    while (client_socket) {
        display_game_status(playing_field, field_size, prev_move, answer,
                            global_number_of_ships - local_numbers_of_ships);

        if (make_move(buffer)) {
            continue;
        }

        strcpy(prev_move, buffer);
        send(client_socket, buffer, BUF_MESSAGE_SIZE, 0);

        recv(client_socket, buffer, BUF_MESSAGE_SIZE, 0);
        strcpy(answer, buffer);

        if (strcmp(answer, "Miss") == 0) {
            playing_field[atoi(&prev_move[1]) - 1][prev_move[0] - 'A'] = '.';
        } else if (strcmp(answer, "Hit") == 0) {
            playing_field[atoi(&prev_move[1]) - 1][prev_move[0] - 'A'] = 'X';
            local_numbers_of_ships++;
        }
    }

    destroy_game_board(&playing_field, field_size);
    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);

    return EXIT_SUCCESS;
}

/**
 * @brief Displays the current game status. This includes the game board, the last move, the result of the
 * last move, and the number of ships left.
 * @param playing_field The game board.
 * @param field_size The size of the game board.
 * @param prev_move The last move made by the player.
 * @param answer The result of the last move.
 * @param ships_left The number of ships left on the game board.
 * @return void
 */
void display_game_status(char** playing_field, int field_size, char* prev_move, char* answer,
                         int ships_left) {
    printf("\033[H\033[J");

    int field_width = field_size * 2;
    int title_length = strlen("BATTLESHIP");
    int padding = (field_width - title_length) / 2;

    for (int i = 0; i < padding; i++) {
        printf("=");
    }

    printf(" BATTLESHIP ");

    for (int i = 0; i < padding; i++) {
        printf("=");
    }

    printf("\n\n");
    printf("   ");

    for (int i = 0; i < field_size; ++i) {
        printf("%c ", 'A' + i);
    }

    printf("\n");

    for (int i = 0; i < field_size; ++i) {
        printf("%2d ", i + 1);

        for (int j = 0; j < field_size; ++j) {
            printf("%c ", playing_field[i][j]);
        }

        printf("\n");
    }

    printf("\n");

    for (int i = 0; i < field_size * 2 + 2; ++i) {
        printf("=");
    }
    printf("\n\n");

    printf("| GAME INFO\n");
    if (prev_move[0] != '\0' && answer[0] != '\0') {
        printf("| Last move: %s - %s\n", prev_move, answer);
    }
    printf("| Ships left: %d\n", ships_left);
    printf("| Enter your move: ");

    return;
}

/**
 * @brief Initializes the configuration for the client. Parses the command-line arguments and sets the
 * configuration values.
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return void
 * @see ConfigOption
 */
void init_configuration(int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "h:p:n:")) != -1) {
        for (int i = 0; i < (int)(sizeof(options) / sizeof(ConfigOption)); ++i) {
            if (options[i].key[0] == opt) {
                options[i].parse(options[i].value, optarg);
                break;
            }
        }
    }

    return;
}

/**
 * @brief Sends the player's name to the server. The player's name is sent as a message to the server.
 * @param client_socket The client's socket.
 * @param name The player's name.
 * @return void
 */
void send_player_name(int client_socket, char* name) {
    send(client_socket, name, BUF_MESSAGE_SIZE, 0);
    return;
}

/**
 * @brief Connects the client to the server. The client creates a socket and connects to the server using the
 * server's address and port number.
 * @param client_socket The client's socket.
 * @return void
 */
void connect_to_server(int* client_socket) {
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(config.server_address);
    server_address.sin_port = htons(config.server_port);

    *client_socket = socket(AF_INET, SOCK_STREAM, 0);
    CHECK_LESS_THAN_ZERO(*client_socket, "SOCKET ERROR");

    CHECK_LESS_THAN_ZERO(connect(*client_socket, (struct sockaddr*)&server_address, sizeof(server_address)),
                         "CONNECT ERROR");

    return;
}

/**
 * @brief Prompts the player to enter a move. The player's move is read from the standard input.
 * @param move The player's move.
 * @return true if the player's move is invalid, false otherwise.
 */
bool make_move(char* move) {
    int scanf_result = scanf("%3s", move);
    if (scanf_result == EOF) {
        exit(EXIT_FAILURE);
    }

    if (scanf_result != 1) {
        return true;
    }

    if (strlen(move) > 3) {
        return true;
    }

    for (int i = 0; move[i]; i++) {
        move[i] = toupper((unsigned char)move[i]);
    }

    return false;
}