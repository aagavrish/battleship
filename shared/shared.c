/*! @file shared.c
File containing shared functions and structures for the battleship game.
The shared functions include parsing configuration options and creating and destroying the game board.
The shared structures include the configuration options and the game board.
The game board is a 2D array of characters, where each cell represents a square on the board.
The game board is used by both the client and the server to keep track of the game state.
@author Gavrish A.A.
@date 13.04.2024 */

#include "shared.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Function to parse an integer value. The value is stored in the provided variable.
 *
 * @param value Pointer to the variable where the value will be stored.
 * @param str String containing the value.
 * @return void
 */
void parse_int(void* value, const char* str) {
    *(int*)value = atoi(str);

    if (*(int*)value == 0) {
        printf("ERROR: invalid value for key\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Function to parse a string value. The value is stored in the provided variable.
 * The function also removes the newline character from the end of the string.
 *
 * @param value Pointer to the variable where the value will be stored.
 * @param str String containing the value.
 * @return void
 */
void parse_string(void* value, const char* str) {
    char* newline = strchr(str, '\n');
    if (newline) {
        *newline = '\0';
    }

    strcpy((char*)value, str);
}

/**
 * @brief Function to create the game board. The game board is a 2D array of characters.
 * Each cell represents a square on the board, and the initial value is '*'.
 * The game board is used by both the client and the server to keep track of the game state.
 * The game board is dynamically allocated based on the field size.
 *
 * @param playing_field Pointer to the pointer to the game board.
 * @param field_size Size of the game board.
 * @return void
 */
void create_game_board(char*** playing_field, int field_size) {
    *playing_field = (char**)malloc(field_size * sizeof(char*));
    for (int i = 0; i < field_size; ++i) {
        (*playing_field)[i] = (char*)malloc(field_size * sizeof(char));
        for (int j = 0; j < field_size; ++j) {
            (*playing_field)[i][j] = '*';
        }
    }
}

/**
 * @brief Function to destroy the game board.
 *
 * @param playing_field Pointer to the pointer to the game board.
 * @param field_size Size of the game board.
 * @return void
 */
void destroy_game_board(char*** playing_field, int field_size) {
    for (int i = 0; i < field_size; ++i) {
        free((*playing_field)[i]);
    }
    free(*playing_field);
}