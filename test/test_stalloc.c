#include "unity.h"
#include "stalloc.h"

void *backing;
struct smhead *stalloc;
void setUp(void)
{
  backing = malloc(1024);
  stalloc = st_create(backing, 1024);
}
void tearDown(void)
{
  free(backing);
}

size_t stalloc_node_count(struct smhead *stalloc)
{
  size_t count = 0;
  for (struct smnode *current = stalloc->first; current != NULL; current = current->next)
  {
    count++;
  }
  return count;
}

void test_alloc_stalloc(void)
{
  void *memory = st_alloc(stalloc, 512);
  TEST_ASSERT_NOT_NULL_MESSAGE(memory, "Allocating memory should not return NULL.");
}

void test_stalloc_node_initial_count(void)
{
  TEST_ASSERT_EQUAL_MESSAGE(1, stalloc_node_count(stalloc), "After creation, there should be one node.");
}

void test_stalloc_node_count_after_alloc(void)
{
  st_alloc(stalloc, 512);
  TEST_ASSERT_EQUAL_MESSAGE(1, stalloc_node_count(stalloc), "After allocation, if there's enough room, there should be one free node.");
}

void test_stalloc_node_count_after_consuming_all(void)
{
  st_alloc(stalloc, 1024 - sizeof(size_t) - sizeof(struct smhead));
  TEST_ASSERT_EQUAL_MESSAGE(0, stalloc_node_count(stalloc), "After allocation, if all memory is consumed, there should be no free nodes.");
}

void test_stalloc_node_count_after_free(void)
{
  void *memory = st_alloc(stalloc, 512);
  st_free(stalloc, memory);
  TEST_ASSERT_EQUAL_MESSAGE(2, stalloc_node_count(stalloc), "After freeing, there should be two free nodes.");
}

void test_stalloc_node_count_after_consuming_all_memory(void)
{
  void *memory = st_alloc(stalloc, 1024 - sizeof(size_t) - sizeof(struct smhead));
  st_free(stalloc, memory);
  TEST_ASSERT_EQUAL_MESSAGE(1, stalloc_node_count(stalloc), "If consuming all memory, there should be one free node after freeing.");
}

void test_stalloc_node_count_after_free_second(void)
{
  void *memory1 = st_alloc(stalloc, 64);
  void *memory2 = st_alloc(stalloc, 64);
  st_free(stalloc, memory2);
  TEST_ASSERT_EQUAL_MESSAGE(2, stalloc_node_count(stalloc), "After freeing the second node, there should be two free nodes.");
}

void test_stalloc_node_count_after_free_first(void)
{
  void *memory1 = st_alloc(stalloc, 64);
  void *memory2 = st_alloc(stalloc, 64);
  st_free(stalloc, memory1);
  TEST_ASSERT_EQUAL_MESSAGE(2, stalloc_node_count(stalloc), "After freeing the first node, there should be two free nodes.");
}

void test_stalloc_node_count_after_free_both_in_order(void)
{
  void *memory1 = st_alloc(stalloc, 64);
  void *memory2 = st_alloc(stalloc, 64);
  st_free(stalloc, memory1);
  st_free(stalloc, memory2);
  TEST_ASSERT_EQUAL_MESSAGE(3, stalloc_node_count(stalloc), "After freeing both nodes, there should be three free nodes.");
}

void test_stalloc_node_count_after_free_both_in_reverse_order(void)
{
  void *memory1 = st_alloc(stalloc, 64);
  void *memory2 = st_alloc(stalloc, 64);
  st_free(stalloc, memory2);
  st_free(stalloc, memory1);
  TEST_ASSERT_EQUAL_MESSAGE(3, stalloc_node_count(stalloc), "After freeing both nodes, there should be three free nodes.");
}

void test_stalloc_node_count_after_free_multiple_in_order(void)
{
  void *memory1 = st_alloc(stalloc, 64);
  void *memory2 = st_alloc(stalloc, 64);
  void *memory3 = st_alloc(stalloc, 64);
  st_free(stalloc, memory1);
  st_free(stalloc, memory2);
  st_free(stalloc, memory3);
  TEST_ASSERT_EQUAL_MESSAGE(4, stalloc_node_count(stalloc), "After freeing both nodes, there should be three free nodes.");
}

