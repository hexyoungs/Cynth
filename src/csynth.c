#include "csynth.h"

#include <stdio.h>

static void _print_device_name(struct Device* device) {
  const char* name = get_device_name(device);
  if (name != NULL) {
    printf("- %s\n", name);
  }
}

static void print_devices() {
  Devices* devices = get_all_devices();
  printf("devices: \n");
  devices_foreach(devices, _print_device_name);
  free_devices(devices);
}

int main() {
  print_devices();
  struct Device* device = get_default_output_device();
  free_device(device);
  return 0;
}
