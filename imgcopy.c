/*
 * imgcopy.c
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
 * respectively.  They should have file extensions that Sophistry
 * recognizes as valid, or else an error occurs.
 * 
 * q is the compression quality.  It must be in range [0, 100], or it
 * must be the special value -1 to indicate a default of 90.  Zero means
 * highest compression but lowest image quality, while 100 means lowest
 * compression but highest image quality.  q is only relevant for JPEG
 * files.
 * 
 * dconv is the down-conversion to use.  It must be one of the constants
 * SPH_IMAGE_DOWN defined by Sophistry.  If SPH_IMAGE_DOWN_NONE is
 * specified but an output JPEG file is indicated by the file path
 * extension, RGB down-conversion will automatically be selected.
 * 
 * pError is optionally a pointer to an integer that receives an error
 * code.  On error, this will be set to one of the SPH_IMAGE_ERR codes.
 * On success, this will be set to zero (SPH_IMAGE_ERR_NONE).
 * 
 * The input image is read by Sophistry and a copy is written to the
 * output image using Sophistry.  This can be used to convert between
 * JPEG and PNG files.  It can also be used to strip unnecessary
 * metadata.  The image will be completely re-encoded.
 * 
 * Parameters:
 * 
 *   pOutPath - the output image file path
 * 
 *   pInPath - the input image file path
 * 
 *   q - the compression quality
 * 
 *   dconv - the down-conversion setting
 * 
 *   pError - pointer to the error code return, or NULL
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
        q,
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
 * Parse the given string as an unsigned decimal integer.
 * 
 * Whitespace is not allowed.  If there is an error, -1 is returned.
 * 
 * Parameters:
 * 
 *   pStr - the string to parse
 * 
 * Return:
 * 
 *   the parsed integer, or -1 if error
 */
static int parseInt(const char *pStr) {
  
  int result = 0;
  int c = 0;
  
  /* Check parameter */
  if (pStr == NULL) {
    abort();
  }
  
  /* Fail if empty string */
  if (*pStr == 0) {
    result = -1;
  }
  
  /* Parse each character */
  for( ; *pStr != 0; pStr++) {
    
    /* Get current character */
    c = *pStr;
    
    /* Convert current character to decimal digit, or -1 if not a
     * decimal digit */
    if (c == '0') {
      c = 0;
    } else if (c == '1') {
      c = 1;
    } else if (c == '2') {
      c = 2;
    } else if (c == '3') {
      c = 3;
    } else if (c == '4') {
      c = 4;
    } else if (c == '5') {
      c = 5;
    } else if (c == '6') {
      c = 6;
    } else if (c == '7') {
      c = 7;
    } else if (c == '8') {
      c = 8;
    } else if (c == '9') {
      c = 9;
    } else {
      c = -1;
    }
    
    /* If current character not a decimal digit, fail */
    if (c == -1) {
      result = -1;
    }
    
    /* If still okay, multiply result by 10, watching for overflow */
    if (result != -1) {
      if (result <= INT_MAX / 10) {
        result = result * 10;
      } else {
        result = -1;
      }
    }
    
    /* If still okay, add new digit in, watching for overflow */
    if (result != -1) {
      if (result <= INT_MAX - c) {
        result = result + c;
      }
    }
    
    /* Break out of loop if failure */
    if (result == -1) {
      break;
    }
  }
  
  /* Return result or error */
  return result;
}

/*
 * Program entrypoint.
 */
int main(int argc, char *argv[]) {
  
  int status = 1;
  int errcode = 0;
  int x = 0;
  int q = 0;
  int dconv = 0;
  
  const char *pModuleName = NULL;
  
  /* Determine the module name */
  if (argc >= 1) {
    if (argv != NULL) {
      pModuleName = argv[0];
    }
  }
  if (pModuleName == NULL) {
    pModuleName = "imgcopy";
  }
  
  /* We must have 2-4 parameters (plus the module name) */
  if ((argc < 3) || (argc > 5)) {
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
  
  /* If 3rd parameter exists, parse it; else set q to -1 */
  if (status && (argc >= 4)) {
    /* Parse 3rd parameter */
    q = parseInt(argv[3]);
    
    /* Check for parsing error */
    if (q == -1) {
      fprintf(stderr, "%s: Can't parse quality as integer!\n",
        pModuleName);
      status = 0;
    }
    
    /* Check range */
    if (status) {
      if ((q < 0) || (q > 100)) {
        fprintf(stderr, "%s: Quality must be in range [0, 100]!\n",
          pModuleName);
        status = 0;
      }
    }
    
  } else if (status) {
    /* No 3rd parameter */
    q = -1;
  }
  
  /* If 4th parameter exists, determine the down-conversion type; else,
   * set it to NONE */
  if (status && (argc >= 5)) {
    /* Parse 4th parameter */
    if (strcmp(argv[4], "rgb") == 0) {
      dconv = SPH_IMAGE_DOWN_RGB;
    
    } else if (strcmp(argv[4], "gray") == 0) {
      dconv = SPH_IMAGE_DOWN_GRAY;
      
    } else {
      fprintf(stderr, "%s: Unrecognized down-conversion type!\n",
        pModuleName);
      status = 0;
    }
    
  } else if (status) {
    /* No 4th parameter */
    dconv = SPH_IMAGE_DOWN_NONE;
  }
  
  /* Call through to program function */
  if (status) {
    if (!imgcopy(argv[1], argv[2], q, dconv, &errcode)) {
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
