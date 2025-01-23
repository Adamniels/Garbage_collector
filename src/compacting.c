#include "compacting.h"
#include "allocation.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// returns array of pointers to all pages with is_active == filter
// the size of the array is written into *num_filtered_pages
// caller owns array and is responsible for freeing it
page_t **filter_pages_by_activity(heap_t *heap, bool filter,
                                  size_t *num_filtered_pages) {

  size_t filtered_page_count = 0;
  for (size_t i = 0; i < heap->page_amount; i++) {
    filtered_page_count += (heap->page_array[i]->is_active == filter) ? 1 : 0;
  }

  page_t **filtered_page_array = calloc(filtered_page_count, sizeof(page_t *));

  size_t current_index = 0;
  for (size_t i = 0; i < heap->page_amount; i++) {
    if (heap->page_array[i]->is_active == filter) {
      filtered_page_array[current_index++] = heap->page_array[i];
    }
  }
  *num_filtered_pages = filtered_page_count;
  return filtered_page_array;
}

// returns array of pointers to all currently passive pages
// the size of the array is written into *num_passive_pages
// caller owns array and is responsible for freeing it
page_t **find_passive_pages(heap_t *heap, size_t *num_passive_pages) {
  return filter_pages_by_activity(heap, false, num_passive_pages);
}

// returns array of pointers to all currently active pages
// the size of the array is written into *num_active_pages
// caller owns array and is responsible for freeing it
page_t **find_active_pages(heap_t *heap, size_t *num_active_pages) {
  return filter_pages_by_activity(heap, true, num_active_pages);
}

// Takes a pointer to a heap object and interprets its header,
// returning an array containing all pointers contained within the object
// Example: Header is [...] 0000 1101 0000 b2 b1 b0 -> returns array containing
// 2 pointers the size of the array is written into num_pointers the size (in
// bytes) of the object (excluding header) is written into obj_size (Header:
// first "1" indicates start, after which "0" = 4-byte data, "1" = 8-byte
// pointer) PRE: p is a pointer into the heap, or NULL
void ***interpret_header(void *p, size_t *num_pointers, size_t *object_size) {

  if (p == NULL) {
    return NULL;
  }

  uint64_t header = *((uint64_t *)p - 1);
  uint8_t b1b0 = 0x3 & header;      // extract first and second bit code
  uint8_t b2 = (0x4 & header) >> 2; // extract third bit code

  if (b1b0 != 0x3) { // not a bitvector, TODO: handle this case too
    return NULL;
  }

  if (b2 == 0) {
    // case where we only have a size
    *object_size = (size_t)(header >> 3); // matches layout in h_alloc.c
    *num_pointers = 0;
    return NULL;
  }

  // the rest is the case where we have a format bitvector
  // count number of 1's and set size
  uint8_t current_bit;
  size_t count = 0;
  for (int i = 3; i < 64;
       i++) { // loop over header backwards, skipping the last 3 bits
    current_bit = 0x1 & (header >> i);
    if (current_bit == 0x1) {
      count++;
    }
  }
  count -= 1; // do not count the leading "1"

  void ***pointer_array = calloc(count, sizeof(void **));

  size_t current_index = 0;
  size_t current_offset = 0;
  bool padding_cleared = false; // to avoid counting leading 0's

  for (int i = 63; i > 2;
       i--) { // loop over header forwards, skipping the last 3 bits

    current_bit = 0x1 & (header >> i);

    if (current_bit == 0x1 && !padding_cleared) {
      // first 1 so we should count from here
      padding_cleared = true;
    } else if (current_bit == 0x1 && padding_cleared) {
      // 1's correspond to pointers, which are 8 bytes on target platform
      // we have a void* stored <current_offset> bytes ahead of p, and we want
      // its address (so a void**) we use uint8_t pointer arithmetic to compute
      // this address and then cast the result to a void**
      if (*(void **)((uint8_t *)p + current_offset) == NULL) {
        // in case we have a null ptr in our object we dont want to add it
        // decrease count(nr of ptr to follow in the obj)
        count--;
        current_offset += 8;
        continue;
      }
      pointer_array[current_index] = (void **)((uint8_t *)p + current_offset);
      current_index++;
      current_offset += 8;
    } else if (padding_cleared) {
      // non-leading 0's correspond to 4-byte data blocks
      current_offset += 4;
    }
  }

  *object_size = current_offset; // pass object size in bytes to caller
  *num_pointers = count;         // pass array size to caller
  return pointer_array; // return array, caller owns it and must free it
}

