#include "../src/compacting.h"
#include "../src/gc.h"
#include "../src/heap.h"

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void ex_find_root_test(void) { CU_ASSERT_TRUE(true); }

void test_find_single_root(void) {
  heap_t *heap = h_init(10400, false, 0.5);
  void *obj1 = h_alloc_struct(heap, "*");
  ioopm_list_t *root_list = find_gc_roots(heap);
}

int find_root_tests() {
  CU_pSuite pSuite = CU_add_suite("find root tests", NULL, NULL);
  if (NULL == pSuite) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  if ((NULL == CU_add_test(pSuite, "example test", ex_find_root_test)) ||
      (NULL ==
       CU_add_test(pSuite, "find single root test", test_find_single_root)) ||
      false) {

    CU_cleanup_registry();
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
