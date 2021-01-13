#include "csynth.h"

#include <stdio.h>

#include "core.h"

static void _print_device_name(struct Device* device) {
  const char* name = get_device_name(device);
  if (name != NULL) {
    printf("- %s\n", name);
  }
}

static void print_devices() {
  Devices* devices = get_all_devices();
  printf("Detected devices: \n");
  list_foreach(devices, _print_device_name);
  free_devices(devices);
  printf("\n");
}

static void _print_config(StreamConfigRange* config) {
  printf("channels: %d\n", config->channels);
  printf("sample format: %d\n", config->sample_format);
  printf("sample rate: (%d, %d)\n", config->min_sample_rate,
         config->max_sample_rate);
  printf("buffer size: (%d, %d)\n", config->min_buffer_size,
         config->max_buffer_size);
  printf("\n");
}

static void print_device_supported_configs(struct Device* device) {
  printf("Current device: \n");
  _print_device_name(device);
  printf("\nsupporting configs: \n\n");
  StreamConfigRanges* stream_configs = get_supported_output_configs(device);
  if (stream_configs != NULL) {
    list_foreach(stream_configs, _print_config);
  }
  free_configs(stream_configs);
}

int main() {
  print_devices();
  struct Device* device = get_default_output_device();
  // print_device_supported_configs(device);
  StreamConfig config = get_default_output_config(device);
  printf("channels: %d\n", config.channels);
  printf("sample format: %d\n", config.sample_format);
  printf("sample rate: %d\n", config.sample_rate);
  printf("buffer size: %d\n", config.buffer_size);
  printf("\n");

  // get configs
  // set callbacks
  // listen keyboard events
  // device.start()
  free_device(device);
  return 0;
}