// helper function to check if an object header (where p points to the object)
// is a forwarding address
bool header_is_forwarding_address(void *p) {
  if (p == NULL)
    return false;
  uint64_t header = *((uint64_t *)p - 1);
  uint64_t tag = header & 0x3; // extract b1b0
  return (tag == 0x1) ? true : false;
}

// equal function, works becuase i actually add the forwarding adress not the
// double ptr
// TODO: finns denna redan i linked list längst ner?
int eq_function_ptr(elem_t a, elem_t b) {
  return (a.ptr > b.ptr) - (a.ptr < b.ptr);
}

// Traverses the object graph starting from the stack roots,
// copies all the objects (including headers) to currently passive pages,
// replaces the old object headers with the new address of the object,
// and resets the previously active pages to passive status
// NOTE: leaves stale pointers inside moved objects, to be remedied by a later
// traversal ASSUMES: that there are sufficiently many passive pages to fit all
// currently active objects
//          we make no assumption as to where on the heap these pages are
//          placed, but allocation must ensure they exist!
void traverse_and_move(heap_t *h, ioopm_list_t *root_list) {
  size_t num_passive_pages, num_active_pages;
  page_t **passive_page_array = find_passive_pages(h, &num_passive_pages);
  page_t **active_page_array = find_active_pages(h, &num_active_pages);

  // BFS where root_list is copied to a list with unvisited nodes
  // loops are avoided since the layout bitmap is overwritten by a forwarding
  // address

  ioopm_list_t *queue = ioopm_linked_list_create(eq_function_ptr);
  for (size_t j = 0; j < ioopm_linked_list_size(root_list); j++) {
    elem_t res;
    ioopm_linked_list_get(root_list, j, &res);
    ioopm_linked_list_append(queue, res);
  }

  // when visiting a node, read its layout and extract any pointers from it,
  // adding them into the root list
  size_t obj_size;
  size_t num_pointers;
  void **current_pointer;
  void ***pointer_array;
  elem_t temp_elem; // for compatibility with linked list library
  // for each unvisited element
  while (ioopm_linked_list_remove(queue, 0, &temp_elem)) {

    current_pointer = (void **)temp_elem.ptr;

    // check if this object is already visited once (i.e. header is forwarding
    // address), in which case we skip it entirely forwarding address ends in
    // 0b01
    if (header_is_forwarding_address(*current_pointer)) {
      continue;
    }

    pointer_array =
        interpret_header(*current_pointer, &num_pointers, &obj_size);
    // check if this object contains references
    if (pointer_array != NULL) {
      // if header was a layout bitmap containing pointers, add them to list of
      // unvisited
      for (size_t i = 0; i < num_pointers; i++) {
        elem_t tmp;
        tmp.ptr = pointer_array[i];
        ioopm_linked_list_append(queue, tmp);
      }
      free(pointer_array);
    } else {
      obj_size = 0;
    }

    // exploration step done, now we must move this object to a new page

    // iterate across passive_page_array until we find the first page where this
    // object fits
    page_t *new_page = NULL;
    for (size_t i = 0; i < num_passive_pages; i++) {
      if ((obj_size + HEADER_SIZE) < passive_page_array[i]->remaining_size) {
        new_page = passive_page_array[i];
        break;
      }
    }

    if (new_page == NULL) {
      exit(1); // TODO: handle this error case
    }

    // move object (including header)
    void *old_header_address = (void *)((uint64_t *)(*current_pointer) - 1);
    void *new_header_address = new_page->next_empty_space;
    size_t byte_size_including_header = obj_size + 8;
    memcpy(new_header_address, old_header_address, byte_size_including_header);

    // update page metadata to reflect new allocation
    // Update allocation map
    int bytes_to_add = (16 - ((obj_size + HEADER_SIZE) % 16)) % 16;
    int total_size = obj_size + HEADER_SIZE + bytes_to_add;
    int page_index = new_page->index;
    int bits_per_page = 2048 / 16; // = 128
    int bits_to_obj_start_in_page =
        ((char *)new_page->next_empty_space - (char *)new_page->page_start) /
        16;
    int start_index = (bits_per_page * page_index) + bits_to_obj_start_in_page;

    set_bits_in_alloc_map(h->alloc_map, start_index, total_size);
    new_page->next_empty_space =
        (void *)(((uint8_t *)new_header_address) + total_size);
    new_page->remaining_size -= total_size;
    new_page->is_active = true;

    // replace old header with tagged pointer to new location, tag b1b0 = 0b01
    // means forwarding address
    uint64_t forwarding_address =
        (uint64_t)((uint64_t *)new_header_address + 1) | 0x1;
    *((uint64_t *)old_header_address) = forwarding_address;
  }

  // make all the prev active pages passive
  for (size_t i = 0; i < num_active_pages; i++) {
    active_page_array[i]->is_active = false;
    active_page_array[i]->remaining_size = PAGE_SIZE;
    active_page_array[i]->next_empty_space = active_page_array[i]->page_start;
    size_t index_page = active_page_array[i]->index;
    h->alloc_map[index_page] = 0;
    h->alloc_map[index_page + 1] = 0;
  }

  // deallocate linked list and page arrays
  free(passive_page_array);
  free(active_page_array);
  ioopm_linked_list_destroy(queue);
}

