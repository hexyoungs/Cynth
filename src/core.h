#ifndef _CSYNTH_CORE_H
#define _CSYNTH_CORE_H

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>

#define HZ_44100 44100  // cd

// ========================================== utils

typedef struct ListNode {
  void* data;
  struct ListNode* next;
} ListNode;

typedef struct List {
  ListNode* head;
} List;

// ========================================== helper macros

#define list_foreach(node) for (; node != NULL; node = node->next)

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

static inline void list_free(List* list) {
  if (list == NULL) return;
  ListNode* node = list->head;
  list_foreach(node) {
    // free(node);
  }
  free(list);
}

// ========================================= csynth
typedef struct StreamConfig {
  uint16_t channels;
  uint32_t sample_rate;
  uint32_t buffer_size;
} StreamConfig;

enum SampleFormat { F32, I16, U16 };

typedef struct StreamConfigRange {
  uint16_t channels;
  uint32_t min_sample_rate;
  uint32_t max_sample_rate;
  uint32_t buffer_size;
  enum SampleFormat sample_format;
} StreamConfigRange;

typedef struct StreamConfigRanges {
  struct Device* device;
  List* list;
} StreamConfigRanges;

// ========================================== platform
struct Device;
typedef struct Devices {
  List* list;
} Devices;

#endif