void test_stalloc_node_count_after_free_multiple_in_reverse_order(void)
{
  void *memory1 = st_alloc(stalloc, 64);
  void *memory2 = st_alloc(stalloc, 64);
  void *memory3 = st_alloc(stalloc, 64);
  st_free(stalloc, memory3);
  st_free(stalloc, memory2);
  st_free(stalloc, memory1);
  TEST_ASSERT_EQUAL_MESSAGE(4, stalloc_node_count(stalloc), "After freeing both nodes, there should be three free nodes.");
}

void test_stalloc_node_count_after_free_both_after_consuming_all(void)
{
  void *memory1 = st_alloc(stalloc, 64);
  void *memory2 = st_alloc(stalloc, 1024 - 64 - sizeof(size_t) - sizeof(struct smhead) - sizeof(size_t));
  st_free(stalloc, memory1);
  st_free(stalloc, memory2);
  TEST_ASSERT_EQUAL_MESSAGE(2, stalloc_node_count(stalloc), "After freeing both nodes when consuming all memory, there should be two free nodes.");
}

void test_stalloc_node_count_after_free_both_reverse_order_after_consuming_all(void)
{
  void *memory1 = st_alloc(stalloc, 64);
  void *memory2 = st_alloc(stalloc, 1024 - 64 - sizeof(size_t) - sizeof(struct smhead) - sizeof(size_t));
  st_free(stalloc, memory2);
  st_free(stalloc, memory1);
  TEST_ASSERT_EQUAL_MESSAGE(2, stalloc_node_count(stalloc), "After freeing both nodes when consuming all memory, there should be two free nodes.");
}

void test_stalloc_requesting_too_much_memory_fails(void)
{
  void *memory = st_alloc(stalloc, 1024);
  TEST_ASSERT_NULL_MESSAGE(memory, "Requesting too much memory should fail.");
}

void test_stalloc_defrag_noop(void)
{
  st_alloc(stalloc, 64);
  st_alloc(stalloc, 64);
  st_alloc(stalloc, 64);
  st_defrag(stalloc);
  TEST_ASSERT_EQUAL_MESSAGE(1, stalloc_node_count(stalloc), "Defrag should not change the number of nodes if no merging is possible.");
}

void test_stalloc_alloc_works_with_fragmented_memory(void)
{
  void *memory1 = st_alloc(stalloc, 128);
  void *memory2 = st_alloc(stalloc, 64);
  void *memory3 = st_alloc(stalloc, 64);
  st_free(stalloc, memory2);
  st_free(stalloc, memory1);
  void *memory = st_alloc(stalloc, 64);
  TEST_ASSERT_NOT_NULL_MESSAGE(memory, "Allocating memory should work with fragmented memory.");
}

void test_stalloc_alloc_works_with_fragmented_memory2(void)
{
  void *memory1 = st_alloc(stalloc, 128);
  void *memory2 = st_alloc(stalloc, 64);
  void *memory3 = st_alloc(stalloc, 64);
  st_free(stalloc, memory1);
  st_free(stalloc, memory2);
  st_free(stalloc, memory3);
  void *memory = st_alloc(stalloc, 32);
  TEST_ASSERT_NOT_NULL_MESSAGE(memory, "Allocating memory should work with fragmented memory.");
}

void test_stalloc_defrag_merges_nodes(void)
{
  void *memory1 = st_alloc(stalloc, 64);
  void *memory2 = st_alloc(stalloc, 64);
  st_free(stalloc, memory1);
  st_free(stalloc, memory2);
  st_defrag(stalloc);
  TEST_ASSERT_EQUAL_MESSAGE(1, stalloc_node_count(stalloc), "Defrag should merge nodes.");
}

void test_stalloc_defrag_merges_nodes_freed_in_any_order(void)
{
  void *memory1 = st_alloc(stalloc, 64);
  void *memory2 = st_alloc(stalloc, 64);
  st_free(stalloc, memory2);
  st_free(stalloc, memory1);
  st_defrag(stalloc);
  TEST_ASSERT_EQUAL_MESSAGE(1, stalloc_node_count(stalloc), "Defrag should merge nodes freed in a different order.");
}