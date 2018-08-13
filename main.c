#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_hash_map.h"

// Проверка возвращаемых значений не выполняется для упрощения.

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
    printf("key: %s ", key);

    return;
}

// Функция печати данных.
void print_data_f(void *const _data)
{
    if (_data == NULL) return;

    const float *data = (float*)_data;
    printf("data: %f\n", *data);

    return;
}

// Функция удаления данных хэш-отображения.
void del_data_f(void *const _data)
{
    if (_data == NULL) return;

    free(_data);

    return;
}

int main(int argc, char **argv)
{
    // Создание хэш-отображения.
    c_hash_map *hash_map = c_hash_map_create(hash_key_s,
                                             comp_key_s,
                                             1000,
                                             1.0f);

    // Вставка в хэш-отображение нескольких данных.

    // Ключи для вставки.
    const char *const key_a = "War";
    const char *const key_b = "Goo";
    const char *const key_c = "Door";

    float *data;

    // Создаем данные и вставляем.
    data = (float*)malloc(sizeof(float));
    *data = 1.1f;
    c_hash_map_insert(hash_map, key_a, data);

    data = (float*)malloc(sizeof(float));
    *data = 2.2f;
    c_hash_map_insert(hash_map, key_b, data);

    data = (float*)malloc(sizeof(float));
    *data = 3.3f;
    c_hash_map_insert(hash_map, key_c, data);

    // Печать содержимого хэш-отображения.
    c_hash_map_for_each(hash_map, print_key_s, print_data_f);
    printf("\n");

    // Удаление из хэш-отображения данных, связанных с ключем key_a ("War").
    c_hash_map_erase(hash_map, key_a, NULL, del_data_f);

    // Печать содержимого хэш-отображения.
    c_hash_map_for_each(hash_map, print_key_s, print_data_f);
    printf("\n");

    // Покажем общую информацию.
    printf("slots count: %Iu\n", c_hash_map_slots_count(hash_map));
    printf("nodes count: %Iu\n", c_hash_map_nodes_count(hash_map));

    // Удаление хэш-отображения.
    // Функция удаления задана только для данных (которые в куче), так как
    // ключи валяются в секции программы "только для чтения".
    c_hash_map_delete(hash_map, NULL, del_data_f);

    getchar();
    return 0;
}
