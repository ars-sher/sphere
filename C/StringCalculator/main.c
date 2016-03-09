#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

//errors functions interface
static bool silent = true; // turns off meaningful errors
void log_error(const char *errformat, ...);
void error(const char *errformat, ...);
void cleanup(); // free all allocated memory

// vector of chars interface
typedef struct {
    size_t size;  // actual size
    size_t capacity;  // allocated slots
    char *data;
} CharVector;
const CharVector EMPTY_CHV = {0, 0, NULL};

CharVector vector_from_str(const char *str);
CharVector vector_from_char(char c);
char vector_append(CharVector *vector, char value);
CharVector vector_construct();
char vector_get(const CharVector *vector, size_t index);
char vector_set(CharVector *vector, size_t index, char value);
void vector_reverse(CharVector* vector);
void vector_free(CharVector *vector);
void vector_print(const CharVector *vector);
void vector_print_chars(const CharVector *vector);
// end of vector of chars interface

// stack of CharVector's interfacce
struct StackElement {
    CharVector data;
    struct StackElement *next;
};
typedef struct StackElement StackElement;

typedef struct {
    StackElement *top;
    int size;
} Stack;

bool stack_is_empty(Stack *sp);
CharVector stack_pop(Stack *sp);
void stack_push(Stack *sp, CharVector el);
Stack stack_construct();
// end of stack interface

// *alloc wrappers
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

//*********************************** start of stack implementation
bool stack_is_empty(Stack *sp) {
    return (sp->size == 0);
}

CharVector stack_pop(Stack *sp) {
    if (stack_is_empty(sp))
        error("an attempt to pop from empty stack");
    StackElement *tmp = sp->top;
    CharVector popped = tmp->data;
    sp->top = sp->top->next;
    free(tmp);
    sp->size--;
    return popped;
}

void stack_push(Stack *sp, CharVector el) {
//    printf("Adding following string to stack:\n");
//    vector_print_chars(&el);
//    printf("\n");
    StackElement *new_elp = (StackElement*) emalloc(sizeof(StackElement), "memory allocation for stack element failed");
    new_elp->data = el;
    new_elp->next = sp->top;
    sp->top = new_elp;
    sp->size++;
}

Stack stack_construct() {
    Stack res = {.top = NULL, .size = 0};
    return res;
}
//*********************************** end of stack implementation

//*********************************** start of vector implementation
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

void vector_ensure_capacity(CharVector *vector, size_t capacity) {
    while (vector->capacity < capacity) {
        if (vector->capacity >= (SIZE_MAX - 1) / 2)
            error("CharVector error: vectors of size more than SIZE_MAX/2 are not supported");
        vector->capacity *= 2;
        vector->data = erealloc(vector->data, sizeof(*(vector->data)) * vector->capacity,
                                "CharVector extension failed");
    }
}

char vector_append(CharVector *vector, char value) {
    vector_ensure_capacity(vector, vector->size + 1);
    vector->data[vector->size++] = value;
    return value;
}

// english letter is represented as a digit
CharVector vector_from_char(char c) {
    CharVector v = vector_construct();
    vector_append(&v, c);
    return v;
}

CharVector vector_from_str(const char *str) {
    CharVector v = vector_construct();
    while (*str) {
        vector_append(&v, *str);
        str++;
    }
    return v;
}

char vector_get(const CharVector *vector, size_t index) {
    if (index >= vector->size)
        error("CharVector index %zu is out of bounds for vector of size %zu", index, vector->size);
    return vector->data[index];
}

// function doesn't fill with zeros expanded space
char vector_set(CharVector *vector, size_t index, char value) {
    vector_ensure_capacity(vector, index + 1);
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

void vector_print(const CharVector *vector) {
    for (size_t i = 0; i < vector->size; i++)
        printf("%d ", vector_get(vector, i));
    printf("\n");
}

void vector_print_chars(const CharVector *vector) {
    for (size_t i = 0; i < vector->size; i++)
        printf("%c", vector_get(vector, i));
}

CharVector vector_concatenate(const CharVector *left, const CharVector *right) {
  CharVector res = vector_construct();
  size_t res_size = left->size + right->size;
  vector_ensure_capacity(&res, res_size);
  memcpy(res.data, left->data, left->size);
  memcpy(res.data + left->size, right->data, right->size);
  res.size = res_size;
  return res;
}

CharVector vector_multiply(const CharVector *v, int factor) {
  CharVector res = vector_construct();
  size_t res_size = v->size * factor;
  vector_ensure_capacity(&res, res_size);
  for (int i = 0; i < factor; i++)
      memcpy(res.data + v->size * i, v->data, v->size);
  res.size = res_size;
  return res;
}
//************************************ end of vector implementation


// error functions implementation
static Stack pvectors_to_free = {.top = NULL, .size = 0};
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
        printf("Error: ");
        vprintf(errformat, args);
        printf("\n");
        va_end(args);
    }
    fprintf(stdout, "[error]");
    cleanup();
    exit(0);
}

void cleanup() {
//    printf("stack size is %d, starting cleanup\n", pvectors_to_free.size);
    while (!stack_is_empty(&pvectors_to_free)) {
        CharVector popped = stack_pop(&pvectors_to_free);
        vector_free(&popped);
    }
}

