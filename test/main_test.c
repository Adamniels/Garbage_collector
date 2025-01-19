#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdbool.h>

int compacting_tests();
int allocation_tests();
int heap_tests();

int main() {
  if (CUE_SUCCESS != CU_initialize_registry()) {
    return CU_get_error();
  }

  // Registrera testsuiter från alla testfile
  if (heap_tests() != CUE_SUCCESS) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  if (allocation_tests() != CUE_SUCCESS) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  if (compacting_tests() != CUE_SUCCESS) {
    CU_cleanup_registry();
    return CU_get_error();
  }
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();

  CU_cleanup_registry();
  return CU_get_error();
}
