#ifndef _CSYNTH_PLATFORM_H
#define _CSYNTH_PLATFORM_H

#include "core.h"

// ====================================== device

// devices

#define devices_foreach(devices, fn)                    \
  assert(devices.list != NULL);                         \
  ListNode* node = devices.list->head;                  \
  list_foreach(node) {                                  \
    struct Device* device = (struct Device*)node->data; \
    fn(device);                                         \
  }

void get_all_devices(struct Devices*);
void free_devices(struct Devices);

// device
struct Device* get_default_output_device();
void free_device(struct Device*);
const char* get_device_name(struct Device*);

// ====================================== configs
void get_supported_output_configs(struct Device*, StreamConfigRanges*);
// default_output_config

// ====================================== stream
// build_output_stream
// pause
// play

#endif
