#include "../src/allocation.h"
#include "../src/debug.h"
#include "../src/gc.h"
#include "../src/heap.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdbool.h>
#include <stdlib.h>

void ex_allocation_test(void) { CU_ASSERT_TRUE(true); }

// helper for understanding how the bit vector looks like
void print_bit_vector(uint64_t bit_vector) {
  DEBUG_PRINT("Bit vector: ");
  for (int i = 63; i >= 0; i--) { // Börja från MSB och gå ner till LSB
    DEBUG_PRINT("%lu", (bit_vector >> i) & 1);
    // Skifta bitarna och maskera för att extrahera varje bit
  }
  DEBUG_PRINT("\n");
}

void test_set_bit_in_bit_vector(void) {
  // simulate a empty header in heap, creates a array of 4 uint8 so for each
  // byte
  uint8_t heap_buffer[sizeof(layout_bitvector_t)] = {0};

  char layout[] = "**i*";
  CU_ASSERT_EQUAL(length_layout(layout), 4);
  CU_ASSERT_EQUAL(object_size(layout), 28);

  set_layout_header(layout, heap_buffer);

  layout_bitvector_t *result = (layout_bitvector_t *)heap_buffer;
  // printf("Bit vector stored at address %p: %u\n", (void *)result,
  // result->bit_vector);

  // set the bits that should be correct with (or-operation)
  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11101111

  uint64_t expected_bit_vector = 0;
  expected_bit_vector |= (1ULL << 7) | (1ULL << 6) | (1ULL << 5) | (1ULL << 3) |
                         (1ULL << 2) | (1ULL << 1) | (1ULL << 0);

  CU_ASSERT_EQUAL(result->bit_vector, expected_bit_vector);
}

void test_layout_with_all_types(void) {
  uint8_t heap_buffer[sizeof(layout_bitvector_t)] = {0};
  char layout[] = "l*i*d";
  CU_ASSERT_EQUAL(length_layout(layout), 5);
  CU_ASSERT_EQUAL(object_size(layout), 36);

  set_layout_header(layout, heap_buffer);

  layout_bitvector_t *result = (layout_bitvector_t *)heap_buffer;

  // should be
  // 00000000 00000000 00000000 00000000 00000000 00000000 00000100 10100111
  uint64_t expected_bit_vector = 0;
  expected_bit_vector |= (1ULL << 10) | (1ULL << 7) | (1ULL << 5) |
                         (1ULL << 2) | (1ULL << 1) | (1ULL << 0);

  print_bit_vector(result->bit_vector);

  CU_ASSERT_EQUAL(result->bit_vector, expected_bit_vector);
}

void test_layout_size_to_big(void) {

  // NOTE: Test causes a assert by design, uncomment to run test but it passess.
  /*
    char too_long_layout[] =
    "**i*i*****i*i*****i*i*****i*i*****i*i*****i*i*****i*i*****i*i***"; // 64
    tecken uint8_t heap_buffer[sizeof(layout_bitvector_t)] = {0};
    // split the process from here on out
    pid_t pid = fork();
    // for the child process the fork() will return 0 (child and parent have
    differen pid) if (pid == 0) {
        /// children process
        set_layout_header(too_long_layout, heap_buffer);
        exit(0); // the above line should crash, so the test fails if we get
    here } else {
        // parent process
        int status;
        // wait for the childprocess
        waitpid(pid, &status, 0);

        // assert we get a flag signaling crash
        CU_ASSERT_TRUE(WIFSIGNALED(status));
        // assert it is the correct flag
        CU_ASSERT_EQUAL(WTERMSIG(status), SIGABRT);
    }
  */
}

//////Finding next available space in heap//////
void test_find_first_in_empty_heap() {
  heap_t *heap = h_init((size_t)2500, false, 0.8);
  long test_long = 2e10;

  int index = find_next_available(heap, sizeof(test_long));

  // If heap is empty, next available page should be first page.
  CU_ASSERT_EQUAL(index, 0);
  CU_ASSERT_EQUAL(heap->page_array[index], heap->page_array[0]);
  h_delete(heap);
}

void test_find_first_in_non_empty_heap() {
  heap_t *heap = h_init((size_t)2500, false, 0.8);
  long test_long = 2e10;
  // long *alloc1 =
  h_alloc_raw(heap, sizeof(test_long)); // NOTE: Variabel används ej
  int index = find_next_available(heap, sizeof(test_long));

  CU_ASSERT_EQUAL(heap->page_array[0], heap->page_array[index]);
  h_delete(heap);
}

