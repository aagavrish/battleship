/*! @file server.c
Файл сервера игры "Морской бой".
@author Гавриш А.А.
@date 13.04.2024 */

#include <arpa/inet.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "../shared/shared.h"

#define CONFIG_FILE "config.cfg"

#define MAX_CONNECTIONS 10
#define MAX_FIELD_SIZE 20
#define BUF_CONFIG_SIZE 50

ServerConfig config;
ConfigOption options[] = {
    {"field_size", &config.field_size, parse_int},
    {"number_of_moves", &config.number_of_moves, parse_int},
    {"number_of_ships", &config.number_of_ships, parse_int},
    {"server_port", &config.server_port, parse_int},
    {"server_address", &config.server_address, parse_string},
};

char** playing_field;

void init_configuration(FILE* file);
void handle_client(int client_socket, int server_socket);
void place_ships(char*** playing_field, int number_of_ships);
void logging(char* message);
void process_player_move(char* move, char* answer, int* number_of_moves, int* number_of_ships);
bool is_valid_position(char** playing_field, int x, int y);
bool check_configuration(ServerConfig config);
GameStatus check_game_status(int number_of_moves, int number_of_ships);

/**
 * @brief Главная функция сервера. Инициализирует конфигурацию, создает сокет сервера,
 * привязывает его к адресу и порту, указанным в конфигурации, и начинает слушать
 * входящие соединения.
 *
 * @return EXIT_SUCCESS в случае успешного завершения, иначе EXIT_FAILURE.
 */
int main(void) {
    FILE* config_file = fopen(CONFIG_FILE, "r");
    if (config_file == NULL) {
        printf("ERROR: config file not found\n");
        return EXIT_FAILURE;
    }

    init_configuration(config_file);
    fclose(config_file);

    if (check_configuration(config)) {
        printf("ERROR: invalid configuration\n");
        return EXIT_FAILURE;
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    CHECK_LESS_THAN_ZERO(server_socket, "SOCKET ERROR");

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(config.server_address);
    server_address.sin_port = htons(config.server_port);

    CHECK_LESS_THAN_ZERO(bind(server_socket, (struct sockaddr*)(&server_address), sizeof(server_address)),
                         "BIND ERROR");
    CHECK_LESS_THAN_ZERO(listen(server_socket, MAX_CONNECTIONS), "LISTEN ERROR");

    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);

    while (true) {
        int client_socket = accept(server_socket, (struct sockaddr*)(&client_address), &client_len);
        CHECK_LESS_THAN_ZERO(client_socket, "ACCEPT ERROR");

        handle_client(client_socket, server_socket);
    }

    shutdown(server_socket, SHUT_RDWR);
    fclose(config_file);
    destroy_game_board(&playing_field, config.field_size);

    return EXIT_SUCCESS;
}

/**
 * @brief Инициализирует конфигурацию сервера. Считывает значения из файла конфигурации.
 *
 * @param file Файл конфигурации.
 * @return void
 */
void init_configuration(FILE* file) {
    char *key, *value;
    char buffer[BUF_CONFIG_SIZE];

    while (fgets(buffer, BUF_CONFIG_SIZE, file) != NULL) {
        key = strtok(buffer, "=");
        value = strtok(NULL, "=");

        for (int i = 0; i < (int)(sizeof(options) / sizeof(ConfigOption)); ++i) {
            if (strcmp(key, options[i].key) == 0) {
                options[i].parse(options[i].value, value);
                break;
            }
        }
    }

    return;
}

/**
 * @brief Обрабатывает подключение клиента. Создает дочерний процесс, который обрабатывает
 * ходы игрока и отправляет ответы.
 *
 * @param client_socket Сокет клиента.
 * @param server_socket Сокет сервера.
 * @return void
 */
void handle_client(int client_socket, int server_socket) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("FORK ERROR");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        close(server_socket);

        char buffer[BUF_MESSAGE_SIZE];
        recv(client_socket, buffer, BUF_MESSAGE_SIZE, 0);
        logging(buffer);

        create_game_board(&playing_field, config.field_size);
        place_ships(&playing_field, config.number_of_ships);

        int number_of_ships = config.number_of_ships, number_of_moves = 0;

        sprintf(buffer, "f=%d,n=%d", config.field_size, number_of_ships);
        send(client_socket, buffer, BUF_MESSAGE_SIZE, 0);

        GameStatus game_status = NEXT;

        while ((game_status = check_game_status(number_of_moves, number_of_ships)) == NEXT) {
            recv(client_socket, buffer, BUF_MESSAGE_SIZE, 0);
            char answer[BUF_MESSAGE_SIZE];
            process_player_move(buffer, answer, &number_of_moves, &number_of_ships);

            send(client_socket, answer, BUF_MESSAGE_SIZE, 0);
        }

        switch (game_status) {
            case WIN:
                strcpy(buffer, "You win");
                break;
            default:
                strcpy(buffer, "You lose");
                break;
        }

        send(client_socket, buffer, BUF_MESSAGE_SIZE, 0);

        shutdown(client_socket, SHUT_RDWR);
        close(client_socket);
        exit(EXIT_SUCCESS);
    }

    close(client_socket);

    return;
}

