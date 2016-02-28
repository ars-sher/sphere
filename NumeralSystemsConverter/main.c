#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <stdbool.h>

// vector of chars interface
typedef struct {
    size_t size;  // actual size
    size_t capacity;  // allocated slots
    char *data;
} CharVector;

CharVector vector_from_str(const char *str);
CharVector vector_from_char(char c);
char vector_append(CharVector *vector, char value);
CharVector vector_construct();
char vector_get(const CharVector *vector, size_t index);
char vector_set(CharVector *vector, size_t index, char value);
void vector_reverse(CharVector* vector);
void vector_free(CharVector *vector);
void vector_print(CharVector *vector);
void vector_print_letters(CharVector *vector);
void vector_init(CharVector *vector);

bool silent = false; // turns off meaningful errors
CharVector src_number_chv;
CharVector res_chv;

void cleanup() {
    vector_free(&src_number_chv);
    vector_free(&res_chv);
}

void log_error(const char *errformat, ...) {
    if (!silent) {
        va_list args;
        va_start(args, errformat);
        printf("Error: ");
        vprintf(errformat, args);
        printf("\n");
        va_end(args);
    }
}

void error(const char *errformat, ...) {
    if (!silent) {
        va_list args;
        va_start(args, errformat);
        printf("Error: "); //TODO
        vprintf(errformat, args);
        printf("\n");
        va_end(args);
    }
    fprintf(stdout, "[error]");
    cleanup();
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
        free(ptr);
    }
    return v;
}

unsigned long long ull_safe_add(unsigned long long x, unsigned long long y) {
    if (ULLONG_MAX - x <= y)
        error("unsigned long long add overflow");
    return x + y;
}

unsigned long long ull_safe_mul(unsigned long long x, unsigned long long y) {
    if (x != 0 && y != 0 && (ULLONG_MAX / x) + 1 <= y) {
        error("unsigned long long multiplication overflow");
    }
    return x * y;
}

char letter_to_dec(char letter) {
  if (isalpha(letter))
      return (char)tolower(letter) - (char)'a' + (char)10;
  else if (isdigit(letter))
      return letter - (char)'0';
  error("symbol %c can't be converted to decimal", letter);
  return 0; // must never reach this
}

char dec_to_letter(char dec) {
    if (dec > 36 || dec < 0) // 36 = 26 english letters + 10 digits
      error("decimal %d can't be converted to letter", dec);
    if (dec < 10)
        return (char)'0' + dec;
    else
        return (char)'a' + dec - (char)10;
}

void vector_init(CharVector *vector) {
    const size_t initial_capacity = 4;
    vector->size = 0;
    vector->capacity = initial_capacity;
    vector->data = emalloc(sizeof((*vector->data)) * vector->capacity, "CharVector initial malloc failed");
}

CharVector vector_construct() {
    CharVector chv;
    vector_init(&chv);
    return chv;
}