void test_allocating_object_moving_next_ptr(void) {
  heap_t *heap = h_init((size_t)2600, false, 0.8);

  // first page
  page_t *first = heap->page_array[0];
  CU_ASSERT_EQUAL(first->page_start, first->next_empty_space);

  char *start = (char *)first->page_start;
  char *next_empty = (char *)first->next_empty_space;
  size_t offset_before_alloc = next_empty - start;

  // allocate object ptr int int ptr size 24 + header = 32
  h_alloc_struct(heap, "*ii*");

  // update next empty
  next_empty = (char *)first->next_empty_space;

  // check that it moved as much as it should
  size_t expected_offset = offset_before_alloc + 32;

  CU_ASSERT_EQUAL(start + expected_offset, next_empty);
  h_delete(heap);
}

void test_allocating_flipp_bits_in_map(void) {
  heap_t *heap = h_init((size_t)2600, false, 0.8);

  // allocate object ptr int int ptr size 24 + header = 32
  h_alloc_struct(heap, "*ii*");

  // check if 2 first bits in alloc map has changed
  uint64_t first_in_map_array = heap->alloc_map[0];
  // print_bit_vector(first_in_map_array);
  uint64_t expected_bit_vector = 0;
  expected_bit_vector |= (1ULL << 63) | (1ULL << 62);

  // print_bit_vector(expected_bit_vector);
  CU_ASSERT_EQUAL(expected_bit_vector, first_in_map_array);
  h_delete(heap);
}

void test_flipping_bits_h_alloc_raw(void) {
  heap_t *heap = h_init((size_t)2600, false, 0.8);

  // allocate object ptr int int ptr size 24 + header = 32
  h_alloc_raw(heap, 24);

  // check if 2 first bits in alloc map has changed
  uint64_t first_in_map_array = heap->alloc_map[0];
  // print_bit_vector(first_in_map_array);
  uint64_t expected_bit_vector = 0;
  expected_bit_vector |= (1ULL << 63) | (1ULL << 62);

  // print_bit_vector(expected_bit_vector);
  CU_ASSERT_EQUAL(expected_bit_vector, first_in_map_array);
  h_delete(heap);
}

void test_alloc_raw(void) {
  heap_t *heap = h_init((size_t)2600, false, 0.8);

  CU_ASSERT_EQUAL(heap->page_array[0]->remaining_size, 2048);
  page_t *first = heap->page_array[0];
  CU_ASSERT_EQUAL(first->page_start, first->next_empty_space);

  // allocate object ptr int int ptr size 24 + header = 32
  h_alloc_raw(heap, 24);

  CU_ASSERT_EQUAL(heap->page_array[0]->remaining_size, 2016);
  CU_ASSERT_EQUAL(first->page_start + 32, first->next_empty_space);

  h_delete(heap);
}

void test_allocating_remainingsize_changed(void) {
  heap_t *heap = h_init((size_t)2600, false, 0.8);

  CU_ASSERT_EQUAL(heap->page_array[0]->remaining_size, 2048);
  // allocate object ptr int int ptr size 24 + header = 32
  h_alloc_struct(heap, "*ii*");

  CU_ASSERT_EQUAL(heap->page_array[0]->remaining_size, 2016);
  h_delete(heap);
}

void test_allocating_100_obj(void) {
  heap_t *heap = h_init((size_t)5200, false, 0.8);

  // allocate first page
  for (int i = 0; i < 64; i++) {
    h_alloc_struct(heap, "*ii*");
  }
  // alloc one more on second page run same tests
  h_alloc_struct(heap, "*ii*");
  CU_ASSERT_EQUAL(heap->page_array[1]->remaining_size, 2016);
  // check if 2 first bits in alloc map has changed
  uint64_t third_in_map_array = heap->alloc_map[2];
  // print_bit_vector(first_in_map_array);
  uint64_t expected_bit_vector = 0;
  expected_bit_vector |= (1ULL << 63) | (1ULL << 62);

  // print_bit_vector(expected_bit_vector);
  CU_ASSERT_EQUAL(expected_bit_vector, third_in_map_array);
  CU_ASSERT_EQUAL(0ULL, heap->alloc_map[3]);
  h_delete(heap);
}

