#ifndef _CSYNTH_PLATFORM
#define _CSYNTH_PLATFORM

// ====================================== device
struct Devices;
struct Device;
typedef void (*iter_devices_cb)(struct Device*);

// devices
struct Devices* get_all_devices();
void free_devices(struct Devices*);
void iter_devices(struct Devices*, iter_devices_cb);

// device
struct Device* get_default_output_device();
void free_device(struct Device*);
const char* get_device_name(struct Device*);

// ====================================== stream
// TODO

#endif
