#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef union elem elem_t;

union elem {
  int int_elem;
  char *string_elem;
  char char_elem;
  void *ptr;
};

/// Compares two elements and returns and works lite strcmp
typedef int ioopm_eq_function(elem_t a, elem_t b);

typedef bool ioopm_predicate(elem_t key, elem_t value, void *extra);
typedef void ioopm_apply_function(elem_t key, elem_t *value, void *extra);
