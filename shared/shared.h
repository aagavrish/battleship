/*! @file shared.c
Файл с общими функциями для сервера и клиента игры "Морской бой".
@author Гавриш А.А.
@date 13.04.2024 */

#ifndef SHARED_H
#define SHARED_H

#define BUF_MESSAGE_SIZE 15

#define CHECK_LESS_THAN_ZERO(val, msg) \
    if ((val) < 0) {                   \
        perror((msg));                 \
        exit(EXIT_FAILURE);            \
    }

/**
 * @struct ServerConfig
 * @brief Структура для хранения конфигурации сервера.
 *
 * @param field_size Размер поля.
 * @param number_of_moves Количество ходов.
 * @param number_of_ships Количество кораблей.
 * @param server_port Порт сервера.
 * @param server_address Адрес сервера.
 */
typedef struct {
    int field_size;
    int number_of_moves;
    int number_of_ships;
    int server_port;
    char server_address[16];
} ServerConfig;

/**
 * @struct ClientConfig
 * @brief Структура для хранения конфигурации клиента.
 *
 * @param client_name[10] Имя клиента.
 * @param server_address[16] Адрес сервера.
 * @param server_port Порт сервера.
 */
typedef struct {
    char client_name[10];
    char server_address[16];
    int server_port;
} ClientConfig;

/**
 * @struct ConfigOption
 * @brief Структура для хранения опций конфигурации.
 *
 * @param key Ключ опции.
 * @param value Значение опции.
 * @param parse Функция для парсинга значения опции.
 */
typedef struct {
    const char* key;
    void* value;
    void (*parse)(void* value, const char* str);
} ConfigOption;

/**
 * @brief Перечисленение для хранения логического значения.
 */
typedef enum {
    false = 0, /**< Ложь*/
    true = 1   /**< Истина*/
} bool;

/**
 * @brief Перечисление для хранения статуса игры.
 */
typedef enum {
    NEXT = 0, /**< Следующий ход. */
    WIN,      /**< Победа. */
    LOSE      /**< Поражение. */
} GameStatus;

void parse_int(void* value, const char* str);
void parse_string(void* value, const char* str);
void create_game_board(char*** playing_field, int field_size);
void destroy_game_board(char*** playing_field, int field_size);

#endif