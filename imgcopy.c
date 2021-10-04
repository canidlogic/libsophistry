/*
 * imgcopy.c
 */
#include "sophistry.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  
  int status = 1;
  int32_t h = 0;
  int32_t y = 0;
  int32_t w = 0;
  SPH_IMAGE_READER *pr = NULL;
  SPH_IMAGE_WRITER *pw = NULL;
  
  /* Allocate reader */
  pr = sph_image_reader_newFromPath("test.png", NULL);
  if (pr == NULL) {
    fprintf(stderr, "Can't open reader!\n");
    status = 0;
  }
  
  /* Allocate writer */
  if (status) {
    pw = sph_image_writer_newFromPath(
        "test_copy.png",
        sph_image_reader_width(pr),
        sph_image_reader_height(pr),
        SPH_IMAGE_DOWN_NONE,
        -1);
    if (pw == NULL) {
      fprintf(stderr, "Can't open writer!\n");
      status = 0;
    }
  }
  
  /* Transfer each row */
  if (status) {
    w = sph_image_reader_width(pr);
    h = sph_image_reader_height(pr);
    for(y = 0; y < h; y++) {
      /* @@TODO: error check */
      memcpy(
        sph_image_writer_ptr(pw),
        sph_image_reader_read(pr, NULL),
        ((size_t) w) * sizeof(uint32_t));
      sph_image_writer_write(pw);
    }
  }
  
  /* Close objects if open */
  sph_image_writer_close(pw);
  sph_image_reader_close(pr);
  
  /* Invert status and return */
  if (status) {
    status = 0;
  } else {
    status = 1;
  }
  return status;
}