uint64_t extract_adress(uint64_t header) { return header & ~0x3; }

// NOTE: here i assum the root list has ptr to ptr that points to the object so
// void**
// // TODO: behöver jag ens ta in heapen???
void traverse_and_forward(heap_t *h, ioopm_list_t *root_list) {

  ioopm_list_t *queue = ioopm_linked_list_create(eq_function_ptr);

  // add roots to queue
  for (size_t j = 0; j < ioopm_linked_list_size(root_list); j++) {
    elem_t res;
    ioopm_linked_list_get(root_list, j, &res);
    ioopm_linked_list_append(queue, res);
  }

  ioopm_list_t *visited = ioopm_linked_list_create(eq_function_ptr);

  while (!ioopm_linked_list_is_empty(queue)) {
    elem_t result;
    ioopm_linked_list_remove(queue, 0, &result);
    void **obj_ptr = result.ptr;
    void *obj_adress = *obj_ptr;

    if (obj_ptr == NULL) {
      // TODO: borde inte kunna vara null så ändra till en assert
      assert(obj_ptr != NULL);
      continue;
    }
    // extract the forwarding adress from the header
    uint64_t header = *((uint64_t *)obj_adress - 1);
    // expect to always have a forward adress
    assert(header_is_forwarding_address(obj_adress));

    void *forwarding_adress = (void *)extract_adress(header);

    // change the objects pointer to point to the forwarding adress instead
    *obj_ptr = forwarding_adress;

    // if the forwarding adress already been visited before, i don't want to do
    // anything becuase it's pointers are already in the queue
    if (ioopm_linked_list_contains(visited, (elem_t){.ptr = obj_adress})) {
      continue;
    }
    // otherwise add it to the visited and continue
    ioopm_linked_list_append(visited, (elem_t){.ptr = obj_adress});

    // follow the forwarding adress
    // with the layout information add its pointer to the queue
    size_t num_pointers;
    size_t obj_size;
    void ***ptrs_in_obj =
        interpret_header(forwarding_adress, &num_pointers, &obj_size);

    if (num_pointers == 0) {
      // means there was no actuall ptr to follow
      free(ptrs_in_obj);
      continue;
    }

    // otherwise add the pointer to the queue
    for (size_t i = 0; i < num_pointers; i++) {
      void **ptr_to_ptr_to_obj = ptrs_in_obj[i];
      ioopm_linked_list_append(queue, (elem_t){.ptr = ptr_to_ptr_to_obj});
    }
    free(ptrs_in_obj);
  }
  ioopm_linked_list_destroy(queue);
  ioopm_linked_list_destroy(visited);
}

size_t count_allocated_bytes_on_heap(heap_t *h) {
  size_t total_bytes_allocated = 0;
  for (size_t i = 0; i < h->page_amount; i++) {
    page_t *page = h->page_array[i];
    total_bytes_allocated +=
        (char *)page->next_empty_space - (char *)page->page_start;
  }
  return total_bytes_allocated;
}

size_t h_gc(heap_t *h) { return h_gc_dbg(h, !h->safe); }

size_t h_gc_dbg(heap_t *h, bool unsafe_stack) {

  // iterate over pages to count size usage
  size_t initial_size_usage = count_allocated_bytes_on_heap(h);

  ioopm_list_t *root_list = find_gc_roots(h);

  // 1st traversal to find all objects (avoid loops by checking forwarding
  // address)
  traverse_and_move(h, root_list);

  // 2nd traversal to replace all occurences of old pre-compacting addresses
  // with new addresses
  traverse_and_forward(h, root_list);

  size_t new_size_usage = count_allocated_bytes_on_heap(h);

  ioopm_linked_list_destroy(root_list);

  return initial_size_usage - new_size_usage;
}

size_t h_avail(heap_t *h) {
  return h->page_amount * PAGE_SIZE - count_allocated_bytes_on_heap(h);
}

size_t h_used(heap_t *h) { return count_allocated_bytes_on_heap(h); }
