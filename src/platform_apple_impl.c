#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/syslog.h>

#include "platform.h"

#if DEBUG

#define DebugMsg(inFormat, ...) syslog(LOG_NOTICE, inFormat, ##__VA_ARGS__)

#define FailIf(inCondition, inHandler, inMessage) \
  if (inCondition) {                              \
    DebugMsg(inMessage);                          \
    goto inHandler;                               \
  }

#define FailWithAction(inCondition, inAction, inHandler, inMessage) \
  if (inCondition) {                                                \
    DebugMsg(inMessage);                                            \
    { inAction; }                                                   \
    goto inHandler;                                                 \
  }

#else

#define DebugMsg(inFormat, ...)

#define FailIf(inCondition, inHandler, inMessage) \
  if (inCondition) {                              \
    goto inHandler;                               \
  }

#define FailWithAction(inCondition, inAction, inHandler, inMessage) \
  if (inCondition) {                                                \
    { inAction; }                                                   \
    goto inHandler;                                                 \
  }

#endif

typedef struct Device {
  AudioDeviceID id;
} Device;

void get_all_devices(struct Devices* devices) {
  AudioObjectPropertyAddress addr;
  addr.mSelector = kAudioHardwarePropertyDevices;
  addr.mScope = kAudioObjectPropertyScopeGlobal;
  addr.mElement = kAudioObjectPropertyElementMaster;

  uint32_t data_size = 0;
  OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject,
                                                   &addr, 0, NULL, &data_size);
  FailIf(status != kAudioHardwareNoError, error,
         "get_devices: get devices failed");

  int device_count = data_size / sizeof(AudioDeviceID);
  AudioDeviceID* device_ids = malloc(data_size);

  status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL,
                                      &data_size, device_ids);
  FailIf(status != kAudioHardwareNoError, error,
         "get_devices: get device ids failed");

  List* list = list_new();
  for (int i = 0; i < device_count; i++) {
    Device* device = (Device*)malloc(sizeof(Device));
    device->id = device_ids[i];
    list_add(list, (void*)device);
  }

  free(device_ids);
  devices->list = list;
  return;

error:
  if (device_ids) free(device_ids);
  return;
}

void free_devices(struct Devices devices) {
  if (devices.list == NULL) return;
  ListNode* node = devices.list->head;
  list_foreach(node) {
    Device* device = (Device*)node->data;
    free(device);
  }
  list_free(devices.list);
  devices.list = NULL;
}

// device

struct Device* get_default_output_device() {
  AudioObjectPropertyAddress addr;
  addr.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
  addr.mScope = kAudioObjectPropertyScopeGlobal;
  addr.mElement = kAudioObjectPropertyElementMaster;

  AudioDeviceID id = 0;
  UInt32 data_size = sizeof(AudioDeviceID);
  OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr,
                                               0, NULL, &data_size, &id);
  FailIf(status != kAudioHardwareNoError, error,
         "get_default_output_device: get default output device failed");

  Device* device = malloc(sizeof(Device));
  device->id = id;
  return device;

error:
  return NULL;
}

void free_device(struct Device* device) {
  if (device != NULL) {
    free(device);
    device = NULL;
  }
}

const char* get_device_name(struct Device* device) {
  AudioObjectPropertyAddress addr;
  addr.mSelector = kAudioDevicePropertyDeviceNameCFString;
  addr.mScope = kAudioObjectPropertyScopeOutput;
  addr.mElement = kAudioObjectPropertyElementMaster;

  CFStringRef name;
  UInt32 data_size = sizeof(CFStringRef);

  OSStatus status =
      AudioObjectGetPropertyData(device->id, &addr, 0, NULL, &data_size, &name);
  FailIf(status != 0, error, "get_device_name: failed");

  return CFStringGetCStringPtr(name, kCFStringEncodingUTF8);
error:
  return NULL;
}
