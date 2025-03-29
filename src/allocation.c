#include "allocation.h"
#include "compacting.h"
#include "debug.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void set_bit_vector(layout_bitvector_t *lbv, int field_index) {
  if (field_index >= 0 && field_index < HEADER_SIZE * 8) {
    // the right side puts the 1 int the correct place and |= (or) so we set the
    // bit on the index to a one
    lbv->bit_vector |= (1ULL << field_index);
  }
}

// helper for set_layout_header
int length_layout(char *layout) {
  int count = 0;
  while (layout[count] != '\0') {
    count++;
  }
  return count;
}

void set_layout_header(char *layout, void *header_pos) {
  // get the adress where the header should be and place the layout there
  layout_bitvector_t *bv = (layout_bitvector_t *)header_pos;

  // make sure bit vector is all zeros at beginning
  bv->bit_vector = 0;

  int len_layout = length_layout(layout);
  // 4 bits for information so 60 bits can describe the vector
  assert(len_layout <= 60);

  // it it is a type that is 8 bytes/2 words we should have to zeros to
  // represent that 8 byte block
  int index = 0;  // (start from pos 4)
  int offset = 3; // 3 bits in the beginning for information
  while (index < len_layout && index < HEADER_SIZE * 8 - offset) {
    char type = layout[len_layout - index - 1];
    if (type == '*') {
      set_bit_vector(bv, index + offset);
    } else if (type == 'l' || type == 'd') {
      offset++;
    }
    index++;
  }

  // set the start bit to 1, here we can start reading
  set_bit_vector(bv, index + offset);

  // set the last 3 bits, 2 bit based on what the header has inside (bitvector
  // for layout), 1 bit that it is a layout and not just a size set them to 111
  // first 1
  bv->bit_vector |= (1ULL << 2) | (1ULL << 1) | (1ULL << 0);
}

int find_next_available(heap_t *heap, size_t size) {
  // Iterera genom page-array
  page_t **arr = heap->page_array;
  for (size_t i = 0; i < heap->page_amount; i++) {
    page_t *page = arr[i];
    // Kolla om aktiv
    if (page->is_active) {
      // Jämför storlek på objekt mot remaining size
      if (size <= page->remaining_size) {
        return (int)i;
      }
    } else {
    }
  }
  for (size_t i = 0; i < heap->page_amount; i++) {
    page_t *page = arr[i];
    // Kolla om passiv
    if (!page->is_active) {
      // Jämför storlek på objekt mot remaining size
      if (size <= page->remaining_size) {
        // set page status to active and return
        page->is_active = 1;
        return (int)i;
      }
    } else {
    }
  }
  return -1;
}

size_t object_size(char *layout) {
  size_t size_layout = 0;
  int index = 0;
  while (layout[index] != '\0') {
    if (layout[index] == '*') {
      size_layout += 8;
    } else if (layout[index] == 'l' || layout[index] == 'd') {
      size_layout += 8;
    } else {
      size_layout += 4;
    }
    index++;
  }
  return size_layout;
}

void set_bits_in_alloc_map(uint64_t *alloc_map, int start_index, int bytes) {
  // which uint64 in the allocation map
  int index_map = start_index / 128;
  // from which bit in this uint64
  int bits_in_page = start_index % 128;

  // number of bits to set
  int bits = bytes / 16;

  uint64_t *part1 = &alloc_map[index_map * 2];
  uint64_t *part2 = &alloc_map[index_map * 2 + 1];

  for (int i = 0; i < bits; i++) {
    if (i + bits_in_page < 64) {
      // lower part of page
      *part1 |= (1ULL << (63 - (i + bits_in_page)));
    } else {
      // higher part of page, go to next page
      *part2 |= (1ULL << (63 - (i + bits_in_page - 64)));
    }
  }
}

