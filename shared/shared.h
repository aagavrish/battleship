/*! @file shared.c
File containing shared functions and structures for the battleship game.
The shared functions include parsing configuration options and creating and destroying the game board.
The shared structures include the configuration options and the game board.
@author Gavrish A.A.
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
 * @brief Structure for storing server configuration.
 *
 * @param field_size Size of the game board.
 * @param number_of_moves Number of moves per player.
 * @param number_of_ships Number of ships on the game board.
 * @param server_port Port number for the server.
 * @param server_address IP address of the server.
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
 * @brief Structure for storing client configuration.
 *
 * @param client_name Name of the client.
 * @param server_address IP address of the server.
 * @param server_port Port number for the server.
 */
typedef struct {
    char client_name[10];
    char server_address[16];
    int server_port;
} ClientConfig;

/**
 * @struct ConfigOption
 * @brief Structure for storing configuration options.
 * The structure contains a key, a pointer to the value, and a function to parse the value.
 * The key is used to identify the option in the command-line arguments.
 * The value is a pointer to the variable where the parsed value will be stored.
 * The parse function is used to convert the string value to the appropriate type.
 *
 * @param key Key for the configuration option.
 * @param value Pointer to the variable where the parsed value will be stored.
 * @param parse Function to parse the value.
 */
typedef struct {
    const char* key;
    void* value;
    void (*parse)(void* value, const char* str);
} ConfigOption;

/**
 * @brief Boolean type definition.
 * Used to represent true and false values.
 */
typedef enum {
    false, /**< False value */
    true   /**< True value */
} bool;

/**
 * @brief Enumeration for the game status.
 * The game status can be NEXT, WIN, or LOSE.
 */
typedef enum {
    NEXT, /**< Next move*/
    WIN,  /**< Victory */
    LOSE  /**< Defeat */
} GameStatus;

void parse_int(void* value, const char* str);
void parse_string(void* value, const char* str);
void create_game_board(char*** playing_field, int field_size);
void destroy_game_board(char*** playing_field, int field_size);

#endif