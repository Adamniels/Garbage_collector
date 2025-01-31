#include "../src/compacting.h"
#include "../src/debug.h"
#include "../src/gc.h"
#include "../src/heap.h"

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// helper for understanding how the bit vector looks like
void print_bit_vector2(uint64_t bit_vector) {
  DEBUG_PRINT("Bit vector: ");
  for (int i = 63; i >= 0; i--) { // Börja från MSB och gå ner till LSB
    DEBUG_PRINT("%lu", (bit_vector >> i) & 1);
    // Skifta bitarna och maskera för att extrahera varje bit
  }
  DEBUG_PRINT("\n");
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

  obj3 = NULL;

  // check allocation map is what it is supposed to be, 6 bit should be set
  // size of obj = 32, one bit represent 16 bytes so 2 bits for each object
  uint64_t first_in_map_array = heap->alloc_map[0];
  uint64_t expected_bit_vector = 0;
  expected_bit_vector |= (127ULL << 58); // 111111 = 2^7-1
  print_bit_vector2(expected_bit_vector);
  print_bit_vector2(first_in_map_array);
  CU_ASSERT_EQUAL(first_in_map_array, expected_bit_vector);

  // check page status active in beginning
  page_t *page1 = heap->page_array[0];
  page_t *page2 = heap->page_array[1];
  CU_ASSERT_TRUE(page1->is_active);
  CU_ASSERT_FALSE(page2->is_active);
  CU_ASSERT_EQUAL(page1->remaining_size, 2048 - (3 * 32));
  ioopm_list_t *artificial_root_list =
      ioopm_linked_list_create(ioopm_ptr_cmp_func);
  ioopm_linked_list_append(artificial_root_list, (elem_t){.ptr = &obj1});
  ioopm_linked_list_append(artificial_root_list, (elem_t){.ptr = &obj2});

  ioopm_list_t *expected1 = ioopm_linked_list_create(ioopm_ptr_cmp_func);
  ioopm_linked_list_append(expected1, (elem_t){.ptr = obj1});
  ioopm_linked_list_append(expected1, (elem_t){.ptr = obj2});

  ioopm_list_t *expected2 = ioopm_linked_list_create(ioopm_ptr_cmp_func);
  ioopm_linked_list_append(expected2, (elem_t){.ptr = obj1});
  ioopm_linked_list_append(expected2, (elem_t){.ptr = obj2});

  // traverse and move, see so everything updates as it should
  traverse_and_move(heap, artificial_root_list, expected1);
  // test allocation map
  first_in_map_array = heap->alloc_map[0];
  uint64_t third_in_map_array = heap->alloc_map[2];
  expected_bit_vector = 0;
  CU_ASSERT_EQUAL(first_in_map_array, expected_bit_vector);
  expected_bit_vector |= (127ULL << 58);
  print_bit_vector2(third_in_map_array);
  CU_ASSERT_EQUAL(third_in_map_array, expected_bit_vector);

  CU_ASSERT_TRUE(page2->is_active);
  CU_ASSERT_EQUAL(page2->remaining_size, 2048 - (3 * 32));
  CU_ASSERT_FALSE(page1->is_active);
  CU_ASSERT_EQUAL(page1->remaining_size, 2048);
  CU_ASSERT_EQUAL(page1->page_start, page1->next_empty_space);
  // ptr shouldn't be updated yet
  CU_ASSERT_EQUAL((uint64_t *)obj1, (uint64_t *)page1->page_start + 1);
  // start+header

  CU_ASSERT_EQUAL((uint64_t *)obj2, (uint64_t *)page1->page_start + 5);
  // start+obj1+header

  CU_ASSERT_EQUAL((uint64_t *)obj1->ptr1, (uint64_t *)page1->page_start + 9);
  // start+obj1+obj2+header

  traverse_and_forward(heap, artificial_root_list, expected2);
  //  obj ptr should be updated
  CU_ASSERT_NOT_EQUAL((uint64_t *)obj1, (uint64_t *)page1->page_start + 1);
  CU_ASSERT_NOT_EQUAL((uint64_t *)obj2, (uint64_t *)page1->page_start + 5);
  CU_ASSERT_NOT_EQUAL((uint64_t *)obj1->ptr1,
                      (uint64_t *)page1->page_start + 9);

  // should be pointing to page2 index 1
  CU_ASSERT_EQUAL((uint64_t *)obj1, (uint64_t *)page2->page_start + 1);
  CU_ASSERT_EQUAL((uint64_t *)obj2, (uint64_t *)page2->page_start + 5);
  CU_ASSERT_EQUAL((uint64_t *)obj1->ptr1, (uint64_t *)page2->page_start + 9);

  ioopm_linked_list_destroy(artificial_root_list);
  ioopm_linked_list_destroy(expected1);
  ioopm_linked_list_destroy(expected2);
  h_delete(heap);
}

