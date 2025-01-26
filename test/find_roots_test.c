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
  result *res_find_roots = find_gc_roots(heap);
  CU_ASSERT_EQUAL(ioopm_linked_list_size(res_find_roots->roots), 1);

  elem_t res;
  ioopm_linked_list_get(res_find_roots->roots, 0, &res);
  CU_ASSERT_PTR_EQUAL(*(void **)res.ptr, obj1);
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