// contains pointers to vectors needed to be freed before exit
typedef enum {
    STRING, NUMBER, END=EOF,
    PLUS='+', MUL='*',
    LP='(', RP=')', QUOTES='"', BROKEN
} token_kind;

// we look always one token ahead
typedef struct {
    token_kind tk;
    CharVector string;
    int number;
} LexToken;
LexToken default_token() {
    LexToken def_token = {END, EMPTY_CHV, 0};
    return def_token;
}
void print_lex_token(LexToken lt) {
    if (lt.tk == STRING) {
        printf("String ");
        vector_print_chars(&lt.string);
        printf("\n");
    }
    else if (lt.tk == NUMBER)
        printf("Number %d\n", lt.number);
    else
        printf("Symbol %c\n", lt.tk);
}

LexToken lt_from_char(char c) {
    LexToken token = default_token();
    token.tk = c;
    return token;
}

LexToken get_token() {
    char c;
    static bool isInString = false; // true, if we are inside " "

    do { // skip whitespaces
        if ((c = getchar()) == EOF) return lt_from_char(c);
    } while (isspace(c) && !isInString);
//    printf("%c\n", c);

    switch (c) {
        case '-':
        case '+':
        case '*':
        case '(':
        case ')':
            if (isInString)
                error("special symbol %c inside string", c); // parser thinks that -,+,*,(,) can't be inside string
            return lt_from_char(c);
        case '"':
            isInString = !isInString;
            return lt_from_char(c);
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            if (!isInString) {
                ungetc(c, stdin);
                int number;
                int res = scanf("%d", &number);
                if (res != 1 || number == -1) // number = -1 on overflow
                    error("scanf result is not 1 or number is -1");
                LexToken lt = {NUMBER, EMPTY_CHV, number};
                return lt;
            }
        default: {
            LexToken lt = {STRING, vector_construct(), 0};
            while (c != EOF && c != '"') {
                vector_append(&lt.string, c);
                c = getchar();
            }
            ungetc(c, stdin);
            return lt;
        }
    }
    error("bad token");
}

CharVector expression(LexToken *curr_tok_ptr);

CharVector primary(LexToken *curr_tok_ptr) {
  CharVector res;
  if (curr_tok_ptr->tk == QUOTES) {
//      print_lex_token(*curr_tok_ptr);
      *curr_tok_ptr = get_token();
      if (curr_tok_ptr->tk == QUOTES) { // empty string
          res = vector_construct();
          stack_push(&pvectors_to_free, res);
      }
      else {
//      print_lex_token(*curr_tok_ptr);
          if (curr_tok_ptr->tk != STRING) {
              error("string after \" expected");
          }
          res = curr_tok_ptr->string;
          stack_push(&pvectors_to_free, res);
          *curr_tok_ptr = get_token();
          if (curr_tok_ptr->tk != QUOTES) {
              error("\" after string expected");
          }
      }
      *curr_tok_ptr = get_token();
      return res;
  }
  else if (curr_tok_ptr->tk == LP) {
      *curr_tok_ptr = get_token();
      CharVector expr = expression(curr_tok_ptr);
      if (curr_tok_ptr->tk != RP) {
          error(") expected");
      }
      *curr_tok_ptr = get_token();
      return expr;
  }
  else {
      error("primary expected");
      return EMPTY_CHV; // must never reach this
  }
}

CharVector term(LexToken *curr_tok_ptr) {
  int left_number = 1; int right_number = 1;
  while (curr_tok_ptr->tk == NUMBER) { // read multiplied numbers in left, if any
      left_number *= curr_tok_ptr->number;
      *curr_tok_ptr = get_token(); // eat *
      if (curr_tok_ptr->tk != MUL) {
          error("Expected * after number");
      }
      *curr_tok_ptr = get_token();
  }
  CharVector prim = primary(curr_tok_ptr); // read string itself
  while (curr_tok_ptr->tk == MUL) { // read multiplied numbers in right, if any
      *curr_tok_ptr = get_token();
      if (curr_tok_ptr->tk != NUMBER) {
          error("Expected number after *");
      }
      right_number *= curr_tok_ptr->number;
      *curr_tok_ptr = get_token();
  }
  CharVector res = vector_multiply(&prim, left_number*right_number);
  stack_pop(&pvectors_to_free);
  stack_push(&pvectors_to_free, res);
  vector_free(&prim);
  return res;
}

CharVector expression(LexToken *curr_tok_ptr) {
    CharVector left = term(curr_tok_ptr);
    while (1948) {
        if (curr_tok_ptr->tk == PLUS) {
            *curr_tok_ptr = get_token(); // eat +
            CharVector right = term(curr_tok_ptr);
            CharVector tmp = vector_concatenate(&left, &right);
            stack_pop(&pvectors_to_free); stack_pop(&pvectors_to_free);
            stack_push(&pvectors_to_free, tmp);
            vector_free(&left); vector_free(&right);
            left = tmp;
        }
        else {
            return left;
        }
    }
}

int main() {
    CharVector res;
    LexToken lt = get_token();
    if (lt.tk != END) {
        res = expression(&lt);
        if (lt.tk != END)
            error("EOF expected");
        printf("\"");
        vector_print_chars(&res);
        printf("\"");
        vector_free(&res);
        stack_pop(&pvectors_to_free);
    }
    cleanup();
    return 0;
}