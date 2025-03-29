#pragma once
#include "lib/linked_list.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 2048
#define MIN_OBJECT_SIZE 16
#define ALIGNMENT 0x1000

/**
 * @brief Represents a single memory page within the heap.
 *
 * Each page tracks metadata related to allocation and compacting:
 *  - `next_empty_space`: pointer to the next available space within the page.
 *  - `page_start`: the usable memory region after the page metadata.
 *  - `remaining_size`: how many bytes are left for allocation in this page.
 *  - `is_active`: marks whether the page is currently used for allocation.
 *  - `is_safe`: used during GC; true if references from stack are considered
 * safe.
 *  - `index`: the page's position in the heap's page array.
 */
typedef struct page {
  void *next_empty_space;
  void *page_start;
  size_t remaining_size;
  bool is_active;
  bool is_safe;
  size_t index;
} page_t;

/**
 * @brief Represents the entire heap memory space managed by the custom
 * allocator.
 *
 * The heap structure holds both the allocated memory region and metadata for:
 *  - Allocation management using a bitmap (alloc_map).
 *  - An array of `page_t` pointers (page_array) representing individual pages.
 *  - Configuration for garbage collection (GC threshold, safe/unsafe stack
 * scanning).
 *
 * Fields:
 *  - `heap_start`: pointer to the start of the usable heap memory.
 *  - `heap_size`: total number of bytes allocated for the heap.
 *  - `page_array`: array of pointers to pages (each holding page metadata).
 *  - `page_amount`: number of pages within the heap.
 *  - `safe`: indicates whether the stack is treated as safe for GC.
 *  - `GC_threshold`: fraction (0.0–1.0) of heap usage that triggers GC.
 *  - `alloc_map`: bitmap representing allocated slots in the heap.
 */
typedef struct heap {
  void *heap_start;
  size_t heap_size;
  page_t **page_array;
  size_t page_amount;
  bool safe;
  float GC_threshold;
  uint64_t *alloc_map;
} heap_t;

/**
 * @brief Initializes a new heap structure with aligned memory, allocation map,
 *        and page metadata for use in a custom memory manager or garbage
 * collector.
 *
 * This function allocates a contiguous memory block large enough to contain:
 *  - Heap metadata (`heap_t`)
 *  - All heap pages (`page_t`)
 *  - An allocation bitmap for tracking object allocations
 *  - An array of pointers to each page
 *
 * It ensures the allocated memory is properly aligned using `posix_memalign`
 * and sets up all internal heap structures accordingly.
 *
 * @param bytes         The number of bytes requested for the heap. Must be
 * large enough to hold at least one full page plus metadata.
 * @param unsafe_stack  If true, enables unsafe stack scanning (i.e., the stack
 * may contain live references not saved in registers). This flag sets
 * `heap->safe` to false.
 * @param gc_threshold  A floating-point threshold (0.0–1.0) that determines
 * when garbage collection should be triggered based on heap usage.
 *
 * @return A pointer to a fully initialized `heap_t` structure ready for
 * allocation and garbage collection operations.
 *
 * @note The program will abort with `assert` if:
 *       - `bytes` is too small to initialize the heap properly
 *       - Memory alignment or allocation fails
 *       - `heap_mem` ends up being NULL after `posix_memalign`
 */
heap_t *h_init(size_t bytes, bool unsafe_stack, float gc_threshold);

/**
 * @brief Frees the memory allocated for the given heap.
 *
 * This function releases all memory associated with the heap that was
 * previously allocated using `h_init`. It assumes the heap is no longer
 * in use and does not perform any additional cleanup of objects within the
 * heap.
 *
 * @param heap A pointer to the heap to be deallocated.
 *
 * @note If `heap` is NULL, the program will abort via `assert`.
 */
void h_delete(heap_t *heap);

/**
 * @brief Frees the heap after overwriting its memory with a debug value.
 *
 * This function fills the entire heap memory region with a given byte
 * pattern (`dbg_value`) before freeing it. This is useful for debugging,
 * as it can help identify use-after-free bugs by making invalid memory
 * accesses more noticeable.
 *
 * @param heap       A pointer to the heap to be deallocated.
 * @param dbg_value  The byte value used to fill the heap memory before
 * deallocation.
 *
 * @note If `heap` is NULL, the program will abort via `assert`.
 */
void h_delete_dbg(heap_t *heap, unsigned char dbg_value);
