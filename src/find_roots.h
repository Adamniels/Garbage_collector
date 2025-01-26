#include "heap.h"
#include "lib/linked_list.h"

typedef struct res {
  ioopm_list_t *roots;
  ioopm_list_t *expected_roots1;
  ioopm_list_t *expected_roots2;

} result;

result *find_gc_roots(heap_t *heap);
