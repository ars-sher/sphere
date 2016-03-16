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
const CharVector EMPTY_CHV = {0, 0, NULL};

int vector_append(CharVector *vector, char value);
int vector_get(const CharVector *vector, size_t index, char *res);
int vector_set(CharVector *vector, size_t index, char value);
void vector_reverse(CharVector* vector);
void vector_free(CharVector *vector);
void vector_print_digits(CharVector *vector);
int vector_init(CharVector *vector);

static bool silent = true; // turns off meaningful errors

// shouldn't called more than twice because it logs "[error]" to stdout
void log_error(const char *errformat, ...) {
    if (!silent) {
        va_list args;
        va_start(args, errformat);
        printf("Error: ");
        vprintf(errformat, args);
        printf("\n");
        va_end(args);
    }
    fprintf(stdout, "[error]");
}

int ull_safe_add(unsigned long long x, unsigned long long y, unsigned long long *res) {
    if (ULLONG_MAX - x <= y) {
        log_error("unsigned long long add overflow");
        return -1;
    }
    *res = x + y;
    return 0;
}

int ull_safe_mul(unsigned long long x, unsigned long long y, unsigned long long *res) {
    if (x != 0 && y != 0 && (ULLONG_MAX / x) + 1 <= y) {
        log_error("unsigned long long multiplication overflow");
        return -1;
    }
    *res = x * y;
    return 0;
}

int letter_to_dec(char letter, char *res) {
  if (isalpha(letter)) {
      *res = (char) tolower(letter) - (char) 'a' + (char) 10;
      return 0;
  }
  else if (isdigit(letter)) {
      *res = letter - (char) '0';
      return 0;
  }
  log_error("symbol %c can't be converted to decimal", letter);
  return -1;
}

int vector_init(CharVector *vector) {
    const size_t initial_capacity = 4;
    vector->size = 0;
    vector->capacity = initial_capacity;
    vector->data = malloc(sizeof((*vector->data)) * vector->capacity);
    if (vector->data == NULL) {
        log_error("CharVector initial malloc failed");
        return -1;
    }
    return 0;
}

int vector_ensure_capacity(CharVector *vector, size_t capacity) {
    while (vector->capacity < capacity) {
        if (vector->capacity >= (SIZE_MAX - 1) / 2) {
            log_error("CharVector error: vectors of size more than SIZE_MAX/2 are not supported");
            vector_free(vector);
            return -1;
        }
        vector->capacity *= 2;
        char *tmp = realloc(vector->data, sizeof(*(vector->data)) * vector->capacity);
        if (tmp == NULL) {
            log_error("CharVector extension failed");
            vector_free(vector);
            return -1;
        }
        vector->data = tmp;
    }
    return 0;
}

int vector_append(CharVector *vector, char value) {
    if (vector_ensure_capacity(vector, vector->size + 1) != 0)
        return -1;
    vector->data[vector->size++] = value;
    return 0;
}

int vector_get(const CharVector *vector, size_t index, char *res) {
    if (index >= vector->size) {
        log_error("CharVector index %zu is out of bounds for vector of size %zu", index, vector->size);
        return -1;
    }
    *res = vector->data[index];
    return 0;
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

void vector_print_digits(CharVector *vector) {
    for (size_t i = 0; i < vector->size; i++) {
        char c; vector_get(vector, i, &c);
        printf("%d", c);
    }
}

int chv_to_number(CharVector* chvp, char base, unsigned long long *res) {
  *res = 0;
  if (chvp->size == 0)
      return 0;
  *res = (unsigned long long int) chvp->data[0];
  if (chvp->size == 1)
      return 0;
  for (size_t i = 1; i < chvp->size; i++) {
      unsigned long long mult;
      if (ull_safe_mul((unsigned int) base, *res, &mult) != 0)
          return -1;
      char vi;
      if (vector_get(chvp, i, &vi) != 0)
          return -1;
      if (ull_safe_add(mult, (unsigned int) vi, res) != 0)
          return -1;
  }
  return 0;
}

int number_to_chv(unsigned long long number, char base, CharVector *res_chvp) {
    if (vector_init(res_chvp) != 0)
        return -1;
    char remainder;
    while (number >= base) {
        remainder = (char) (number % base);
        number = number / base;
        if (vector_append(res_chvp, remainder) != 0) {
            return -1;
        }
//        printf("appended remainder is %d and number is %llu\n", remainder, number);
    }
//    printf("left number is %llu\n", number);
    if (number != 0)
        if (vector_append(res_chvp, (char) number) != 0) {
            return -1;
        }
    vector_reverse(res_chvp);
    return 0;
}

int convert(CharVector* src, char src_base, char dst_base, CharVector *dst) {
    unsigned long long src_number;
    if (chv_to_number(src, src_base, &src_number) != 0)
        return -1;
//    printf("and the number is %llu\n", src_number);
    return number_to_chv(src_number, dst_base, dst);
}

int read_args(CharVector *src_number_chvp, char* psrc_base, char* pdst_base) {
    int src_base; int dst_base;
    int res = scanf("%d", &src_base);
    if (res == EOF || res == 0) {
        log_error("Read crap instead of src_base");
        return -1;
    }
    res = scanf("%d", &dst_base);
    if (res == EOF || res == 0) {
        log_error("Read crap instead of dst_base");
        return -1;
    }
    if (!(2 <= dst_base && dst_base < src_base && src_base <= 36)) {
       log_error("Bases must be: 2 <= dst_base < src_base <= 36");
        return -1;
    }
    *psrc_base = (char) src_base;
    *pdst_base = (char) dst_base;
    char c;
    if (vector_init(src_number_chvp) != 0)
        return -1;
    while ((c = getchar()) != EOF && isspace(c));
    ungetc(c, stdin);
    while ((c = getchar()) != EOF) {
        if (isspace(c)) {
            log_error("number string expected");
            vector_free(src_number_chvp);
            return  -1;
        }
        char dec;
        if (letter_to_dec(c, &dec) != 0) {
            vector_free(src_number_chvp);
            return -1;
        }

        if (dec >= *psrc_base) {
            log_error("Numeral system with base %d can't contain letter %c", *psrc_base, c);
            vector_free(src_number_chvp);
            return -1;
        }
        if (vector_append(src_number_chvp, dec) != 0)
            return -1;
    }
    if (src_number_chvp->size == 0) {
        log_error("number string expected");
        vector_free(src_number_chvp);
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    CharVector src_number_chv; char src_base; char dst_base;
    if (read_args(&src_number_chv, &src_base, &dst_base) != 0)
        return 0; // task definition demands return 0 even in case of failure
    CharVector res_chv;
    if (convert(&src_number_chv, src_base, dst_base, &res_chv) != 0) {
        vector_free(&src_number_chv);
        return 0;
    }
    vector_print_digits(&res_chv);
    vector_free(&src_number_chv);
    vector_free(&res_chv);
    return 0;
}
