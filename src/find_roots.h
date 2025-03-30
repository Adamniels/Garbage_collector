#include "heap.h"
#include "lib/linked_list.h"

/**
 * @brief Container for GC root scanning results.
 *
 * Used to return multiple lists from `find_gc_roots()`:
 * - `roots`: A list of pointers to stack locations holding references into the
 * heap.
 * - `expected_roots1` and `expected_roots2`: Lists of the actual pointer values
 *    found on the stack that point into the heap (used for validation in
 * compacting).
 */
typedef struct res {
  // ptr to ptr
  ioopm_list_t *roots;
  // A bug where some values where added/changed just before the return and just
  // after, I solved this 2 validation lists with only a ptr to the object, so
  // one less level the *roots
  ioopm_list_t *expected_roots1;
  ioopm_list_t *expected_roots2;

} result;

/**
 * @brief Scans the stack for potential root pointers referencing the heap.
 *
 * This function scans the stack from the current frame
 * (`__builtin_frame_address(0)`) down to the environment pointer (`environ`)
 * and checks every word-sized slot to see if it contains a valid pointer into
 * the heap.
 *
 * All found root locations are added to the returned `result` structure:
 * - `roots` contains the stack addresses (void **) that hold pointers to the
 * heap.
 * - `expected_roots1` and `expected_roots2` contain the dereferenced values of
 * those pointers.
 *
 * @param heap A pointer to the heap being scanned.
 * @return A dynamically allocated `result *` structure containing the root
 * lists.
 *
 * @note Caller is responsible for freeing the result and its contained lists.
 */
result *find_gc_roots(heap_t *heap);

/**
 * @brief Checks if a given pointer refers to a valid allocated object within
 * the heap.
 *
 * This function verifies whether `ptr` lies within the heap's allocated region
 * and corresponds to a set bit in the heap's allocation bitmap. It performs
 * several checks:
 *
 * 1. Ensures the pointer lies within the bounds of the usable heap region
 * (excluding page metadata).
 * 2. Verifies that the pointer is 8-byte aligned (required since all
 * allocations include headers).
 * 3. Computes the offset of the pointer within the heap, adjusts for page
 * metadata, and calculates the appropriate bit index in the allocation map.
 * 4. Checks whether the bit at that index is set, indicating the pointer
 * belongs to an allocated block.
 *
 * @param heap Pointer to the `heap_t` structure describing the heap.
 * @param ptr  The pointer to check.
 *
 * @return `true` if the pointer points to a currently allocated object in the
 * heap, `false` otherwise.
 *
 * @note If `heap` is NULL, the function will abort using `assert`. Not a 100%
 * but can exclude most ptr that by chance can "point" into the heap
 */
bool is_allocated_on_heap(heap_t *heap, void *ptr);
