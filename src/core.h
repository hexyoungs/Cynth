#ifndef _CSYNTH_CORE_H
#define _CSYNTH_CORE_H

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>

#define HZ_44100 44100  // cd
#define free_to_null(ptr) \
  if (ptr != NULL) {      \
    free(ptr);            \
    ptr = NULL;           \
  }

// ========================================== utils

typedef struct ListNode {
  void* data;
  struct ListNode* next;
} ListNode;

typedef struct List {
  ListNode* head;
} List;

static inline List* list_new();
static inline ListNode* list_add(List* list, void* data);
static inline void _list_free(List* list, int free_list);

// ========================================== helper macros
#define list_nodes_foreach(node) for (; node != NULL; node = node->next)
#define list_foreach(list, fn)   \
  ListNode* node = (list)->head; \
  list_nodes_foreach(node) { fn(node->data); }

#define list_free(list) _list_free(list, 1)
#define list_nodes_free(list) _list_free(list, 0)

// impls

static inline List* list_new() {
  List* ptr = (List*)malloc(sizeof(List));
  ptr->head = NULL;
  return ptr;
}

static inline ListNode* list_add(List* list, void* data) {
  ListNode* node = list->head;
  ListNode* prev = NULL;
  while (node != NULL) {
    prev = node;
    node = node->next;
  }

  ListNode* next = (ListNode*)malloc(sizeof(ListNode));
  next->data = data;
  next->next = NULL;
  if (list->head == NULL) {
    list->head = next;
  } else {
    prev->next = next;
  }
  return next;
}

static inline void _list_free(List* list, int free_list) {
  if (list == NULL) return;
  ListNode* cur = list->head;
  ListNode* prev = list->head;
  while (cur != NULL) {
    prev = cur;
    cur = cur->next;
    free(prev);
  }
  if (free_list) free(list);
}

// ========================================== platform
struct Device;
typedef struct List Devices;

// ========================================= config
enum SampleFormat { F32 = 0, I16, U16, I32 };

typedef struct StreamConfig {
  uint16_t channels;
  uint32_t sample_rate;
  uint32_t buffer_size;  // 0 for default
  enum SampleFormat sample_format;
} StreamConfig;

typedef struct StreamConfigRange {
  uint16_t channels;
  uint32_t min_sample_rate;
  uint32_t max_sample_rate;
  uint32_t min_buffer_size;
  uint32_t max_buffer_size;
  enum SampleFormat sample_format;
} StreamConfigRange;

typedef struct List StreamConfigRanges;

#endif