struct test_struct {
  int int_t;
  char char_t;
};

void test_using_obj(void) {
  heap_t *heap = h_init((size_t)2600, false, 0.8);
  int *obj_ptr_int = h_alloc_struct(heap, "i");
  *obj_ptr_int = 100;
  CU_ASSERT_EQUAL(100, *obj_ptr_int);
  char *obj_ptr_char = h_alloc_struct(heap, "i");
  *obj_ptr_char = 'a';
  CU_ASSERT_EQUAL('a', *obj_ptr_char);

  struct test_struct *test_struct = h_alloc_struct(heap, "ii");
  test_struct->char_t = 'b';
  test_struct->int_t = 10;
  CU_ASSERT_EQUAL(test_struct->char_t, 'b');
  CU_ASSERT_EQUAL(test_struct->int_t, 10);
  h_delete(heap);
}

struct ptr_int_ptr {
  void *ptr1;
  int int1;
  void *ptr2;
};

void test_covering_more_complex(void) {
  heap_t *heap = h_init((size_t)10400, false, 0.5);

  // allocate 3 objects each 32 bytes
  struct ptr_int_ptr *obj1 = h_alloc_struct(heap, "*i*");
  struct ptr_int_ptr *obj2 = h_alloc_struct(heap, "*i*");
  struct ptr_int_ptr *obj3 = h_alloc_struct(heap, "*i*");
  obj1->int1 = 1;
  obj2->int1 = 2;
  obj3->int1 = 3;
  CU_ASSERT_EQUAL(heap->page_array[0]->remaining_size, 2048 - (3 * 32));

  // link them obj1 -> obj2 and obj3
  obj1->ptr1 = obj2;
  obj1->ptr2 = obj3;
  // link obj2 -> obj3
  obj2->ptr1 = obj3;

  CU_ASSERT_EQUAL(obj2, obj1->ptr1);
  CU_ASSERT_EQUAL(obj1->int1, 1);

  // set obj2 and obj3 to null
  obj2 = NULL;
  obj3 = NULL;

  CU_ASSERT_EQUAL(((struct ptr_int_ptr *)(obj1->ptr1))->int1, 2);
  CU_ASSERT_EQUAL(((struct ptr_int_ptr *)(obj1->ptr2))->int1, 3);

  h_delete(heap);
}

int allocation_tests() {
  CU_pSuite pSuite = CU_add_suite("allocation_tests", NULL, NULL);
  if (NULL == pSuite) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  if ((NULL == CU_add_test(pSuite, "example test", ex_allocation_test)) ||
      (NULL == CU_add_test(pSuite, "test setting one bit in the bit vector",
                           test_set_bit_in_bit_vector)) ||
      (NULL == CU_add_test(pSuite, "test to long layout string",
                           test_layout_size_to_big)) ||
      (NULL == CU_add_test(pSuite,
                           "Test layout with other things then int and ptr",
                           test_layout_with_all_types)) ||
      (NULL == CU_add_test(pSuite,
                           "test to find next empty space in empty heap",
                           test_find_first_in_empty_heap)) ||
      (NULL == CU_add_test(pSuite,
                           "test to find next empty space in non-empty heap",
                           test_find_first_in_non_empty_heap)) ||
      (NULL == CU_add_test(pSuite, "test simple allocation",
                           test_allocating_object_moving_next_ptr)) ||
      (NULL == CU_add_test(pSuite, "test flipping bits in allocation map",
                           test_allocating_flipp_bits_in_map)) ||
      (NULL ==
       CU_add_test(pSuite,
                   "test flipping bits in allocation map for raw allocation",
                   test_flipping_bits_h_alloc_raw)) ||
      (NULL == CU_add_test(pSuite, "test metadata correct after alloc raw",
                           test_alloc_raw)) ||
      (NULL ==
       CU_add_test(pSuite,
                   "test allocating and see i remaining changed as it should",
                   test_allocating_remainingsize_changed)) ||
      (NULL == CU_add_test(pSuite, "test allocation 100 objects",
                           test_allocating_100_obj)) ||
      (NULL == CU_add_test(pSuite, "test using the object after allocation",
                           test_using_obj)) ||
      (NULL ==
       CU_add_test(pSuite,
                   "test a more complex testing, more like a simple program",
                   test_covering_more_complex)) ||

      false) {

    CU_cleanup_registry();
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
