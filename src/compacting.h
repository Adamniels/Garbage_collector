#pragma once

#include "allocation.h"
#include "find_roots.h"
#include "gc.h"
#include "heap.h"
#include "lib/linked_list.h"

/**
 * @brief Interprets the header of an object and returns all pointer fields.
 *
 * Based on a bitvector in the object's header, this function returns an array
 * of pointers to all pointer fields contained within the object.
 *
 * @param p              A pointer to the start of the object (just after the
 * header).
 * @param num_pointers   Output: number of pointer fields found.
 * @param obj_size       Output: total size of the object in bytes (excluding
 * header).
 *
 * @return An array of `void**` pointing to each pointer field in the object.
 *         Returns NULL if the object has no pointer fields or if layout is
 * size-only.
 *
 * @note The returned array must be freed by the caller.
 */
void ***interpret_header(void *p, size_t *num_pointers, size_t *obj_size);

/**
 * @brief Performs garbage collection using the current heap's safety setting.
 *
 * Internally calls `h_gc_dbg` with the inverse of the heap's safety flag.
 *
 * @param h Pointer to the heap.
 * @return Number of bytes reclaimed by the garbage collector.
 */
size_t h_gc(heap_t *h);

/**
 * @brief Performs garbage collection with optional unsafe stack scanning.
 *
 * This version of the garbage collector runs in two passes:
 * 1. Scans the stack and marks reachable objects, moving them to passive pages.
 * 2. Updates all references to point to the new addresses.
 *
 * @param h              Pointer to the heap.
 * @param unsafe_stack   If true, skips safety checks when scanning the stack.
 * @return Number of bytes reclaimed after GC.
 */
size_t h_gc_dbg(heap_t *h, bool unsafe_stack);

/**
 * @brief Extracts the actual address from a forwarding header.
 *
 * Clears the lowest 3 bits (which store type information) to recover the raw
 * pointer.
 *
 * @param header The encoded header value.
 * @return The extracted address.
 */
uint64_t extract_adress(uint64_t header);

/**
 * @brief Counts the total number of allocated bytes across all active pages.
 *
 * @param h Pointer to the heap.
 * @return The number of allocated bytes.
 */
size_t count_allocated_bytes_on_heap(heap_t *h);

/**
 * @brief Traverses all reachable objects from the root list and moves them to
 * passive pages.
 *
 * Performs a breadth-first traversal of the object graph, copying each object
 * to a new page and updating the old header with a forwarding address.
 *
 * @param h              Pointer to the heap.
 * @param root_list      List of root pointers to scan.
 * @param expected_list  List of expected pointer values for
 * validation/debugging.
 */
void traverse_and_move(heap_t *h, ioopm_list_t *root_list,
                       ioopm_list_t *expected_list);

/**
 * @brief Second pass of GC: updates all root and internal pointers to point to
 * moved objects.
 *
 * Follows the forwarding addresses in object headers and updates all
 * pointers in the object graph to point to the new locations.
 *
 * @param h              Pointer to the heap.
 * @param root_list      List of stack pointer locations (pointers to pointers).
 * @param expected_list  Expected pointer values (used for validation).
 */
void traverse_and_forward(heap_t *h, ioopm_list_t *root_list,
                          ioopm_list_t *expected_list);
