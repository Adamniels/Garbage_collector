#pragma once
#include "heap.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define HEADER_SIZE 8 // size of void ptr

/**
 * Information about the "layout header"
 *
 * the header is a size of 8 bytes (64 bits), the first 2 bits(from right) is
 * for telling us what metadata we have in our header if the header is a layout
 * specification the third bit(from right) tells us if we just have a size in
 * bytes(0) or a bit vector(1)
 *
 * to read the layout vector and start reading from the left, the first 1 you
 * encounter indicate the start of the layout then the layout continue to the 3
 * last bits ex "**i*" 00000000 00000000 00000000 00000000 00000000 00000000
 * 00000000 11101111 00000000 00000000 00000000 00000000 00000000 00000000
 * 00000000 (startbit 1)(layout 1101)(info about header 111)
 *
 */

typedef struct {
  uint64_t bit_vector;
} layout_bitvector_t;

/**
 * Calculate the length of the layout string
 */
int length_layout(char *layout);

/**
 * Sets a bit in the bit vector at the specified index.
 *
 * The bit is set only if the field index is within the valid
 * range (0 to HEADER_SIZE*4 - 1).
 *
 * Takes a pointer to the bit vector(TODO: for now a struct with a bitvector
 * inside) and the field index which we should set to true
 */
void set_bit_vector(layout_bitvector_t *lbv, int field_index);

/**
 * Calculates the object size based on the layout string
 */
size_t object_size(char *layout);

/**
 * Sets the header to a bitvector
 *
 * Takes a layout string and a ptr to the start position of the header
 *
 * we assume Long and double is 8 bytes and the other are 4 bytes (except ptrs)
 */
void set_layout_header(char *layout, void *header_pos);
int find_next_available(heap_t *heap, size_t size);

/**
 * Set the bits in the allocation map takes in the start index, the we should
 * start flipping from and how many bytes to flipp (obs takes in bytes and
 * divide by 16 to get the number off bits, so need to be modulo 16 = 0)
 */
void set_bits_in_alloc_map(uint64_t *alloc_map, int start_index, int bytes);
