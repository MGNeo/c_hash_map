#include <stdio.h>
#include <stdlib.h>
#include "c_hash_map.h"

// ������� ���������� ���� ��� ������.
size_t hash_func_s(const void *const _key)
{
    if (_key == NULL) return 0;

    const char *key = *((char**)_key);
    size_t hash = 0;

    while(*key != 0)
    {
        hash += *key;
        ++key;
    }

    return hash;
}

// ������� ���������� ��������� ������-�����.
size_t comp_func_s(const void *const _a,
                   const void *const _b)
{
    if ( (_a == NULL) || (_b == NULL) ) return 0;

    const char *const p_string_a = *((char**)_a);
    const char *const p_string_b = *((char**)_b);

    if (strcmp(p_string_a, p_string_b) == 0)
    {
        return 1;
    }

    return 0;
}

// ������� ������ ���������� ����� �������� ���-�����������.
void print_key_func_s(const void *const _key)
{
    const char *const p_string = *((char**)_key);
    printf("KEY: %s ", p_string);

    return;
}

// ������� ������ float ������ �������� ���-�����������.
void print_data_func_f(const void *const _data)
{
    const float value = *((float*)_data);
    printf("DATA: %f\n", value);

    return;
}

int main()
{
    // �������� ������� ���-����������� (���� - ��������� �� ������, ������ - float).
    c_hash_map *hash_map = c_hash_map_create(hash_func_s,
                                             comp_func_s,
                                             sizeof(char*),
                                             sizeof(float),
                                             1000,
                                             1.0f);

    // ������� ��������� � ��� ���������.
    const char *const one_key = "one";
    const char *const two_key = "two";
    const char *const three_key = "three";
    float value;

    value = 1.0f;
    c_hash_map_insert(hash_map, &one_key, &value);

    value = 2.0f;
    c_hash_map_insert(hash_map, &two_key, &value);

    value = 3.0f;
    c_hash_map_insert(hash_map, &three_key, &value);

    // ������ ����������� ���-�����������.
    c_hash_map_for_each(hash_map, print_key_func_s, print_data_func_f);

    printf("\n");

    // ������� ���-�����������.
    c_hash_map_resize(hash_map, 10);

    // �������� ������ �� ����� two_key.
    value = 3.1415f;
    c_hash_map_insert(hash_map, &two_key, &value);

    // ������ ����������� ���-�����������.
    c_hash_map_for_each(hash_map, print_key_func_s, print_data_func_f);

    // �������� ���-�����������.
    c_hash_map_delete(hash_map, NULL, NULL);

    getchar();
    return 0;
}
