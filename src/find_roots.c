#include "find_roots.h"
#include "lib/linked_list.h"
#include <stdio.h>

// helper function check if allocated on our heap

ioopm_list_t *find_gc_roots(heap_t *heap) {
  // TODO: dont return anything useful
  ioopm_list_t *roots = ioopm_linked_list_create(ioopm_ptr_cmp_func);
  printf("test att skriva ut i funktion\n");

  return roots;
}
