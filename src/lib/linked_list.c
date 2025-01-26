#include "linked_list.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct node {
  elem_t value;
  struct node *next;
} node_t;

struct list {
  node_t *first;
  node_t *last;
  ioopm_eq_function *equal_func;
  size_t size;
};

node_t *create_node(elem_t value, node_t *next) {
  node_t *new_node = malloc(sizeof(node_t));
  new_node->value = value;
  new_node->next = next;
  return new_node;
}

ioopm_list_t *ioopm_linked_list_create(ioopm_eq_function *equal_func) {
  ioopm_list_t *list = calloc(1, sizeof(ioopm_list_t));
  if (list) // if we succeded with allocating memory
  {
    node_t *sentinal_node =
        calloc(1, sizeof(node_t)); // could have used create_node aswell
    sentinal_node->next = NULL;
    list->first = sentinal_node;
    list->last = sentinal_node;
    list->equal_func = equal_func;
  }
  return list;
}

void node_destroy(node_t *node) {
  free(node);
  node = NULL;
}

void ioopm_linked_list_clear(ioopm_list_t *list) {
  node_t *ptr = list->first->next;
  node_t *prev = NULL;
  while (ptr) {
    node_destroy(prev);
    prev = ptr;
    ptr = ptr->next;
  }
  // free last
  node_destroy(prev);
  // move last to sentinal again
  list->last = list->first;
  // reset sentinal
  list->first->next = NULL;
  // reset size
  list->size = 0;
}

bool ioopm_linked_list_is_empty(ioopm_list_t *list) {
  // check if first and last points to the sentinal
  return list->size == 0;
}

void ioopm_linked_list_destroy(ioopm_list_t *list) {
  ioopm_linked_list_clear(list);
  // free sentinal
  free(list->first);
  // free list
  free(list);
}

static node_t *find_previous_node(ioopm_list_t *list, int index) {
  if (!(index >= 0 && (size_t)index <= list->size)) {
    // if invalid index return null
    return NULL;
  }

  // incase last index
  if ((size_t)index == list->size) {
    return list->last;
  }

  node_t *ptr = list->first;
  for (int i = 0; i < index; i++) {
    ptr = ptr->next;
  }
  return ptr;
}

static node_t **find_previous_ptr(ioopm_list_t *list, int index) {

  if (index < 0 || (size_t)index > list->size) {
    return NULL;
  }

  if ((size_t)index == list->size) {
    return &(list->last->next);
  }

  node_t **prev = &(list->first->next);

  for (int i = 0; i < index; i++) {
    prev = &(*prev)->next;
  }

  return prev;
}

void ioopm_linked_list_insert(ioopm_list_t *list, int index, elem_t value) {
  node_t **prev = find_previous_ptr(list, index);
  if (!(prev)) {
    // its null meaning out of index
    return;
  }

  node_t *new_node = create_node(value, *prev);
  *prev = new_node;
  // incase it was last index move last ptr
  if ((size_t)index == list->size) {
    list->last = new_node;
  }
  list->size++;
}

void ioopm_linked_list_append(ioopm_list_t *list, elem_t value) {
  ioopm_linked_list_insert(list, list->size, value);
}

void ioopm_linked_list_prepend(ioopm_list_t *list, elem_t value) {
  ioopm_linked_list_insert(list, 0, value);
}

bool ioopm_linked_list_get(ioopm_list_t *list, int index, elem_t *result) {
  node_t *prev = find_previous_node(list, index);
  if (!prev || !prev->next) { // incase index is wrong !prev->next cause
                              // prev-next is the one to remove, cant be null
    return false;
  }
  *result = prev->next->value;
  return true;
}

bool ioopm_linked_list_remove(ioopm_list_t *list, int index, elem_t *result) {
  node_t *prev = find_previous_node(list, index);
  if (!prev || !prev->next) { // incase index is wrong !prev->next cause
                              // prev-next is the one to remove, cant be null
    return false;
  }
  node_t *to_remove = prev->next;
  *result = to_remove->value;
  prev->next = to_remove->next;
  node_destroy(to_remove);

  // if i have to move the last ptr, thanks to the sentinal i never have to
  // change the first ptr
  if ((size_t)index == list->size - 1) {
    list->last = prev;
  }
  list->size--;
  return true;
}

bool ioopm_linked_list_contains(ioopm_list_t *list, elem_t element) {

  node_t *ptr = list->first->next;
  while (ptr) {
    if (list->equal_func(ptr->value, element) == 0) {
      return true;
    }
    ptr = ptr->next;
  }
  return false;
}

size_t ioopm_linked_list_size(ioopm_list_t *list) { return list->size; }

bool ioopm_linked_list_all(ioopm_list_t *list, ioopm_predicate *prop,
                           void *extra) {
  elem_t index;
  index.int_elem = 0;

  node_t *ptr = list->first->next;
  while (ptr) {
    // incase one is wrong return false
    if (!prop(index, ptr->value, extra)) {
      return false;
    }
    index.int_elem++;
    ptr = ptr->next;
  }
  // otherwise return true
  return true;
}

bool ioopm_linked_list_any(ioopm_list_t *list, ioopm_predicate *prop,
                           void *extra) {
  elem_t index;
  index.int_elem = 0;

  node_t *ptr = list->first->next;
  while (ptr) {
    // incase on i correct return true
    if (prop(index, ptr->value, extra)) {
      return true;
    }
    index.int_elem++;
    ptr = ptr->next;
  }

  // else return false
  return false;
}

void ioopm_linked_list_apply_to_all(ioopm_list_t *list,
                                    ioopm_apply_function *fun, void *extra) {
  elem_t index;
  index.int_elem = 0;
  node_t *ptr = list->first->next;
  while (ptr) {
    // apply function to all of the elements
    fun(index, &(ptr->value), extra);
    index.int_elem++;
    ptr = ptr->next;
  }
}

void print_linked_list(ioopm_list_t *list) {
  node_t *current = list->first->next; // Börja efter sentineln
  size_t index = 0;
  printf("Printing linked list:\n");
  while (current != NULL) {
    printf("Index %zu: Pointer: %p\n", index, *(void **)current->value.ptr);
    current = current->next;
    index++;
  }
  printf("End of list. Total nodes: %zu\n", list->size);
}

int ioopm_ptr_cmp_func(elem_t a, elem_t b) {
  return (uintptr_t)a.ptr - (uintptr_t)b.ptr;
}
