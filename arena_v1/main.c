#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

void _arena_init(size_t capacity);
void *arena_alloc(size_t size);
void *arena_realloc(void *old_ptr, size_t old_size, size_t new_size);
void arena_reset();
void arena_free();
void arena_print();

#define arena_init(size) \
    {_arena_init(size); atexit(arena_free);}

int main() {
    arena_init(1024);
    void *ptr = arena_alloc(18);
    arena_print();
    void *ptr_new = arena_realloc(ptr, 18, 20);
    arena_print();
    char *ptr2 = arena_alloc(11);
    arena_print();
    void *ptr3 = arena_alloc(218);
    arena_print();
    void *ptr4 = arena_alloc(1000);
    arena_print();
    void *ptr5 = arena_alloc(1023);
    arena_print();
    void *ptr6 = arena_alloc(1000);
    arena_print();

    printf(
        "\n%p, %p, %p, %p, %p, %p, %p\n", 
        ptr, ptr_new, ptr2, ptr3, ptr4, ptr5, ptr6
    );
}

typedef struct Arena{
    struct Arena *next;
    size_t capacity;
    size_t size;
    uint8_t *data;
} Arena;


static Arena *get_local_arena()
{
    static Arena arena = {
        .next = NULL,
        .capacity = 0,
        .size = 0,
        .data = NULL
    };
    return &arena;
}

void *custom_malloc(size_t size) {
    void *ptr = malloc(sizeof(uint8_t) * size);
    if (ptr == NULL) {
        fprintf(stderr, "Error: Out of RAM");
        exit(1);
    }
    return ptr;
}

void _arena_init(size_t capacity) {
    Arena *arena = get_local_arena();
    if (arena->data == NULL) {
        arena->data = custom_malloc(sizeof(uint8_t) * capacity);
        arena->capacity = capacity;
    }
}

void *arena_alloc(size_t size) {
    Arena *arena = get_local_arena();
    Arena *cur = arena;

    assert(cur->capacity >= size);

    while (cur->size + size > cur->capacity) {
        if (cur->next == NULL) {
            Arena *next = custom_malloc(sizeof(Arena));
            next->capacity = arena->capacity;
            next->size = 0;
            next->next = NULL;
            next->data = custom_malloc(sizeof(uint8_t) * arena->capacity);
            cur->next = next;
        }
        cur = cur->next;
    }

    uint8_t *data = &(cur->data)[cur->size];
    cur->size += size;
    return data;
}

void *arena_realloc(void *old_ptr, size_t old_size, size_t new_size) {
    if (new_size <= old_size) return old_ptr;

    void *new_ptr = arena_alloc(new_size);
    char *new_ptr_char = new_ptr;
    char *old_ptr_char = old_ptr;

    for (size_t i = 0; i < old_size; i++) {
        new_ptr_char[i] = old_ptr_char[i];
    }

    return new_ptr;
}

void arena_reset() {
    Arena *cur = get_local_arena();
    while (cur != NULL) {
        cur->size = 0;
        cur = cur->next;
    }
}

void arena_free() {
    Arena *arena = get_local_arena();

    free(arena->data);
    arena->capacity = 0;
    arena->size = 0;

    Arena *cur = arena->next;
    while (cur != NULL) {
        Arena *nxt = cur->next;
        free(cur->data);
        free(cur);
        cur = nxt;
    }
    
    arena->next = NULL;
}

void arena_print() {
    Arena *cur = get_local_arena();
    while (cur != NULL) {
        printf("Capacity: %zu | Size: %zu | Data Pointer: %s", cur->capacity, cur->size, cur->data);
        cur = cur->next;
    }
}