void vector_check_and_extend(CharVector *vector) {
    if (vector->size == vector->capacity) {
        if (vector->capacity >= (SIZE_MAX - 1) / 2)
            error("CharVector error: vectors of size more than SIZE_MAX/2 are not supported");
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

// english letter is represented as a digit
CharVector vector_from_char(char c) {
    CharVector v = vector_construct();
    vector_append(&v, letter_to_dec(c));
    return v;
}

// str is usual c string, it must be null-terminated, e.g. cre15\0. Each english letter is interpreted as a digit.
CharVector vector_from_str(const char *str) {
    CharVector v = vector_construct();
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

void swap_chars(char *c1, char*c2) {
    char tmp = *c1; *c1 = *c2; *c2 = tmp;
}

// in-place reverse of the vector
void vector_reverse(CharVector* vector) {
    char* start = vector->data;
    char* end = vector->data + vector->size - 1;
    for (size_t i = 0; i < vector->size / 2; i++)
        swap_chars(start++, end--);
}

void vector_free(CharVector *vector) {
    free(vector->data);
    vector->data = NULL;
    vector->size = vector->capacity = 0;
}

void vector_print(CharVector *vector) {
    for (size_t i = 0; i < vector->size; i++)
        printf("%d ", vector_get(vector, i));
    printf("\n");
}

void vector_print_letters(CharVector *vector) {
    for (size_t i = 0; i < vector->size; i++)
        printf("%c", dec_to_letter(vector_get(vector, i)));
}

unsigned long long chv_to_number(CharVector* chv, char base) {
  unsigned long long res = 0;
  if (chv->size == 0)
      return 0;
  if (chv->size == 1)
      return chv->data[0];
  res = chv->data[0];
  for (size_t i = 1; i < chv->size; i++)
      res = ull_safe_add(ull_safe_mul(base, res), vector_get(chv, i));
  return res;
}

CharVector number_to_chv(unsigned long long number, char base) {
    res_chv = vector_construct();
    char remainder = base;
    while (number >= base) {
        remainder = number % base;
        number = number / base;
        vector_append(&res_chv, remainder);
//        printf("appened remainder is %d and number is %llu\n", remainder, number);
    }
    if (number != 0)
        vector_append(&res_chv, number);
    vector_reverse(&res_chv);
    return res_chv;
}

CharVector convert(CharVector* src, char src_base, char dst_base) {
    unsigned long long src_number = chv_to_number(src, src_base);
//    printf("and the number is %llu\n", src_number);
    return number_to_chv(src_number, dst_base);
}

void show_usage_parse_arg() {
    printf("argc-argv usage: main <src_base> <dst_base> <number>\n");
    printf("Correct example, with args from stdin:\n");
    printf("  gcc -O2 -Werror -std=c99 main.c -o main && echo -n \"10 2 42\" | ./main \n");
}

long stolerrcheck(char* str) {
    char *endptr;
    long value = strtol(str, &endptr, 10);
    if (endptr == str || *endptr != '\0')
        error("The src and dst base must be int");
    return value;
}

// we would need that in case of passing args to executable
CharVector parse_args(int argc, char *argv[], char* psrc_base, char* pdst_base) {
    if (argc != 4) {
        show_usage_parse_arg();
        error("Number of arguments %d is not equal 4", argc);
    }
    *psrc_base = stolerrcheck(argv[1]);
    *pdst_base = stolerrcheck(argv[2]);
    if (!(2 <= *pdst_base && *pdst_base < *psrc_base && *psrc_base <= 36))
        error("Bases must be: 2 <= dst_base < src_base <= 36");
    src_number_chv = vector_construct();
    char* number_str = argv[3];
    while (*number_str ) {
        char dec = letter_to_dec(*number_str);
        if (dec >= *psrc_base)
            error("Numeral system with base %d can't contain letter %c", *psrc_base, *number_str);
        vector_append(&src_number_chv, dec);
        number_str++;
    }
    return src_number_chv;
}

CharVector read_args(char* psrc_base, char* pdst_base) {
    int src_base; int dst_base;
    int res = scanf("%d", &src_base);
    if (res == EOF || res == 0)
        error("Read crap instead of src_base");
    res = scanf("%d", &dst_base);
    if (res == EOF || res == 0)
        error("Read crap instead of dst_base");
    if (!(2 <= dst_base && dst_base < src_base && src_base <= 36))
        error("Bases must be: 2 <= dst_base < src_base <= 36");
    *psrc_base = (char) src_base;
    *pdst_base = (char) dst_base;
    char c;
    src_number_chv = vector_construct();
    while ((c = getchar()) != EOF && isspace(c));
    ungetc(c, stdin);
    while ((c = getchar()) != EOF) {
        if (isspace(c))
            error("number string expected");
        char dec = letter_to_dec(c);
        if (dec >= *psrc_base)
            error("Numeral system with base %d can't contain letter %c", *psrc_base, c);
        vector_append(&src_number_chv, dec);
    }
    if (src_number_chv.size == 0)
        error("number string expected");
    return src_number_chv;
}

int main(int argc, char *argv[]) {
    char src_base; char dst_base;
//    CharVector src_number_chv = parse_args(argc, argv, &src_base, &dst_base);
    src_number_chv = read_args(&src_base, &dst_base);
    res_chv = convert(&src_number_chv, src_base, dst_base);
    vector_print_letters(&res_chv);
    cleanup();
    return 0;
}