/**
 * @brief Расставляет корабли на игровом поле. Количество кораблей задается в аргументе.
 *
 * @param playing_field Указатель на игровое поле.
 * @param number_of_ships Количество кораблей.
 * @return void
 */
void place_ships(char*** playing_field, int number_of_ships) {
    srand(time(NULL));

    int ships_placed = 0;
    while (ships_placed < number_of_ships) {
        int x = rand() % config.field_size;
        int y = rand() % config.field_size;

        if (is_valid_position(*playing_field, x, y)) {
            (*playing_field)[x][y] = 'S';
            ships_placed++;
        }
    }

    return;
}

/**
 * @brief Проверяет, является ли позиция на игровом поле допустимой. Позиция допустима, если
 * вокруг нее нет кораблей.
 *
 * @param playing_field Игровое поле.
 * @param x Координата X.
 * @param y Координата Y.
 * @return true, если позиция допустима, иначе false.
 */
bool is_valid_position(char** playing_field, int x, int y) {
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (x + dx < 0 || x + dx >= config.field_size) {
                continue;
            }

            if (y + dy < 0 || y + dy >= config.field_size) {
                continue;
            }

            if (playing_field[x + dx][y + dy] == 'S') {
                return false;
            }
        }
    }

    return true;
}

/**
 * @brief Проверяет конфигурацию сервера на корректность. Проверяет размер поля и количество кораблей.
 * Если размер поля больше максимального или количество кораблей больше, чем может поместиться на поле,
 *
 * @param config Конфигурация сервера.
 * @return true, если конфигурация допустима, иначе false.
 */
bool check_configuration(ServerConfig config) {
    if (config.field_size > MAX_FIELD_SIZE) {
        return true;
    }

    double max_ships = (double)config.field_size / 2;

    if (config.number_of_ships > ceil(max_ships) * ceil(max_ships)) {
        return true;
    }

    return false;
}

/**
 * @brief Ведет логирование подключения клиента. Записывает в лог время подключения и имя клиента.
 *
 * @param message Сообщение.
 */
void logging(char* message) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    char buf[80];
    strftime(buf, sizeof(buf), "[%H:%M:%S]", t);
    printf("%s Client %s connected\n", buf, message);

    return;
}

/**
 * @brief Обрабатывает ход игрока. Проверяет, попал ли игрок в корабль, и изменяет игровое поле
 *
 * @param move Ход игрока в формате "XY", где X - буква, Y - число.
 * @param answer Строка, в которую записывается ответ.
 * @param number_of_moves Указатель на количество ходов.
 * @param number_of_ships Указатель на количество кораблей.
 */
void process_player_move(char* move, char* answer, int* number_of_moves, int* number_of_ships) {
    if (isalpha(move[0]) == 0 || isdigit(move[1]) == 0) {
        strcpy(answer, "Invalid move");
        return;
    }

    if (move[0] < 'A' || move[0] > 'A' + config.field_size) {
        strcpy(answer, "Invalid move");
        return;
    }

    int x = move[0] - 'A';
    int y = atoi(&move[1]) - 1;

    if (y >= config.field_size || y < 0 || x >= config.field_size || x < 0) {
        strcpy(answer, "Invalid move");
        return;
    }

    switch (playing_field[x][y]) {
        case '*':
            strcpy(answer, "Miss");
            playing_field[x][y] = '.';

            (*number_of_moves)++;
            break;
        case 'S':
            strcpy(answer, "Hit");
            playing_field[x][y] = 'X';

            (*number_of_ships)--;
            break;
        case 'X':
            strcpy(answer, "Already hit");
            break;
        case '.':
            strcpy(answer, "Already missed");
            break;
    }

    return;
}

/**
 * @brief Проверяет статус игры. Если количество ходов превышает максимальное, возвращает LOSE.
 *
 * @param number_of_moves Количество ходов.
 * @param number_of_ships Количество кораблей.
 * @return Статус игры.
 */
GameStatus check_game_status(int number_of_moves, int number_of_ships) {
    if (number_of_moves >= config.number_of_moves) {
        return LOSE;
    }

    if (number_of_ships == 0) {
        return WIN;
    }

    return NEXT;
}