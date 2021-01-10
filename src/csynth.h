#ifndef _CSYNTH
#define _CSYNTH

#include <inttypes.h>

#define HZ_44100 44100  // cd

typedef struct StreamConfig {
  uint16_t channels;
  uint32_t sample_rate;
  uint32_t buffer_size;
} StreamConfig;

#endif