void *h_alloc_struct(heap_t *h, char *layout) {
  size_t allocated_bytes = count_allocated_bytes_on_heap(h);
  if (((float)allocated_bytes / (float)h->heap_size) > h->GC_threshold) {
    size_t reclaimed = h_gc(h);
    if (DEBUG_MODE) {
      puts("=== GC report ===\n");
      printf("\nGC collected: %ld bytes\n", reclaimed);
      puts("\n=================");
    }
  }

  // calculate the size of object and the total size with header and padding
  size_t obj_size = object_size(layout);

  // Align objects
  // I need to move the ptr later so it still lines up with the allocation map,
  // so in parts off 16 bytes how much i need to add: (16 - (size of object +
  // header(8 bytes) % 16)) % 16
  int bytes_to_add = (16 - ((obj_size + HEADER_SIZE) % 16)) % 16;
  int total_size = obj_size + HEADER_SIZE + bytes_to_add;

  // find next free space for the objects total size
  int page_index = find_next_available(h, total_size);

  // if no available page exist
  if (page_index == -1) {
    // TODO: could try to run an a gc before
    printf("object dont fit on any of the pages left, heap could be full or to "
           "big object size");
    assert(!"Object dont fit on any page");
  }

  // create a header
  page_t *page = h->page_array[page_index];
  set_layout_header(layout, page->next_empty_space);

  // create a new ptr, pointing after header(ptr to return)
  void *ptr_to_obj = (void *)((char *)page->next_empty_space + HEADER_SIZE);

  uint8_t *tmp = (uint8_t *)ptr_to_obj;

  // reset all space in memory
  int elements_to_set = obj_size;
  for (int i = 0; i < elements_to_set; i++) {
    tmp[i] = 0;
  }

  // flipp the bits in the allocation map
  // find between which index in the alloc map we should set, start and how many
  int bits_per_page = 2048 / 16; // = 128
  int bits_to_obj_start_in_page =
      ((char *)page->next_empty_space - (char *)page->page_start) / 16;
  int start_index = (bits_per_page * page_index) + bits_to_obj_start_in_page;

  set_bits_in_alloc_map(h->alloc_map, start_index, total_size);

  // move next ptr
  page->next_empty_space =
      (void *)((char *)page->next_empty_space + total_size);

  // change remaining size
  page->remaining_size -= total_size;

  // return ptr that points to space after header and before the object
  return ptr_to_obj;
}

void *h_alloc_raw(heap_t *h, size_t bytes) {
  size_t allocated_bytes = count_allocated_bytes_on_heap(h);
  if (((float)allocated_bytes / (float)h->heap_size) > h->GC_threshold) {
    size_t reclaimed = h_gc(h);
    // TODO: borde vara i debug mode
    if (DEBUG_MODE) {
      puts("=== GC report ===\n");
      printf("\nGC collected: %ld bytes\n", reclaimed);
      puts("\n=================");
    }
  }

  // calculate total size
  int bytes_to_add = (16 - ((bytes + HEADER_SIZE) % 16)) % 16;
  int total_size = bytes + HEADER_SIZE + bytes_to_add;

  int page_index = find_next_available(h, total_size);

  // create a header
  page_t *page = h->page_array[page_index];
  void *head = page->next_empty_space;

  // header contains size bit-shifted 3 bits to the left, followed by 3-bit
  // metadata tag move in 3 bit and set code for decoding which header
  *((uint64_t *)head) = ((uint64_t)bytes << 3) | 0x3;

  // move ptr to after header (ptr to return)
  void *ptr_to_obj = (void *)((uint8_t *)head + HEADER_SIZE);

  // flipp the bits in the allocation map
  int bits_per_page = 2048 / 16; // = 128
  int bits_to_obj_start_in_page =
      ((char *)page->next_empty_space - (char *)page->page_start) / 16;
  int start_index = (bits_per_page * page_index) + bits_to_obj_start_in_page;

  set_bits_in_alloc_map(h->alloc_map, start_index, total_size);

  // update next ptr
  page->next_empty_space =
      (void *)((char *)page->next_empty_space + total_size);

  // Update remaining_size
  page->remaining_size -= total_size;

  // return ptr pointing to just after header
  return ptr_to_obj;
}
