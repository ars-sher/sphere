#include <stdio.h>
#include <stdlib.h>

void error(const char *errstr) {
    printf("Error: %s\n", errstr);
    exit(0);
}

void *emalloc(size_t size, const char *errstr){
    void *v = malloc(size);
    if (v == NULL)
        error(errstr);
    return v;
}

void *erealloc(void* ptr, size_t size, const char *errstr) {
    void *v = realloc(ptr, size);
    if (v == NULL)
        error(errstr);
    return v;
}

char letter_to_dec(char letter) {

}

char dec_to_letter(char dec) {

}

typedef struct {
    size_t size;  // actual size
    size_t capacity;  // allocated slots
    char *data;
} CharVector;

void vector_init(CharVector *vector) {
    const size_t initial_capacity = 4;
    vector->size = 0;
    vector->capacity = initial_capacity;
    vector->data = emalloc(sizeof((*vector->data)) * vector->capacity, "CharVector initial malloc failed"); // TODO check sizeof
}

void vector_check_and_extend(CharVector *vector) {
    if (vector->size == vector->capacity) {
        vector->capacity *= 2;
        vector->data = erealloc(vector->data, sizeof(*(vector->data)) * vector->capacity,
                                "CharVector extension failed");
    }
    else if (vector->size > vector->capacity) {
        error("CharVector fatal error: size is greater than capacity");
    }
}

void vector_append(CharVector *vector, char value) {
    vector_check_and_extend(vector);
    vector->data[vector->size++] = value;
}

char vector_get(CharVector *vector, size_t index) {
    if (index >= vector->size) {
        char errstr[200];
        sprintf(errstr, "CharVector index %zu is out of bounds for vector of size %zu", index, vector->size);
        error(errstr);
    }
    return vector->data[index];
}

void vector_set(CharVector *vector, size_t index, char value) {
    while (index >= vector->capacity) {
        for (size_t i = vector->size; i < vector->capacity; i++)
            vector->data[i] = 0;
        vector->size = vector->capacity;
        vector_check_and_extend(vector);
    }
    for (size_t i = vector->size; i < index; i++)
        vector->data[i] = 0;
    vector->data[index] = value;
    vector->size = index + 1;
}

void vector_free(CharVector *vector) {
    free(vector->data);
}

void vector_print(CharVector *vector) {
    for (size_t i = 0; i < vector->size; i++)
        printf("%d ", vector->data[i]);
    printf("\n");
}

int main() {
    CharVector chv;
    vector_init(&chv);
    char i;
    for (i = 100; i > 50; i--) {
        vector_append(&chv, i);
    }
    vector_set(&chv, 101000, 24);
//    vector_print(&chv);
    vector_free(&chv);
    printf("Hello, World!\n");
    return 0;
}

// add one digit in system x to another; (char, char, x) -> (char, char). first res char is 0 or 1, in fact

// add one number in system x to another; (char*, char* ) -> char*

// multiply one number in system x to another; (char*, char*) -> char*

// conversion, likely recursive (int, int, char*) -> char*
