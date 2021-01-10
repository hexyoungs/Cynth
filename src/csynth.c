#include "csynth.h"

#include <stdio.h>

#include "platform.h"

static void print_device_name(struct Device *device) {
  const char *name = get_device_name(device);
  if (name != NULL) {
    printf("found device: %s\n", name);
  }
}

static void print_all_devices() {
  struct Devices *devices = get_all_devices();
  iter_devices(devices, print_device_name);
  free_devices(devices);
}

int main() {
  print_all_devices();
  return 0;
}
