#include "../src/compacting.h"
#include "../src/gc.h"
#include "../src/heap.h"

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void print_bit_vector2(uint64_t bit_vector) {
  printf("Bit vector: ");
  for (int i = 63; i >= 0; i--) { // Börja från MSB och gå ner till LSB
    printf("%lu", (bit_vector >> i) & 1);
    // Skifta bitarna och maskera för att extrahera varje bit
  }
  printf("\n");
}

void ex_compacting_test(void) { CU_ASSERT_TRUE(true); }

struct ptr_ptr_int {
  void *ptr1;
  void *ptr2;
  int int1;
};

void test_traverse_move_and_forward(void) {
  heap_t *heap = h_init(10400, false, 0.5);
  struct ptr_ptr_int *obj1 = h_alloc_struct(heap, "**i");
  struct ptr_ptr_int *obj2 = h_alloc_struct(heap, "**i");
  struct ptr_ptr_int *obj3 = h_alloc_struct(heap, "**i");
  obj1->ptr1 = obj3;
  obj1->ptr2 = NULL;
  obj2->ptr1 = NULL;
  obj2->ptr2 = NULL;
  obj3->ptr1 = NULL;
  obj3->ptr2 = NULL;

  obj3 = NULL;

  // check page status active in beginning
  page_t *page1 = heap->page_array[0];
  CU_ASSERT_TRUE(page1->is_active);
  CU_ASSERT_EQUAL(page1->remaining_size, 2048 - (3 * 32));
  ioopm_list_t *artificial_root_list =
      ioopm_linked_list_create(ioopm_ptr_cmp_func);
  ioopm_linked_list_append(artificial_root_list, (elem_t){.ptr = &obj1});
  ioopm_linked_list_append(artificial_root_list, (elem_t){.ptr = &obj2});

  // traverse and move, see so everything updates as it should
  traverse_and_move(heap, artificial_root_list);
}

int compacting_tests() {
  CU_pSuite pSuite = CU_add_suite("compacting_tests", NULL, NULL);
  if (NULL == pSuite) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  if ((NULL == CU_add_test(pSuite, "example test", ex_compacting_test)) ||
      (NULL == CU_add_test(pSuite,
                           "test traversing traverse_move and traverse_forward",
                           test_traverse_move_and_forward))) {

    CU_cleanup_registry();
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
