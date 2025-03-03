#include <stddef.h>

//* A `smnode` is a node in a singly-linked list of free memory blocks.
//* The `size` field stores the size of the memory block in bytes NOT including
//* the size of the `size` field itself.
//* The `next` field is a pointer to the next free memory block. It points
//* to the start of the next node, or is `NULL` if this is the last node.
//* The `data` field is a dummy pointer to the allocated memory.
//* Allocated memory points to the *data field, so the `size` field is not
//* used in allocated memory blocks.
//* Each allocation has `sizeof(size_t)` (aligned to a pointer size) bytes of
//* overhead.
struct smnode
{
  size_t size;
  union
  {
    struct smnode *next;
    void *data;
  };
};

//* A `smhead` is a header for a singly-linked list of free memory blocks.
//* The `next` field is a pointer to the first free memory block.
struct smhead
{
  struct smnode *first;
};

//* In order to create a Stalloc instance, you must provide a memory block
//* and its size. The memory block must be aligned to the size of a pointer.
//* The size of the memory block must be at least `sizeof(struct smnode)`.
struct smhead *st_create(void *memory, size_t size);

//* Allocates a memory block of the given size. The size must be greater than
//* zero. The returned memory block is aligned to the size of a pointer.
//* Returns `NULL` if the memory block could not be allocated.
void *st_alloc(struct smhead *stalloc, size_t size);

//* Frees a memory block that was previously allocated with `st_alloc`.
//* Nodes are sorted in ascending order by memory address.
void st_free(struct smhead *stalloc, void *memory);

//* Defragments the memory block by merging adjacent free memory blocks.
void st_defrag(struct smhead *stalloc);