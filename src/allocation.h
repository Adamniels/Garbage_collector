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

/**
 * @brief Represents a compact layout descriptor using a 64-bit bit vector.
 *
 * The bit vector encodes the positions of pointer fields in an object.
 * Used to guide the garbage collector during traversal of structured data.
 */
typedef struct {
  uint64_t bit_vector;
} layout_bitvector_t;

/**
 * @brief Calculates the length of a layout string.
 *
 * A layout string is a null-terminated string where each character represents
 * a field in a struct: `'*'` for pointer, `'l'` for long(8 bytes), `'d'` for
 * double(8 bytes), `'i'` for int(4 bytes). note the only important thing is
 * that we set the pointers correct, it doens't matter if we use 'ii' or 'd'/'l'
 *
 * @param layout The layout string.
 * @return The number of characters (fields) in the layout.
 */
int length_layout(char *layout);

/**
 * @brief Sets a specific bit in a layout bit vector.
 *
 * Marks a field as a pointer by setting the bit at `field_index` to 1.
 * Does nothing if the index is out of bounds.
 *
 * @param lbv          Pointer to a layout_bitvector_t structure.
 * @param field_index  The index of the field to mark as a pointer (0-based).
 */
void set_bit_vector(layout_bitvector_t *lbv, int field_index);

/**
 * @brief Calculates the total size (in bytes) of an object based on a layout
 * string.
 *
 * - `'*'`, `'l'`, `'d'` are treated as 8 bytes each.
 * - Any other character is treated as 4 bytes.
 *
 * @param layout The layout string describing the object.
 * @return The total number of bytes the object occupies (excluding
 * header/padding).
 */
size_t object_size(char *layout);

/**
 * @brief Encodes a layout string into a bitvector and writes it into a header.
 *
 * The header is stored at the specified memory address. Bit positions indicate
 * pointer fields. Adds layout markers and a metadata flag in the lowest bits.
 *
 * Assumptions:
 * - `'l'` and `'d'` occupy 8 bytes.
 * - All other non-pointer types occupy 4 bytes.
 *
 * @param layout      The layout string to encode.
 * @param header_pos  Pointer to where the header should be written.
 */
void set_layout_header(char *layout, void *header_pos);

/**
 * @brief Finds the index of the first heap page with enough space for an
 * allocation.
 *
 * First looks in active pages, then in passive pages (which are activated if
 * chosen).
 *
 * @param heap Pointer to the heap structure.
 * @param size Size in bytes of the object to be allocated.
 * @return Index of the suitable page, or -1 if no page has enough space.
 */
int find_next_available(heap_t *heap, size_t size);

/**
 * @brief Sets bits in the allocation map to indicate allocated memory regions.
 *
 * The allocation map uses 2x `uint64_t` per page (128 bits), with one bit per
 * 16-byte slot. This function flips `bytes / 16` bits starting from
 * `start_index`.
 *
 * @param alloc_map    Pointer to the allocation map.
 * @param start_index  Starting index in 16-byte chunks.
 * @param bytes        Number of bytes to mark as allocated (must be divisible
 * by 16).
 */
void set_bits_in_alloc_map(uint64_t *alloc_map, int start_index, int bytes);