void test_GC_same_case_as_test_traverse_and_forward(void) {
  heap_t *heap = h_init(10400, false, 0.5);

  struct ptr_ptr_int *obj1 = h_alloc_struct(heap, "**i");
  struct ptr_ptr_int *obj2 = h_alloc_struct(heap, "**i");
  struct ptr_ptr_int *obj3 = h_alloc_struct(heap, "**i");
  obj1->ptr1 = obj3;

  obj3 = NULL;

  // check allocation map is what it is supposed to be, 6 bit should be set
  // size of obj = 32, one bit represent 16 bytes so 2 bits for each object
  uint64_t first_in_map_array = heap->alloc_map[0];
  uint64_t expected_bit_vector = 0;
  expected_bit_vector |= (127ULL << 58); // 111111 = 2^7-1
  print_bit_vector2(expected_bit_vector);
  print_bit_vector2(first_in_map_array);
  CU_ASSERT_EQUAL(first_in_map_array, expected_bit_vector);

  // check page status active in beginning
  page_t *page1 = heap->page_array[0];
  page_t *page2 = heap->page_array[1];
  CU_ASSERT_TRUE(page1->is_active);
  CU_ASSERT_FALSE(page2->is_active);
  CU_ASSERT_EQUAL(page1->remaining_size, 2048 - (3 * 32));

  size_t reclaimed = h_gc_dbg(heap, false);

  // test allocation map
  first_in_map_array = heap->alloc_map[0];
  uint64_t third_in_map_array = heap->alloc_map[2];
  expected_bit_vector = 0;
  CU_ASSERT_EQUAL(first_in_map_array, expected_bit_vector);
  expected_bit_vector |= (127ULL << 58);
  print_bit_vector2(third_in_map_array);
  CU_ASSERT_EQUAL(third_in_map_array, expected_bit_vector);

  CU_ASSERT_TRUE(page2->is_active);
  CU_ASSERT_EQUAL(page2->remaining_size, 2048 - (3 * 32));
  CU_ASSERT_FALSE(page1->is_active);
  CU_ASSERT_EQUAL(page1->remaining_size, 2048);
  CU_ASSERT_EQUAL(page1->page_start, page1->next_empty_space);

  //  obj ptr should be updated
  CU_ASSERT_NOT_EQUAL((uint64_t *)obj1, (uint64_t *)page1->page_start + 1);
  CU_ASSERT_NOT_EQUAL((uint64_t *)obj2, (uint64_t *)page1->page_start + 5);
  CU_ASSERT_NOT_EQUAL((uint64_t *)obj1->ptr1,
                      (uint64_t *)page1->page_start + 9);

  // should be pointing to page2 index 1
  CU_ASSERT_EQUAL((uint64_t *)obj1, (uint64_t *)page2->page_start + 1);
  CU_ASSERT_EQUAL((uint64_t *)obj2, (uint64_t *)page2->page_start + 5);
  CU_ASSERT_EQUAL((uint64_t *)obj1->ptr1, (uint64_t *)page2->page_start + 9);

  // set obj 1 and 3 to null, they should then be collected
  obj1 = NULL;
  reclaimed = h_gc_dbg(heap, false);
  CU_ASSERT_EQUAL(reclaimed, 64);

  h_delete(heap);
}

// TODO: skriva tester för en vanlig GC efter att jag gjort find_roots

int compacting_tests() {
  CU_pSuite pSuite = CU_add_suite("compacting_tests", NULL, NULL);
  if (NULL == pSuite) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  if ((NULL == CU_add_test(pSuite, "example test", ex_compacting_test)) ||
      (NULL == CU_add_test(pSuite,
                           "test traversing traverse_move and traverse_forward",
                           test_traverse_move_and_forward)) ||
      (NULL ==
       CU_add_test(pSuite,
                   "same test as traverse move and forward but with gc ",
                   test_GC_same_case_as_test_traverse_and_forward)) ||
      false) {

    CU_cleanup_registry();
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
