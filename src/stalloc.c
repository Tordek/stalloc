#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdint.h>

#include "stalloc.h"

struct smhead *st_create(void *memory, size_t size)
{
#ifndef NDEBUG
  assert(((void)"The arena must be able to hold enough data for the head and a node.",
          size >= sizeof(struct smhead) + sizeof(struct smnode)));
  assert(((void)"The arena must be aligned to a pointer size.",
          (uintptr_t)memory % sizeof(void *) == 0));
#endif

  struct smhead *head = (struct smhead *)memory;
  head->first = (struct smnode *)((char *)memory + sizeof(struct smhead));
  struct smnode *node = head->first;

  size -= sizeof(struct smhead);
  size -= sizeof(size_t);
  node->size = size;
  node->next = NULL;
  return head;
}

void *st_alloc(struct smhead *stalloc, size_t size)
{
  struct smnode *chosen = NULL;
  size_t smallest_node_size = SIZE_MAX;

  // Find the smallest node that can fit the data.
  for (struct smnode *current = stalloc->first; current != NULL; current = current->next)
  {
    if (current->size >= size && current->size < smallest_node_size)
    {
      chosen = current;
      smallest_node_size = current->size;
    }
  }

  // Allocation fails if no node is large enough.
  if (!chosen)
  {
    return NULL;
  }

  // If the node is larger than needed, split it.
  if (chosen->size > size)
  {
    size_t node_size = size + sizeof(size_t);
    struct smnode *tail = (struct smnode *)((char *)chosen + node_size);
    tail->size = chosen->size - node_size;
    tail->next = chosen->next;
    chosen->next = tail;
    chosen->size = size;
  }

  // Remove the chosen node from the free list.
  stalloc->first = chosen->next;

  return &(chosen->data);
}

void st_free(struct smhead *stalloc, void *memory)
{
  // Get the real node pointer.
  struct smnode *node = (struct smnode *)((char *)memory - sizeof(size_t));

  // The list is empty, add the first node.
  if (stalloc->first == NULL)
  {
    stalloc->first = node;
    node->next = NULL;
    return;
  }

  // The node to be freed is before the first node.
  if (node < stalloc->first)
  {
    node->next = stalloc->first;
    stalloc->first = node;
    return;
  }

  struct smnode *prev = stalloc->first;
  // Find the next node that is located after the node to be freed
  for (struct smnode *current = prev->next; current != NULL; prev = current, current = current->next)
  {
    if (current > node)
    {
      node->next = current;
      break;
    }
  }

  // The end of the list was reached, or a place is found.
  prev->next = node;
}

void st_defrag(struct smhead *stalloc)
{
  struct smnode *current = stalloc->first;
  while (current != NULL)
  {
    struct smnode *next = current->next;

    // If the node points to the node immediately following it, it can be merged.
    if ((char *)current + current->size + sizeof(size_t) == (char *)next)
    {
      current->size += next->size + sizeof(size_t);
      current->next = next->next;
      // If the node was merged, try again.
    }
    else
    {
      current = current->next;
    }
  }
}