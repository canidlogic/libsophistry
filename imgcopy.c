/*
 * imgcopy.c
 * 
 * See the README file for further information.
 */
#include "sophistry.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * @@TODO:
 */
static int imgcopy(
    const char * pOutPath,
    const char * pInPath,
           int   q,
           int   dconv,
           int * pError) {
  
  int status = 1;
  int32_t h = 0;
  int32_t y = 0;
  int32_t w = 0;
  int erri = 0;
  uint32_t *ps = NULL;
  SPH_IMAGE_READER *pr = NULL;
  SPH_IMAGE_WRITER *pw = NULL;
  
  /* Check parameters */
  if ((pOutPath == NULL) || (pInPath == NULL) ||
      (q < -1) || (q > 100)) {
    abort();
  }
  if ((dconv != SPH_IMAGE_DOWN_NONE) &&
      (dconv != SPH_IMAGE_DOWN_RGB) &&
      (dconv != SPH_IMAGE_DOWN_GRAY)) {
    abort();
  }
  
  /* Allocate reader */
  pr = sph_image_reader_newFromPath(pInPath, &erri);
  if (pr == NULL) {
    if (pError != NULL) {
      *pError = erri;
    }
    status = 0;
  }
  
  /* Allocate writer */
  if (status) {
    pw = sph_image_writer_newFromPath(
        pOutPath,
        sph_image_reader_width(pr),
        sph_image_reader_height(pr),
        dconv,
        q);
    if (pw == NULL) {
      if (pError != NULL) {
        *pError = -1;
      }
      status = 0;
    }
  }
  
  /* Transfer each row */
  if (status) {
    w = sph_image_reader_width(pr);
    h = sph_image_reader_height(pr);
    for(y = 0; y < h; y++) {
      ps = sph_image_reader_read(pr, &erri);
      if (erri) {
        if (pError != NULL) {
          *pError = -1;
        }
        status = 0;
        break;
      }
      
      memcpy(
        sph_image_writer_ptr(pw),
        ps,
        ((size_t) w) * sizeof(uint32_t));
      sph_image_writer_write(pw);
    }
  }
  
  /* Close objects if open */
  sph_image_writer_close(pw);
  sph_image_reader_close(pr);
  
  /* Return status */
  return status;
}

/*
 * Program entrypoint.
 */
int main(int argc, char *argv[]) {
  
  int status = 0;
  int errcode = 0;
  
  status = imgcopy(
    "test_copy.png", "test.png", -1, SPH_IMAGE_DOWN_GRAY, &errcode);
  
  /* @@TODO: */
  return 0;
}
