/*
 * pngcopy.c
 * 
 * See the README file for further information.
 */
#include "sophistry.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Perform the image copy operation.
 * 
 * pOutPath and pInPath specify the output and input image paths,
 * respectively.
 * 
 * dconv is the down-conversion to use.  It must be one of the constants
 * SPH_IMAGE_DOWN defined by Sophistry.
 * 
 * pError is optionally a pointer to an integer that receives an error
 * code.  On error, this will be set to one of the SPH_IMAGE_ERR codes.
 * On success, this will be set to zero (SPH_IMAGE_ERR_NONE).
 * 
 * The input image is read by Sophistry and a copy is written to the
 * output image using Sophistry.  This can be used to strip unnecessary
 * metadata and apply down-conversion.  The image will be completely
 * re-encoded.
 * 
 * Parameters:
 * 
 *   pOutPath - the output image file path
 * 
 *   pInPath - the input image file path
 *  
 *   dconv - the down-conversion setting
 * 
 *   pError - pointer to the error code return, or NULL
 */
static int pngcopy(
    const char * pOutPath,
    const char * pInPath,
           int   dconv,
           int * pError) {
  
  int status = 1;
  int32_t h = 0;
  int32_t y = 0;
  int32_t w = 0;
  uint32_t *ps = NULL;
  SPH_IMAGE_READER *pr = NULL;
  SPH_IMAGE_WRITER *pw = NULL;

  /* Check parameters */
  if ((pOutPath == NULL) || (pInPath == NULL)) {
    abort();
  }
  if ((dconv != SPH_IMAGE_DOWN_NONE) &&
      (dconv != SPH_IMAGE_DOWN_RGB) &&
      (dconv != SPH_IMAGE_DOWN_GRAY)) {
    abort();
  }
  
  /* Clear error code if provided */
  if (pError != NULL) {
    *pError = SPH_IMAGE_ERR_NONE;
  }
  
  /* Allocate reader */
  pr = sph_image_reader_newFromPath(pInPath, pError);
  if (pr == NULL) {
    status = 0;
  }
  
  /* Allocate writer */
  if (status) {
    pw = sph_image_writer_newFromPath(
        pOutPath,
        sph_image_reader_width(pr),
        sph_image_reader_height(pr),
        dconv,
        0,
        pError);
    if (pw == NULL) {
      status = 0;
    }
  }
  
  /* Transfer each row */
  if (status) {
    /* Get width and height */
    w = sph_image_reader_width(pr);
    h = sph_image_reader_height(pr);
    
    /* Go row by row */
    for(y = 0; y < h; y++) {
      /* Read a row */
      ps = sph_image_reader_read(pr, pError);
      if (ps == NULL) {
        status = 0;
        break;
      }
      
      /* Transfer scanline to output */
      memcpy(
        sph_image_writer_ptr(pw),
        ps,
        ((size_t) w) * sizeof(uint32_t));
      
      /* Write the scanline */
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
  
  int status = 1;
  int errcode = 0;
  int x = 0;
  int dconv = 0;
  
  const char *pModuleName = NULL;
  
  /* Determine the module name */
  if (argc >= 1) {
    if (argv != NULL) {
      pModuleName = argv[0];
    }
  }
  if (pModuleName == NULL) {
    pModuleName = "pngcopy";
  }
  
  /* We must have 2-3 parameters (plus the module name) */
  if ((argc < 3) || (argc > 4)) {
    fprintf(stderr, "%s: Unexpected number of parameters!\n",
      pModuleName);
    status = 0;
  }
  
  /* Verify all parameters exist */
  if (status) {
    if (argv == NULL) {
      abort();
    }
    for(x = 0; x < argc; x++) {
      if (argv[x] == NULL) {
        abort();
      }
    }
  }
  
  /* If 3rd parameter exists, determine the down-conversion type; else,
   * set it to NONE */
  if (status && (argc >= 4)) {
    /* Parse 3rd parameter */
    if (strcmp(argv[3], "rgb") == 0) {
      dconv = SPH_IMAGE_DOWN_RGB;
    
    } else if (strcmp(argv[3], "gray") == 0) {
      dconv = SPH_IMAGE_DOWN_GRAY;
      
    } else {
      fprintf(stderr, "%s: Unrecognized down-conversion type!\n",
        pModuleName);
      status = 0;
    }
    
  } else if (status) {
    /* No 3rd parameter */
    dconv = SPH_IMAGE_DOWN_NONE;
  }
  
  /* Call through to program function */
  if (status) {
    if (!pngcopy(argv[1], argv[2], dconv, &errcode)) {
      fprintf(stderr, "%s: %s!\n", 
        pModuleName,
        sph_image_errorString(errcode));
      status = 0;
    }
  }
  
  /* Invert status and return */
  if (status) {
    status = 0;
  } else {
    status = 1;
  }
  return status;
}
