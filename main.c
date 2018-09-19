#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_hash_map.h"

// Функция генерации хэша из ключа-строки.
size_t hash_key_s(const void *const _key)
{
    if (_key == NULL) return 0;

    const char *c = (char*)_key;
    size_t hash = 0;

    while (*c != 0)
    {
        hash += *(c++);
    }

    return hash;
}

// Функция детального сравнения ключей-строк.
size_t comp_key_s(const void *const _key_a,
                  const void *const _key_b)
{
    if ( (_key_a == NULL) || (_key_b == NULL) )
    {
        return 0;
    }

    const char *const key_a = (char*)_key_a;
    const char *const key_b = (char*)_key_b;

    if (strcmp(key_a, key_b) == 0)
    {
        return 1;
    }

    return 0;
}

// Функция печати ключа.
void print_key_s(const void *const _key)
{
    if (_key == NULL) return;

    const char *const key = (char*)_key;
    printf("[%s]: ", key);

    return;
}

// Функция печати данных.
void print_data_f(void *const _data)
{
    if (_data == NULL) return;

    const float *data = (float*)_data;
    printf("%f\n", *data);

    return;
}

int main(int argc, char **argv)
{
    size_t error;
    c_hash_map *hash_map;

    // Попытаемся создать хэш-отображение.
    hash_map = c_hash_map_create(hash_key_s, comp_key_s, 10, 0.5f, &error);
    // Если произошла ошибка, покажем ее.
    if (hash_map == NULL)
    {
        printf("create error: %Iu\n", error);
        printf("Program end.\n");
        getchar();
        return -1;
    }

    // Добавим в хэш-отображене один элемент.
    const char *const key_1 = "First key";
    const float data_1 = 1.f;
    {
        const ptrdiff_t r_code = c_hash_map_insert(hash_map, key_1, &data_1);
        // Покажем результат операции.
        printf("insert[%s, %f]: %Id\n", key_1, data_1, r_code);
    }
    // Вновь добавим тот же самый элемент.
    {
        const ptrdiff_t r_code = c_hash_map_insert(hash_map, key_1, &data_1);
        // Покажем результат операции.
        printf("insert[%s, %f]: %Id\n", key_1, data_1, r_code);
    }

    // Добавим другой элемент.
    const char *const key_2 = "Second key";
    const float data_2 = 2.f;
    {
        const ptrdiff_t r_code = c_hash_map_insert(hash_map, key_2, &data_2);
        // Покажем результат операции.
        printf("insert[%s, %f]: %Id\n", key_2, data_2, r_code);
    }

    // При помощи обхода хэш-отображения покажем содержимое каждого элемента (каждой пары).
    {
        const ptrdiff_t r_code = c_hash_map_for_each(hash_map, print_key_s, print_data_f);
        // Если возникла ошибка, покажем ее.
        if (r_code < 0)
        {
            printf("for each error, r_code: %Id\n", r_code);
            printf("Program end.\n");
            getchar();
            return -2;
        }
    }

    // Покажем количество пар в хэш-отображении.
    {
        error = 0;
        const ptrdiff_t p_count = c_hash_map_pairs_count(hash_map, &error);
        // Если произошла ошибка, покажем ее.
        if ( (p_count == 0) && (error > 0) )
        {
            printf("pairs count error: %Iu\n", error);
            printf("Program end.\n");
            getchar();
            return -3;
        }
        // Покажем количество пар.
        printf("p_count: %Iu\n", p_count);
    }

    // Удалим хэш-отображение.
    {
        const ptrdiff_t r_code = c_hash_map_delete(hash_map, NULL, NULL);
        // Если произошла ошибка, покажем ее.
        if (r_code < 0)
        {
            printf("delete error, r_code: %Id\n", r_code);
            printf("Program end.\n");
            getchar();
            return -4;
        }
    }

    getchar();
    return 0;
}
