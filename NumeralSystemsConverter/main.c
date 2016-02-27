#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

void log_error(const char *errformat, ...) {
    va_list args;
    va_start(args, errformat);
    printf("Error: " );
    vprintf(errformat, args);
    printf("\n");
    va_end(args);
}

void error(const char *errformat, ...) {
    va_list args;
    va_start(args, errformat);
    log_error(errformat, args);
    va_end(args);
    exit(0);
}

void *emalloc(size_t size, const char *errstr){
    void *v = malloc(size);
    if (v == NULL) {
        log_error(errstr);
        error("malloc failed while allocating %d bytes", size);
    }
    return v;
}

void *erealloc(void* ptr, size_t size, const char *errstr) {
    void *v = realloc(ptr, size);
    if (v == NULL) {
        log_error(errstr);
        error("realloc failed while allocating %d bytes", size);
    }
    return v;
}

char letter_to_dec(char letter) {
  if (isalpha(letter))
      return (char)tolower(letter) - (char)'a' + (char)10;
  else if (isdigit(letter))
      return letter - (char)'0';
  else {
      error("symbol %c can't be converted to decimal", letter);
  }
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
    vector->data = emalloc(sizeof((*vector->data)) * vector->capacity, "CharVector initial malloc failed");
}

void vector_check_and_extend(CharVector *vector) {
    if (vector->size == vector->capacity) {
        vector->capacity *= 2;
        vector->data = erealloc(vector->data, sizeof(*(vector->data)) * vector->capacity,
                                "CharVector extension failed");
    }
    else if (vector->size > vector->capacity) {
        error("CharVector fatal error: size %d is greater than capacity %d", vector->size, vector->capacity);
    }
}

char vector_append(CharVector *vector, char value) {
    vector_check_and_extend(vector);
    vector->data[vector->size++] = value;
    return value;
}

// str must be null-terminated, e.g. cre15\0. Each latin letter is interpreted as a digit.
CharVector vector_from_str(const char *str) {
    CharVector v;
    vector_init(&v);
    while (*str) {
      vector_append(&v, letter_to_dec(*str));
      str++;
    }
    return v;
}

char vector_get(const CharVector *vector, size_t index) {
    if (index >= vector->size)
        error("CharVector index %zu is out of bounds for vector of size %zu", index, vector->size);
    return vector->data[index];
}

char vector_set(CharVector *vector, size_t index, char value) {
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
    return value;
}

void vector_free(CharVector *vector) {
    free(vector->data);
    vector->data = NULL;
    vector->size = vector->capacity = 0;
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
    vector_set(&chv, 101, 24);
//    vector_print(&chv);
    vector_free(&chv);
    printf("Hello, World!\n");
    return 0;
}

// add one digit in system x to another; (char, char, x) -> (char, char). first res char is 0 or 1, in fact

// add one number in system x to another; (char*, char* ) -> char*

// multiply one number in system x to another; (char*, char*) -> char*

// conversion, likely recursive (int, int, char*) -> char*
