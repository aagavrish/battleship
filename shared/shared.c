/*! @file shared.c
Файл с общими функциями для сервера и клиента игры "Морской бой".
@author Гавриш А.А.
@date 13.04.2024 */

#include "shared.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Функция для парсинга целочисленного значения.
 *
 * @param value Указатель на переменную, в которую будет записан результат.
 * @param str Строка, содержащая значение.
 */
void parse_int(void* value, const char* str) {
    *(int*)value = atoi(str);

    if (*(int*)value == 0) {
        printf("ERROR: invalid value for key\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Функция для парсинга строкового значения.
 *
 * @param value Указатель на переменную, в которую будет записан результат.
 * @param str Строка, содержащая значение.
 */
void parse_string(void* value, const char* str) {
    char* newline = strchr(str, '\n');
    if (newline) {
        *newline = '\0';
    }

    strcpy((char*)value, str);
}

/**
 * @brief Функция для создания игрового поля.
 *
 * @param playing_field Указатель на указатель на игровое поле.
 * @param field_size Размер поля.
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
 * @brief Функция для уничтожения игрового поля.
 *
 * @param playing_field Указатель на указатель на игровое поле.
 * @param field_size Размер поля.
 */
void destroy_game_board(char*** playing_field, int field_size) {
    for (int i = 0; i < field_size; ++i) {
        free((*playing_field)[i]);
    }
    free(*playing_field);
}