#include "c_hash_map.h"

// �������� ������� ���-�����������.
// � ������ ������ ���������� ��������� �� ��������� �����������.
// � ������ ������ ���������� NULL.
// ��������� ������� ����������� � ����� ������.
c_hash_map *c_hash_map_create(size_t (*const _hash_func)(const void *const _key),
                              size_t (*const _comp_func)(const void *const _a,
                                                         const void *const _b),
                              const size_t _key_size,
                              const size_t _data_size,
                              const size_t _slots_count,
                              const float _max_load_factor)
{
    if (_hash_func == NULL) return NULL;
    if (_comp_func == NULL) return NULL;
    if (_key_size == 0) return NULL;
    if (_data_size == 0) return NULL;
    if (_max_load_factor <= 0.0f) return NULL;

    void *new_slots = NULL;

    if (_slots_count > 0)
    {
        // ��������� ������ ����� ������.
        const size_t new_slots_size = _slots_count * sizeof(void*);
        if ( (new_slots_size == 0) ||
             (new_slots_size / _slots_count != sizeof(void*)) )
        {
            return NULL;
        }

        // ���������� �������� ������ ��� ����� �����.
        new_slots = malloc(new_slots_size);
        if (new_slots == NULL) return NULL;


    }
}
