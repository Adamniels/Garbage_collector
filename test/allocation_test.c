// #include "../src/compacting.h"
// #include "../src/gc.h"
// #include "../src/heap.h"

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdbool.h>
#include <stdlib.h>

void ex_allocation_test(void) { CU_ASSERT_TRUE(true); }

int allocation_tests() {
  CU_pSuite pSuite = CU_add_suite("allocation_tests", NULL, NULL);
  if (NULL == pSuite) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  if ((NULL == CU_add_test(pSuite, "example test", ex_allocation_test))) {

    CU_cleanup_registry();
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
