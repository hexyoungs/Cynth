#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/syslog.h>

#include "core.h"
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

Devices* get_all_devices() {
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

  Devices* devices = (Devices*)malloc(sizeof(Devices));
  devices->head = NULL;
  for (int i = 0; i < device_count; i++) {
    Device* device = (Device*)malloc(sizeof(Device));
    device->id = device_ids[i];

    list_add(devices, (void*)device);
  }

  free(device_ids);
  return devices;

error:
  if (device_ids) free(device_ids);
  return NULL;
}

void free_devices(Devices* devices) {
  ListNode* node = devices->head;
  list_nodes_foreach(node) {
    Device* device = (Device*)node->data;
    free(device);
    node->data = NULL;
  }
  list_free(devices);
  devices = NULL;
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

// configs

StreamConfigRanges* get_supported_output_configs(struct Device* device) {
  AudioObjectPropertyAddress addr;
  addr.mSelector = kAudioDevicePropertyStreamConfiguration;
  addr.mScope = kAudioObjectPropertyScopeOutput;
  addr.mElement = kAudioObjectPropertyElementMaster;

  UInt32 data_size = 0;
  OSStatus status =
      AudioObjectGetPropertyDataSize(device->id, &addr, 0, NULL, &data_size);
  FailIf(status != 0, error, "get_supported_output_configs: failed");

  uint8_t* buffer_list_data = malloc(data_size);
  status = AudioObjectGetPropertyData(device->id, &addr, 0, NULL, &data_size,
                                      buffer_list_data);
  FailIf(status != 0, error, "get_supported_output_configs: failed");

  AudioBufferList* audio_buffer_list = (AudioBufferList*)buffer_list_data;
  if (audio_buffer_list->mNumberBuffers == 0) goto error;

  UInt32 n_buffers = audio_buffer_list->mNumberBuffers;
  UInt32 n_channels = 0;
  for (int i = 0; i < n_buffers; i++) {
    AudioBuffer* buffer = &audio_buffer_list->mBuffers[i];
    n_channels += buffer->mNumberChannels;
  }
  // macOS should support U8, I16, I32, F32 and F64. This should allow for
  // using I16 but just use F32 for now as its the default anyway.
  enum SampleFormat sample_format = F32;

  addr.mSelector = kAudioDevicePropertyAvailableNominalSampleRates;
  data_size = 0;
  status =
      AudioObjectGetPropertyDataSize(device->id, &addr, 0, NULL, &data_size);
  FailIf(status != 0, error, "get_supported_output_configs: failed");

  UInt32 n_ranges = data_size / sizeof(AudioValueRange);
  uint8_t* range_data = malloc(data_size);
  status = AudioObjectGetPropertyData(device->id, &addr, 0, NULL, &data_size,
                                      range_data);
  FailIf(status != 0, error, "get_supported_output_configs: failed");
  AudioValueRange* audio_value_range = (AudioValueRange*)range_data;

  AudioComponentDescription desc;
  desc.componentSubType = kAudioUnitSubType_HALOutput;
  desc.componentType = kAudioUnitType_Output;
  desc.componentManufacturer = kAudioUnitManufacturer_Apple;
  desc.componentFlags = 0;
  desc.componentFlagsMask = 0;

  AudioComponent component = AudioComponentFindNext(NULL, &desc);
  FailIf(component == NULL, error, "No matching AudioComponent found");

  AudioUnit audio_unit;
  status = AudioComponentInstanceNew(component, &audio_unit);
  FailIf(status != 0, error, "Failed to create AudioUnit");
  status = AudioUnitInitialize(audio_unit);
  FailIf(status != 0, error, "Failed to init AudioUnit");
  status = AudioUnitSetProperty(
      audio_unit, kAudioOutputUnitProperty_CurrentDevice,
      kAudioUnitScope_Global, 0, (void*)&device->id, sizeof(AudioDeviceID));
  FailIf(status != 0, error, "Failed to setup AudioUnit");

  AudioValueRange buffer_size_range;
  UInt32 buffer_size_range_size = sizeof(AudioValueRange);
  status = AudioUnitGetProperty(
      audio_unit, kAudioDevicePropertyBufferFrameSizeRange,
      kAudioUnitScope_Global, 0, &buffer_size_range, &buffer_size_range_size);
  FailIf(status != 0, error, "Failed to get buffer range");

  StreamConfigRanges* stream_configs =
      (StreamConfigRanges*)malloc(sizeof(StreamConfigRanges));
  stream_configs->head = NULL;
  for (int i = 0; i < n_ranges; i++) {
    StreamConfigRange* config_range = malloc(sizeof(StreamConfigRange));
    config_range->min_sample_rate = audio_value_range[i].mMinimum;
    config_range->max_sample_rate = audio_value_range[i].mMaximum;
    config_range->min_buffer_size = buffer_size_range.mMinimum;
    config_range->max_buffer_size = buffer_size_range.mMaximum;
    config_range->channels = n_channels;
    config_range->sample_format = sample_format;

    list_add(stream_configs, (void*)config_range);
  }

  free_to_null(buffer_list_data);
  free_to_null(range_data);
  return stream_configs;

error:
  free_to_null(buffer_list_data);
  free_to_null(range_data);
  return NULL;
}

void free_configs(StreamConfigRanges* configs) {
  ListNode* node = configs->head;
  list_nodes_foreach(node) {
    StreamConfigRange* config = (StreamConfigRange*)node->data;
    free_to_null(config);
    node->data = NULL;
  }
  list_free(configs);
  configs = NULL;
}
