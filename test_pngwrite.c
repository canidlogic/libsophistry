/*
 * test_pngwrite.c
 * 
 * Test program that writes a PNG file.
 */
#include "sophistry.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  
  SPH_IMAGE_WRITER *pw = NULL;
  uint32_t *ps = NULL;
  int x = 0;
  SPH_ARGB argb;
  
  /* Initialize structure */
  memset(&argb, 0, sizeof(SPH_ARGB));
  
  /* Allocate writer */
  pw = sph_image_writer_newFromPath(
    "test.png", 256, 256, SPH_IMAGE_DOWN_NONE, -1);
  if (pw == NULL) {
    fprintf(stderr, "Can't open writer!\n");
    return 1;
  }
  
  /* Write scanline as blue fading out */
  ps = sph_image_writer_ptr(pw);
  for(x = 0; x < 256; x++) {
    argb.a = 255 - x;
    argb.r = 0;
    argb.g = 0;
    argb.b = 255;
    ps[x] = sph_argb_pack(&argb);
  }
  
  /* Write all scanlines */
  for(x = 0; x < 256; x++) {
    sph_image_writer_write(pw);
  }
  
  /* Close writer */
  sph_image_writer_close(pw);
  
  return 0;
}
