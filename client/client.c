/*! @file client.c
Файл клиента игры "Морской бой".
@author Гавриш А.А.
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

ClientConfig config;
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
 * @brief Главная функция клиента. Инициализирует конфигурацию, создает сокет клиента,
 * подключается к серверу и отправляет имя пользователя.
 *
 * @param argc Количество аргументов командной строки.
 * @param argv Массив аргументов командной строки.
 * @return EXIT_SUCCESS в случае успешного завершения, иначе EXIT_FAILURE.
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
 * @brief Отображает текущее состояние игры.
 *
 * @param playing_field Игровое поле.
 * @param field_size Размер игрового поля.
 * @param prev_move Предыдущий ход.
 * @param answer Ответ сервера на предыдущий ход.
 *
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
 * @brief Инициализирует конфигурацию клиента.
 *
 * @param argc Количество аргументов командной строки.
 * @param argv Массив аргументов командной строки.
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
 * @brief Отправляет имя игрока на сервер.
 *
 * @param client_socket Сокет клиента.
 * @param name Имя игрока.
 */
void send_player_name(int client_socket, char* name) {
    send(client_socket, name, BUF_MESSAGE_SIZE, 0);
    return;
}

/**
 * @brief Подключается к серверу.
 *
 * @param client_socket Сокет клиента.
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
 * @brief Считывает ход игрока.
 *
 * @param move Ход игрока.
 * @return true в случае ошибки, иначе false.
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