#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_hash_map.h"

// Проверка возвращаемых значений не выполняется для упрощения.

// Функция генерации хэша из строки.
size_t hash_s(const void *const _data)
{
    if (_data == NULL) return 0;

    const char *c = (char*)_data;
    size_t hash = 0;
    
    while (*c != 0)
    {
        hash += *(c++);
    }

    return hash;
}

// Функция детального сравнения ключей-строк.
size_t comp_s(const void *const _a,
                   const void *const _b)
{
    if ( (_a == NULL) || (_b == NULL) )
    {
        return 0;
    }

    const char *const a = (char*)_a;
    const char *const b = (char*)_b;

    if (strcmp(a, b) == 0)
    {
        return 1;
    }

    return 0;
}

// Функция печати ключа.
void key_print(const void *const _key)
{
    if (_key == NULL) return;
    
    const char *const key = (char*)_key;
    printf("key: %s ", key);
    
    return;
}

// Функция печати данных.
void data_print(void *const _data)
{
    if (_data == NULL) return;
    
    const float *data = (float*)_data;
    printf("data: %f\n", *data);
    
    return;
}

// Функция удаления данных хэш-отображения.
void del_data(void *const _data)
{
    if (_data == NULL) return;
    
    free(_data);
    
    return;
}

int main(int argc, char **argv)
{
    // Создание хэш-отображения.
    c_hash_map *hash_map = c_hash_map_create(hash_s,
                                             comp_s,
                                             1000,
                                             1.0f);

    // Вставка в хэш-отображение нескольких данных.

    // Ключи для вставки.
    const char *const key_a = "War";
    const char *const key_b = "Goo";
    const char *const key_c = "Door";

    float *data;

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
    c_hash_map_for_each(hash_map, key_print, data_print);
    printf("\n");

    // Удаление из хэш-отображения данных, связанных с ключем key_a ("War").
    c_hash_map_erase(hash_map, key_a, NULL, del_data);

    // Печать содержимого хэш-отображения.
    c_hash_map_for_each(hash_map, key_print, data_print);
    printf("\n");

    // Удаление хэш-отображения.
    // Функция удаления задана только для данных, так как
    // ключи валяются в секции программы "только для чтения".
    c_hash_map_delete(hash_map, NULL, del_data);

    getchar();
    return 0;
}
