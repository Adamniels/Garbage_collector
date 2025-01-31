#include "../src/gc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
  Simple driver application using a rudimentary singly linked list storing
  integer values, allocated on the managed heap using the gc
*/

typedef struct node node_t;

struct __attribute__((packed)) node {
  struct node *next;
  long int val;
};

typedef struct __attribute__((packed)) list {
  struct node *head;
} list_t;

// allocate a list on the managed heap
list_t *create_list_demo(heap_t *h) {
  char *list_layout = "*";
  list_t *list = (list_t *)h_alloc_struct(h, list_layout);
  if (list == NULL) {
    printf("\nMemory allocation failed\n");
    exit(1);
  }
  list->head = NULL;
  return list;
}

// allocate a new node on the managed heap, storing the int val
node_t *create_node_demo(heap_t *h, int val) {
  char *node_layout = "*ii";
  struct node *new_node = (node_t *)h_alloc_struct(h, node_layout);
  if (new_node == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }
  new_node->val = val;
  new_node->next = NULL;
  return new_node;
}

// prepend a new node storing int val
void prepend(heap_t *h, list_t *list, int val) {
  node_t *new_node = create_node_demo(h, val);
  new_node->next = list->head;
  list->head = new_node;
}

// remove the first node containing the value val
// note: due to GC we do not free unlinked nodes
void delete_node(struct list *list, int val) {
  if (list->head == NULL)
    return;
  node_t *temp = list->head;
  if (temp->val == val) {
    list->head = temp->next;
    return;
  }

  node_t *prev = NULL;
  while (temp != NULL && temp->val != val) {
    prev = temp;
    temp = temp->next;
  }

  if (temp == NULL) {
    return;
  }
  prev->next = temp->next;
}

// search list for int val, return true if present, false if absent
bool contains(list_t *list, int val) {
  size_t index = 0;
  node_t *p = list->head;
  node_t *dbg_last_p;
  while (p != NULL && index < 1024) {
    if (p->val == val) {
      return true;
    }
    dbg_last_p = p;
    p = p->next;
    index++;
  }
  return false;
}

// print list, debug method
void print_list(struct list *list) {
  struct node *temp = list->head;
  while (temp != NULL) {
    printf("%ld -> ", temp->val);
    temp = temp->next;
  }
  printf("NULL\n");
}

// exercise the GC by repeatedly allocating, traversing, dropping and replacing
// linked lists
int demo(void) {

  // 1 MiB heap
  unsigned long heap_size = 2UL * 1024UL * 1024UL;
  heap_t *h = h_init(heap_size, false, 0.4);
  unsigned long list_length =
      1 * 1024; // list size = 96 KiB = 8*1024*sizeof(node_t) + sizeof(list_t)
  unsigned long list_size = list_length * sizeof(node_t) + sizeof(list_t);

  printf("Initializing 4 linked lists of %lu bytes each, using a total of %lu "
         "bytes, out of a total of %lu bytes\n",
         list_size, list_size, heap_size);

  list_t *list1;
  list_t *list2;
  list_t *list3;
  list_t *list4;
  unsigned long reclaimed;
  unsigned long found1;
  unsigned long found2;
  unsigned long found3;
  unsigned long found4;
  unsigned long prime1 = 3;
  unsigned long prime2 = 5;
  unsigned long prime3 = 7;
  unsigned long prime4 = 11;
  int repeats = 2;

  for (int i = 0; i < repeats; i++) {
    list1 = create_list_demo(h);
    list2 = create_list_demo(h);
    list3 = create_list_demo(h);
    list4 = create_list_demo(h);
    /* add <list_length> known prime multiples to each list */
    for (int i = 1; i <= list_length; i++) {
      prepend(h, list1, prime1 * i);
      prepend(h, list2, prime2 * i);
      prepend(h, list3, prime3 * i);
      prepend(h, list4, prime4 * i);
      if (i % 10 == 1) {
        reclaimed = h_gc(h);
        puts("\n=== GC report running gc every 10 loop===\n");
        printf("Should not collect anything yet becuase we\n");
        printf("are still filling up the lists\n");
        printf("\nGC collected: %lu bytes\n", reclaimed);
        puts("\n=========================================\n");
      }
    }

    printf("\nFinished filling 4 linked lists of %lu bytes each, using a total "
           "of %lu bytes\n",
           list_size, 4 * list_size);

    printf("Currently using %lu bytes, with %lu bytes available\n", h_used(h),
           h_avail(h));

    printf("Before GC: list1 at address %p\n", list1);
    /* run GC, moving all allocated nodes to new heap pages */
    reclaimed = h_gc(h);
    puts("\n=== GC report after filling the list, still not removed anything "
         "===\n");
    printf("Should not collect anything still\n");
    printf("\nGC collected: %lu bytes\n", reclaimed);
    puts("\n==================================================================="
         "==\n");
    printf("After GC: list1 at address %p\n", list1);
    /* validate the lists after GC to ensure that they still contain the same
     * numbers */

    // TODO: Fix segfault from contains()
    found1 = 0;
    found2 = 0;
    found3 = 0;
    found4 = 0;
    for (int i = 1; i <= list_length; i++) {
      found1 += contains(list1, prime1 * i)
                    ? 1
                    : 0; // SEGFAULTS: efter 820 noder är next utanför heapen
                         // men ej null, alltså corrupted!
      found2 += contains(list2, prime2 * i) ? 1 : 0;
      found3 += contains(list3, prime3 * i) ? 1 : 0;
      found4 += contains(list4, prime4 * i) ? 1 : 0;
    }
    puts("\n=== Validating we can still reach each element after multiple GCs "
         "===\n");
    printf("Found %lu of %lu list entries in list %d\n", found1, list_length,
           1);
    printf("Found %lu of %lu list entries in list %d\n", found2, list_length,
           2);
    printf("Found %lu of %lu list entries in list %d\n", found3, list_length,
           3);
    printf("Found %lu of %lu list entries in list %d\n", found4, list_length,
           4);
    puts("\n==================================================================="
         "==\n");

    /* clear stack pointers in heap, allowing future GC to cleanup */
    list1 = NULL;
    list2 = NULL;

    puts("=== GC report, now we have set list 1 and 2 to null ===\n");
    printf("Currently using %lu bytes, with %lu bytes available\n", h_used(h),
           h_avail(h));
    reclaimed = h_gc(h);
    printf("Therefore we should now reclaim half of all the memory");
    printf("\nGC collected: %ld bytes\n", reclaimed);
    puts("\n=================");
  }
  /* final GC */
  list3 = NULL;
  list4 = NULL;
  reclaimed += h_gc(h);
  puts("=== Final GC report set everthing to null, the total of all GCs ===\n");
  printf("\nGC Total collected: %ld bytes\n", reclaimed);
  puts("\n===========================================");

  puts("=== validation report ===\n");
  printf("Allocated 4 lists of %ld bytes each %d times, for a total of %ld "
         "bytes, on a heap of size %ld bytes",
         list_size, repeats, 4 * repeats * list_size, heap_size);
  puts("\n=================");

  h_delete(h);
  return EXIT_SUCCESS;
}

int main(void) { return demo(); }
