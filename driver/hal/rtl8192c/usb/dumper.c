/**
 * gcc dumper.c -I ../../../include/ -DPLATFORM_LINUX=1
 */

#include <stdint.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

#include "Hal8192CUHWImg.c"

#include <assert.h>
#include <stdio.h>

void save(const uint8_t *const array, const size_t size,
          const char *const filename) {
  FILE *out = fopen(filename, "w");
  assert(out);
  const size_t written = fwrite(array, 1, size, out);
  assert(written == size);
  fclose(out);
}

int main() {
  save(Rtl8192CUFwTSMCImgArray, sizeof(Rtl8192CUFwTSMCImgArray),
       "Rtl8192CUFwTSMCImgArray_v88.bin");
  save(Rtl8192CUFwUMCACutImgArray, sizeof(Rtl8192CUFwUMCACutImgArray),
       "Rtl8192CUFwUMCACutImgArray_v88.bin");
  save(Rtl8192CUFwUMCBCutImgArray, sizeof(Rtl8192CUFwUMCBCutImgArray),
       "Rtl8192CUFwUMCBCutImgArray_v88.bin");
}
