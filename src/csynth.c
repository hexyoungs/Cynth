#include <stdio.h>

#include "platform.h"

static print_device_name(struct Device* device) {
  const char* name = get_device_name(device);
  if (name != NULL) {
    printf("found device: %s\n", name);
  }
}

int main() {
  struct Devices devices;
  get_all_devices(&devices);
  devices_foreach(devices, print_device_name);
  free_devices(devices);
  return 0;
}
