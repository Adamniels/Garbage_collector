#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdbool.h>

int compacting_tests();
int allocation_tests();
int heap_tests();
int find_root_tests();

int main() {
  if (CUE_SUCCESS != CU_initialize_registry()) {
    return CU_get_error();
  }

  // Registrera testsuiter
  if (heap_tests() != CUE_SUCCESS || allocation_tests() != CUE_SUCCESS ||
      compacting_tests() != CUE_SUCCESS || find_root_tests() != CUE_SUCCESS) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  // Kör alla andra tester (valfritt)
  printf("=== Running all other tests ===\n");
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();

  // Hämta enbart find_root_tests-sviten
  CU_pSuite find_root_suite = CU_get_suite("find root tests");
  if (!find_root_suite) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  // Kör endast find_root_tests med verbose-utmatning
  printf("=== Running find_root_tests with full output ===\n");
  CU_basic_set_mode(CU_BRM_NORMAL);
  CU_run_suite(find_root_suite);

  CU_cleanup_registry();
  return CU_get_error();
}
