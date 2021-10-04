/*
 * sophistry.c
 * 
 * Implementation of sophistry.h
 */
#include "sophistry.h"
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

/* Include <stdio.h> and <stddef.h> before png.h! */
#include "png.h"

/*
 * SPH_IMAGE_WRITER structure.
 * 
 * Prototype given in header.
 */
struct SPH_IMAGE_WRITER_TAG {
  
  /*
   * The output file handle.
   * 
   * This will be closed when the object is released.
   */
  FILE *pOut;
  
  /*
   * The kind of image being written.
   * 
   * This must be one of the SPH_IMAGE_TYPE constants.
   * 
   * ================================================
   * Currently, only SPH_IMAGE_TYPE_PNG is supported!
   * ================================================
   */
  int ftype;
  
  /*
   * The PNG library codec pointer.
   * 
   * Only valid if ftype is SPH_IMAGE_TYPE_PNG.
   * 
   * Remember to set the longjmp location in each function before using
   * this!
   * 
   * This will be closed when the object is released.
   */
  png_structp png_ptr;
  
  /*
   * The PNG library info pointer.
   * 
   * Only valid if ftype is SPH_IMAGE_TYPE_PNG.
   * 
   * This will be closed when the object is released.
   */
  png_infop info_ptr;
  
  /*
   * The width of the image in pixels.
   */
  int32_t w;
  
  /*
   * The height of the image in pixels.
   */
  int32_t h;
  
  /*
   * The number of scanlines that have been written so far.
   */
  int32_t scan_count;
  
  /*
   * The down-conversion requested.
   * 
   * This must be one of the SPH_IMAGE_DOWN constants.
   */
  int dconv;
  
  /*
   * Pointer to the scanline buffer.
   * 
   * This dynamically allocated buffer is freed when the object is 
   * freed.
   */
  uint32_t *pScan;
  
  /*
   * Pointer to the binary I/O buffer.
   * 
   * This dynamically allocated buffer is freed when the object is
   * freed.
   */
  uint8_t *pData;
};

/*
 * SPH_IMAGE_READER structure.
 * 
 * Prototype given in header.
 */
struct SPH_IMAGE_READER_TAG {
  
  /*
   * Error flag.
   * 
   * If non-zero, a read error was encountered.
   */
  int err_flag;
  
  /*
   * The input file handle.
   * 
   * This will be closed when the object is released.
   */
  FILE *pIn;
  
  /*
   * The kind of image being read.
   * 
   * This must be one of the SPH_IMAGE_TYPE constants.
   * 
   * ================================================
   * Currently, only SPH_IMAGE_TYPE_PNG is supported!
   * ================================================
   */
  int ftype;
  
  /*
   * The PNG library codec pointer.
   * 
   * Only valid if ftype is SPH_IMAGE_TYPE_PNG.
   * 
   * Remember to set the longjmp location in each function before using
   * this!
   * 
   * This will be closed when the object is released.
   */
  png_structp png_ptr;
  
  /*
   * The PNG library info pointer.
   * 
   * Only valid if ftype is SPH_IMAGE_TYPE_PNG.
   * 
   * This will be closed when the object is released.
   */
  png_infop info_ptr;
  
  /*
   * The width of the image in pixels.
   */
  int32_t w;
  
  /*
   * The height of the image in pixels.
   */
  int32_t h;
  
  /*
   * The number of scanlines that have been read so far.
   */
  int32_t scan_count;
  
  /*
   * The number of channels in the input image.
   * 
   * The valid values are 1-4.  One means grayscale, two means grayscale
   * plus alpha, three means RGB, four means RGBA.
   */
  int ccount;
  
  /*
   * Pointer to the scanline buffer.
   * 
   * This dynamically allocated buffer is freed when the object is 
   * freed.
   */
  uint32_t *pScan;
  
  /*
   * Pointer to the binary I/O buffer.
   * 
   * This dynamically allocated buffer is freed when the object is
   * freed.
   */
  uint8_t *pData;
};

/*
 * Local functions
 * ===============
 */

/* Function prototypes */
static void sph_png_rowRGBA(
    const uint32_t * pScan,
          uint8_t  * pData,
          int32_t    w);
static void sph_png_rowRGB(
    const uint32_t * pScan,
          uint8_t  * pData,
          int32_t    w);
static void sph_png_rowGray(
    const uint32_t * pScan,
          uint8_t  * pData,
          int32_t    w);

static void sph_png_decodeRow(
    const uint8_t  * pData,
          uint32_t * pScan,
          int        ccount,
          int32_t    w);

static int sph_path_getImageType(const char *pPath);

/*
 * Convert a scanline row to the PNG RGBA format.
 * 
 * pScan points to the scanline to convert.  On input it holds packed
 * ARGB color values.  Its length is w pixels.  w must be in range
 * [1, SPH_IMAGE_MAXDIM].
 * 
 * RGBA binary values that can be passed to the PNG codec will be
 * written to pData.  The length of this buffer must be (w * 4) bytes.
 * 
 * Parameters:
 * 
 *   pScan - pointer to the scanline buffer
 * 
 *   pData - pointer to the data buffer
 * 
 *   w - width of the scanline buffer in pixels
 */
static void sph_png_rowRGBA(
    const uint32_t * pScan,
          uint8_t  * pData,
          int32_t    w) {
  
  SPH_ARGB argb;
  
  /* Clear structure */
  memset(&argb, 0, sizeof(SPH_ARGB));
  
  /* Check parameters */
  if ((pScan == NULL) || (pData == NULL)) {
    abort();
  }
  if ((w < 1) || (w > SPH_IMAGE_MAXDIM)) {
    abort();
  }
  
  /* Convert scanline */
  for( ; w > 0; w--) {
    sph_argb_unpack(*pScan, &argb);
    pData[0] = (unsigned char) argb.r;
    pData[1] = (unsigned char) argb.g;
    pData[2] = (unsigned char) argb.b;
    pData[3] = (unsigned char) argb.a;
    pData += 4;
    pScan++;
  }
}

/*
 * Convert a scanline row to the PNG RGB format.
 * 
 * pScan points to the scanline to convert.  On input it holds packed
 * ARGB color values.  Its length is w pixels.  w must be in range
 * [1, SPH_IMAGE_MAXDIM].
 * 
 * RGB binary values that can be passed to the PNG codec will be written
 * to pData.  The length of this buffer must be (w * 3) bytes.
 * 
 * Parameters:
 * 
 *   pScan - pointer to the scanline buffer
 * 
 *   pData - pointer to the data buffer
 * 
 *   w - width of the scanline buffer in pixels
 */
static void sph_png_rowRGB(
    const uint32_t * pScan,
          uint8_t  * pData,
          int32_t    w) {
  
  SPH_ARGB argb;
  
  /* Clear structure */
  memset(&argb, 0, sizeof(SPH_ARGB));
  
  /* Check parameters */
  if ((pScan == NULL) || (pData == NULL)) {
    abort();
  }
  if ((w < 1) || (w > SPH_IMAGE_MAXDIM)) {
    abort();
  }
  
  /* Convert scanline */
  for( ; w > 0; w--) {
    sph_argb_unpack(*pScan, &argb);
    sph_argb_downRGB(&argb);
    pData[0] = (unsigned char) argb.r;
    pData[1] = (unsigned char) argb.g;
    pData[2] = (unsigned char) argb.b;
    pData += 3;
    pScan++;
  }
}

/*
 * Convert a scanline row to the PNG grayscale format.
 * 
 * pScan points to the scanline to convert.  On input it holds packed
 * ARGB color values.  Its length is w pixels.  w must be in range
 * [1, SPH_IMAGE_MAXDIM].
 * 
 * Grayscale binary values that can be passed to the PNG codec will be
 * written to pData.  The length of this buffer must be (w) bytes.
 * 
 * Parameters:
 * 
 *   pScan - pointer to the scanline buffer
 * 
 *   pData - pointer to the data buffer
 * 
 *   w - width of the scanline buffer in pixels
 */
static void sph_png_rowGray(
    const uint32_t * pScan,
          uint8_t  * pData,
          int32_t    w) {
  
  SPH_ARGB argb;
  
  /* Clear structure */
  memset(&argb, 0, sizeof(SPH_ARGB));
  
  /* Check parameters */
  if ((pScan == NULL) || (pData == NULL)) {
    abort();
  }
  if ((w < 1) || (w > SPH_IMAGE_MAXDIM)) {
    abort();
  }
  
  /* Convert scanline */
  for( ; w > 0; w--) {
    sph_argb_unpack(*pScan, &argb);
    sph_argb_downGray(&argb);
    *pData = (unsigned char) argb.b;
    pData++;
    pScan++;
  }
}

/*
 * Given binary scanline data from a PNG file, decode it into a scanline
 * buffer.
 * 
 * ccount is the number of color channels, which must be in range
 * [1, 4].  One channel is grayscale, two is grayscale plus alpha, three
 * is RGB, four is RGB plus alpha.  w is the width in pixels.  w must be
 * in range [1, SPH_IMAGE_MAXDIM].
 * 
 * pData points to the bytes to decode.  Its length is (w * ccount)
 * bytes.
 * 
 * Packed ARGB pixels will be written to pScan.
 * 
 * Parameters:
 * 
 *   pData - pointer to the bytes to decode
 * 
 *   pScan - pointer to the output scanline
 * 
 *   ccount - the number of color channels
 * 
 *   w - the width of the scanline in pixels
 */
static void sph_png_decodeRow(
    const uint8_t  * pData,
          uint32_t * pScan,
          int        ccount,
          int32_t    w) {
  
  SPH_ARGB argb;
  
  /* Clear structure */
  memset(&argb, 0, sizeof(SPH_ARGB));
  
  /* Check parameters */
  if ((pScan == NULL) || (pData == NULL)) {
    abort();
  }
  if ((w < 1) || (w > SPH_IMAGE_MAXDIM)) {
    abort();
  }
  if ((ccount < 1) || (ccount > 4)) {
    abort();
  }
  
  /* Decode scanline */
  for( ; w > 0; w--) {
    
    /* Decode input */
    if (ccount == 1) {
      /* Grayscale */
      argb.a = 255;
      argb.r = *pData;
      argb.g = argb.r;
      argb.b = argb.r;
      
    } else if (ccount == 2) {
      /* Grayscale plus alpha */
      argb.a = pData[1];
      argb.r = pData[0];
      argb.g = argb.r;
      argb.b = argb.r;
      
    } else if (ccount == 3) {
      /* RGB */
      argb.a = 255;
      argb.r = pData[0];
      argb.g = pData[1];
      argb.b = pData[2];
      
    } else if (ccount == 4) {
      /* RGBA */
      argb.a = pData[3];
      argb.r = pData[0];
      argb.g = pData[1];
      argb.b = pData[2];
      
    } else {
      abort();  /* shouldn't happen */
    }
    
    /* Write packed result to output */
    *pScan = sph_argb_pack(&argb);
    
    /* Advance pointers */
    pData += ccount;
    pScan++;
  }
}

/*
 * Given a file path for an image, determine from the file extension
 * which image type is meant.
 * 
 * If the end of the string is a case-insensitive match for one of the
 * following:
 * 
 *   .PNG
 *   .JPEG
 *   .JPG
 * 
 * Then this function returns the appropriate SPH_IMAGE_TYPE constant.
 * Otherwise, this function returns -1 to indicate that the file type
 * could not be determined from the path.
 * 
 * Parameters:
 * 
 *   pPath - the path to check
 * 
 * Return:
 * 
 *   the image type, or -1
 */
static int sph_path_getImageType(const char *pPath) {
  
  size_t slen = 0;
  int extlen = 0;
  uint32_t extcode = 0;
  int c = 0;
  int i = 0;
  int result = 0;
  
  /* Check parameter */
  if (pPath == NULL) {
    abort();
  }
  
  /* Get the size of the string, not including the terminating NUL */
  slen = strlen(pPath);
  
  /* If the size of the string is at least four, check whether the
   * fourth from last character is an ASCII dot; if it is, set extlen to
   * three to indicate a three-character file extension */
  if (slen >= 4) {
    if (pPath[slen - 4] == 0x2e) {
      extlen = 3;
    }
  }
  
  /* If the extension length hasn't been set yet and the size of the
   * string is at least five, check whether the fifth from last
   * character is an ASCII dot; if it is, set extlen to four to indicate
   * a four-character file extension */
  if ((extlen == 0) && (slen >= 5)) {
    if (pPath[slen - 5] == 0x2e) {
      extlen = 4;
    }
  }
  
  /* If extlen is non-zero, form the uppercase extension code; else,
   * leave extension code set to zero */
  for(i = extlen; i >= 1; i--) {
    c = pPath[slen - i];
    if ((c >= 0x61) && (c <= 0x7a)) {
      /* Lowercase letter -- make uppercase */
      c = c - 0x20;
    }
    extcode = (extcode << 8) | ((uint32_t) c);
  }
  
  /* Look up extension code */
  if (extcode == 0x504e47) {
    result = SPH_IMAGE_TYPE_PNG;
  
  } else if ((extcode == 0x4a5047) || (extcode == 0x4a504547)) {
    result = SPH_IMAGE_TYPE_JPEG;
  
  } else {
    /* Unrecognized */
    result = -1;
  }
  
  /* Return result */
  return result;
}

/*
 * Public function implementations
 * ===============================
 * 
 * See header for specifications.
 */

/*
 * sph_argb_pack function.
 */
uint32_t sph_argb_pack(const SPH_ARGB *pc) {
  
  uint32_t a = 0;
  uint32_t r = 0;
  uint32_t g = 0;
  uint32_t b = 0;
  
  /* Check parameter */
  if (pc == NULL) {
    abort();
  }
  
  /* Get each channel, clamping the ranges */
  if (pc->a < 0) {
    a = (uint32_t) 0;
  } else if (pc->a > 255) {
    a = (uint32_t) 255;
  } else {
    a = (uint32_t) pc->a;
  }
  
  if (pc->r < 0) {
    r = (uint32_t) 0;
  } else if (pc->r > 255) {
    r = (uint32_t) 255;
  } else {
    r = (uint32_t) pc->r;
  }
  
  if (pc->g < 0) {
    g = (uint32_t) 0;
  } else if (pc->g > 255) {
    g = (uint32_t) 255;
  } else {
    g = (uint32_t) pc->g;
  }
  
  if (pc->b < 0) {
    b = (uint32_t) 0;
  } else if (pc->b > 255) {
    b = (uint32_t) 255;
  } else {
    b = (uint32_t) pc->b;
  }
  
  /* Merge into result */
  return (uint32_t) (
    (a << 24) |
    (r << 16) |
    (g <<  8) |
     b);
}

/*
 * sph_argb_unpack function.
 */
void sph_argb_unpack(uint32_t c, SPH_ARGB *pc) {
  
  /* Check parameters */
  if (pc == NULL) {
    abort();
  }
  
  /* Clear structure */
  memset(pc, 0, sizeof(SPH_ARGB));
  
  /* Get each channel */
  pc->a = (uint32_t) ((c >> 24) & 0xff);
  pc->r = (uint32_t) ((c >> 16) & 0xff);
  pc->g = (uint32_t) ((c >>  8) & 0xff);
  pc->b = (uint32_t) ( c        & 0xff);
}

/*
 * sph_argb_downRGB function.
 */
void sph_argb_downRGB(SPH_ARGB *pc) {
  
  /* Check parameter */
  if (pc == NULL) {
    abort();
  }
  
  /* Clamp each channel */
  if (pc->a < 0) {
    pc->a = 0;
  } else if (pc->a > 255) {
    pc->a = 255;
  }
  
  if (pc->r < 0) {
    pc->r = 0;
  } else if (pc->r > 255) {
    pc->r = 255;
  }
  
  if (pc->g < 0) {
    pc->g = 0;
  } else if (pc->g > 255) {
    pc->g = 255;
  }
  
  if (pc->b < 0) {
    pc->b = 0;
  } else if (pc->b > 255) {
    pc->b = 255;
  }
  
  /* If the alpha channel is zero, replace color with opaque white */
  if (pc->a < 1) {
    pc->a = 255;
    pc->r = 255;
    pc->g = 255;
    pc->b = 255;
  }
  
  /* If the alpha channel is in range [1, 244] then approximate a mix of
   * the color against a white background */
  if ((pc->a > 0) && (pc->a < 255)) {
    
    /* Compute channels */
    pc->r = 255 + ((pc->a * (pc->r - 255)) / 255);
    pc->g = 255 + ((pc->a * (pc->g - 255)) / 255);
    pc->b = 255 + ((pc->a * (pc->b - 255)) / 255);
    
    /* Clamp channels and set alpha to opaque */
    pc->a = 255;
    
    if (pc->r < 0) {
      pc->r = 0;
    } else if (pc->r > 255) {
      pc->r = 255;
    }
    
    if (pc->g < 0) {
      pc->g = 0;
    } else if (pc->g > 255) {
      pc->g = 255;
    }
    
    if (pc->b < 0) {
      pc->b = 0;
    } else if (pc->b > 255) {
      pc->b = 255;
    }
  }
}

/*
 * sph_argb_downGray function.
 */
void sph_argb_downGray(SPH_ARGB *pc) {
  
  int32_t gray = 0;
  
  /* Check parameter */
  if (pc == NULL) {
    abort();
  }
  
  /* Down-convert to RGB first */
  sph_argb_downRGB(pc);
  
  /* Adjust results if RGB channels not equal */
  if ((pc->r != pc->g) || (pc->r != pc->b)) {
    
    /* Compute grayscale value */
    gray = (2126 * ((int32_t) pc->r) +
            7152 * ((int32_t) pc->g) +
             722 * ((int32_t) pc->b)) / 10000;
    
    /* Clamp grayscale value */
    if (gray < 0) {
      gray = 0;
    } else if (gray > 255) {
      gray = 255;
    }
    
    /* Replace RGB channels with grayscale value */
    pc->r = (int) gray;
    pc->g = (int) gray;
    pc->b = (int) gray;
  }
}

/*
 * sph_image_writer_new function.
 */
SPH_IMAGE_WRITER *sph_image_writer_new(
    FILE    * pOut,
    int       ftype,
    int32_t   w,
    int32_t   h,
    int       dconv,
    int       q) {
  
  SPH_IMAGE_WRITER *pw = NULL;
  
  /* Check parameters */
  if (pOut == NULL) {
    abort();
  }
  if ((ftype != SPH_IMAGE_TYPE_PNG) && (ftype != SPH_IMAGE_TYPE_JPEG)) {
    abort();
  }
  if ((w < 1) || (w > SPH_IMAGE_MAXDIM)) {
    abort();
  }
  if ((h < 1) || (h > SPH_IMAGE_MAXDIM)) {
    abort();
  }
  if ((dconv != SPH_IMAGE_DOWN_NONE) &&
      (dconv != SPH_IMAGE_DOWN_RGB) &&
      (dconv != SPH_IMAGE_DOWN_GRAY)) {
    abort();
  }
  
  /* Set quality to 90 if it has the -1 value */
  if (q == -1) {
    q = 90;
  }
  
  /* Clamp quality */
  if (q < 0) {
    q = 0;
  } else if (q > 100) {
    q = 100;
  }
  
  /* If JPEG, we must have RGB or grayscale down-conversion */
  if (ftype == SPH_IMAGE_TYPE_JPEG) {
    if ((dconv != SPH_IMAGE_DOWN_RGB) &&
        (dconv != SPH_IMAGE_DOWN_GRAY)) {
      abort();
    }
  }
  
  /* Allocate image writer structure */
  pw = (SPH_IMAGE_WRITER *) malloc(sizeof(SPH_IMAGE_WRITER));
  if (pw == NULL) {
    abort();
  }
  memset(pw, 0, sizeof(SPH_IMAGE_WRITER));
  
  /* Allocate scanline buffer */
  pw->pScan = (uint32_t *) malloc(((size_t) w) * sizeof(uint32_t));
  if (pw->pScan == NULL) {
    abort();
  }
  memset(pw->pScan, 0, ((size_t) w) * sizeof(uint32_t));
  
  /* Allocate data buffer */
  if (dconv == SPH_IMAGE_DOWN_NONE) {
    /* RGBA */
    pw->pData = (uint8_t *) malloc(((size_t) w) * ((size_t) 4));
    if (pw->pData == NULL) {
      abort();
    }
    memset(pw->pData, 0, ((size_t) w) * ((size_t) 4));
  
  } else if (dconv == SPH_IMAGE_DOWN_RGB) {
    /* RGB */
    pw->pData = (uint8_t *) malloc(((size_t) w) * ((size_t) 3));
    if (pw->pData == NULL) {
      abort();
    }
    memset(pw->pData, 0, ((size_t) w) * ((size_t) 3));
  
  } else if (dconv == SPH_IMAGE_DOWN_GRAY) {
    /* Grayscale */
    pw->pData = (uint8_t *) malloc((size_t) w);
    if (pw->pData == NULL) {
      abort();
    }
    memset(pw->pData, 0, (size_t) w);
  
  } else {
    /* Unrecognized down-conversion */
    abort();
  }
  
  /* Initialize all general fields */
  pw->pOut = pOut;
  pw->ftype = ftype;
  pw->w = w;
  pw->h = h;
  pw->scan_count = 0;
  pw->dconv = dconv;
  
  /* Initialize specific codec */
  if (ftype == SPH_IMAGE_TYPE_PNG) {
    /* Initialize PNG codec */
    pw->png_ptr = png_create_write_struct(
                PNG_LIBPNG_VER_STRING,
                NULL, NULL, NULL);  /* Default error handling */
    if (pw->png_ptr == NULL) {
      /* Error initializing PNG codec */
      abort();
    }
    
    pw->info_ptr = png_create_info_struct(pw->png_ptr);
    if (pw->info_ptr == NULL) {
      /* Error creating information structure */
      abort();
    }
  
    /* Establish error handler for PNG */
    if (setjmp(png_jmpbuf(pw->png_ptr))) {
      /* Careful -- local variables may be in uncertain state? */
      abort();
    }
  
    /* Initialize PNG I/O */
    png_init_io(pw->png_ptr, pw->pOut);
    
    /* Initialize writing information */
    if (pw->dconv == SPH_IMAGE_DOWN_NONE) {
      /* No down-conversion, so full ARGB */
      png_set_IHDR(pw->png_ptr, pw->info_ptr,
          pw->w, pw->h,   /* Width and height */
          8,              /* Bits per channel */
          PNG_COLOR_TYPE_RGB_ALPHA,
          PNG_INTERLACE_NONE,
          PNG_COMPRESSION_TYPE_DEFAULT,
          PNG_FILTER_TYPE_DEFAULT);
    
    } else if (pw->dconv == SPH_IMAGE_DOWN_RGB) {
      /* RGB down-conversion */
      png_set_IHDR(pw->png_ptr, pw->info_ptr,
          pw->w, pw->h,   /* Width and height */
          8,              /* Bits per channel */
          PNG_COLOR_TYPE_RGB,
          PNG_INTERLACE_NONE,
          PNG_COMPRESSION_TYPE_DEFAULT,
          PNG_FILTER_TYPE_DEFAULT);
    
    } else if (pw->dconv == SPH_IMAGE_DOWN_GRAY) {
      /* Grayscale down-conversion */
      png_set_IHDR(pw->png_ptr, pw->info_ptr,
          pw->w, pw->h,   /* Width and height */
          8,              /* Bits per channel */
          PNG_COLOR_TYPE_GRAY,
          PNG_INTERLACE_NONE,
          PNG_COMPRESSION_TYPE_DEFAULT,
          PNG_FILTER_TYPE_DEFAULT);
    
    } else {
      /* Unrecognized down-conversion */
      abort();
    }
    
    /* Write PNG headers to output */
    png_write_info(pw->png_ptr, pw->info_ptr);
  
  } else if (ftype == SPH_IMAGE_TYPE_JPEG) {
    /* @@FIXME: JPEG files not supported at the moment */
    abort();
  
  } else {
    /* Unrecognized image file type */
    abort();
  }
  
  /* Return writer object */
  return pw;
}

/*
 * sph_image_writer_newFromPath function.
 */
SPH_IMAGE_WRITER *sph_image_writer_newFromPath(
    const char    * pPath,
          int32_t   w,
          int32_t   h,
          int       dconv,
          int       q) {
  
  int ftype = 0;
  int status = 1;
  FILE *pOut = NULL;
  SPH_IMAGE_WRITER *pw = NULL;
  
  /* Check path parameter */
  if (pPath == NULL) {
    abort();
  }
  
  /* Determine type from path */
  ftype = sph_path_getImageType(pPath);
  
  /* Fail if type could not be determined */
  if (ftype == -1) {
    status = 0;
  }
  
  /* Open the output file */
  if (status) {
    pOut = fopen(pPath, "wb");
    if (pOut == NULL) {
      status = 0;
    }
  }
  
  /* If JPEG type and dconv is NONE, change to RGB */
  if (status) {
    if ((ftype == SPH_IMAGE_TYPE_JPEG) &&
        (dconv == SPH_IMAGE_DOWN_NONE)) {
      dconv = SPH_IMAGE_DOWN_RGB;
    }
  }
  
  /* Call through if no error */
  if (status) {
    pw = sph_image_writer_new(pOut, ftype, w, h, dconv, q);
  }
  
  /* Return writer or NULL */
  return pw;
}

/*
 * sph_image_writer_close function.
 */
void sph_image_writer_close(SPH_IMAGE_WRITER *pw) {
  
  /* Only proceed if non-NULL parameter */
  if (pw != NULL) {
  
    /* Shut down codecs */
    if (pw->ftype == SPH_IMAGE_TYPE_PNG) {
  
      /* Establish error handler for PNG */
      if (setjmp(png_jmpbuf(pw->png_ptr))) {
        /* Careful -- local variables may be in uncertain state? */
        abort();
      }
  
      /* Free PNG structures */
      png_destroy_write_struct(&(pw->png_ptr), &(pw->info_ptr));
    
    } else if (pw->ftype == SPH_IMAGE_TYPE_JPEG) {
      /* @@FIXME: JPEG support */
    
    } else {
      /* Unrecognized image type */
      abort();
    }
  
    /* Close file */
    fclose(pw->pOut);
    
    /* Free scanline buffer and data buffer */
    free(pw->pScan);
    free(pw->pData);
    
    /* Free structure */
    free(pw);
  }
}

/*
 * sph_image_writer_ptr function.
 */
uint32_t *sph_image_writer_ptr(SPH_IMAGE_WRITER *pw) {
  
  /* Check parameter */
  if (pw == NULL) {
    abort();
  }
  
  /* Return pointer */
  return pw->pScan;
}

/*
 * sph_image_writer_write function.
 */
void sph_image_writer_write(SPH_IMAGE_WRITER *pw) {
  
  /* Check parameter */
  if (pw == NULL) {
    abort();
  }
  
  /* Check that not all scanlines have been written */
  if (pw->scan_count >= pw->h) {
    abort();
  }
  
  /* Handle based on image type */
  if (pw->ftype == SPH_IMAGE_TYPE_PNG) {
  
    /* PNG -- first of all, register error handler */
    if (setjmp(png_jmpbuf(pw->png_ptr))) {
      /* Careful -- local variables may be in uncertain state? */
      abort();
    }
  
    /* Serialize into bytes */
    if (pw->dconv == SPH_IMAGE_DOWN_NONE) {
      /* No down-conversion, so full RGBA */
      sph_png_rowRGBA(pw->pScan, pw->pData, pw->w);
      
    } else if (pw->dconv == SPH_IMAGE_DOWN_RGB) {
      /* RGB down-conversion */
      sph_png_rowRGB(pw->pScan, pw->pData, pw->w);
      
    } else if (pw->dconv == SPH_IMAGE_DOWN_GRAY) {
      /* Grayscale down-conversion */
      sph_png_rowGray(pw->pScan, pw->pData, pw->w);
      
    } else {
      /* Unrecognized down-conversion setting */
      abort();
    }
  
    /* Write the serialized scanline */
    png_write_row(
        pw->png_ptr,
        (png_bytep) pw->pData);
        
    /* Increase the scanline count */
    (pw->scan_count)++;
    
    /* If we just wrote the last scanline, finish writing */
    if (pw->scan_count >= pw->h) {
      png_write_end(pw->png_ptr, pw->info_ptr);
    }
    
  } else if (pw->ftype == SPH_IMAGE_TYPE_JPEG) {
    /* @@FIXME: add JPEG support */
    abort();
  
  } else {
    /* Unrecognized image type */
    abort();
  }
}

/*
 * sph_image_reader_new function.
 */
SPH_IMAGE_READER *sph_image_reader_new(
    FILE * pIn,
    int    ftype,
    int  * pError) {

  SPH_IMAGE_READER *pr = NULL;
  int status = 1;
  
  uint32_t w = 0;
  uint32_t h = 0;
  int bdepth = 0;
  int ctype = 0;
  int imethod = 0;
  int ccount = 0;
  int alpha_flag = 0;
  
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;

  /* Check parameters */
  if (pIn == NULL) {
    abort();
  }
  if ((ftype != SPH_IMAGE_TYPE_PNG) && (ftype != SPH_IMAGE_TYPE_JPEG)) {
    abort();
  }
  
  /* Set error to unknown in case we need to leave from a longjmp */
  if (pError != NULL) {
    *pError = SPH_IMAGE_RERR_UNKNOWN;
  }
  
  /* Read header information and initialize codecs */
  if (ftype == SPH_IMAGE_TYPE_PNG) {
    
    /* Initialize PNG codec */
    png_ptr = png_create_read_struct(
                PNG_LIBPNG_VER_STRING,
                NULL, NULL, NULL);  /* Default error handling */
    if (png_ptr == NULL) {
      /* Error initializing PNG codec */
      abort();
    }
    
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
      /* Error creating information structure */
      abort();
    }
  
    /* Establish error handler for PNG */
    if (setjmp(png_jmpbuf(png_ptr))) {
      /* Careful -- local variables may be in uncertain state? */
      status = 0;
    }
  
    /* Initialize PNG I/O */
    if (status) {
      png_init_io(png_ptr, pIn);
    }
    
    /* Read the headers of the input file */
    if (status) {
      png_read_info(png_ptr, info_ptr);
    }
    
    /* Get information about the input file format */
    if (status) {
      png_get_IHDR(png_ptr, info_ptr,
        &w, &h,
        &bdepth,
        &ctype,
        &imethod,
        NULL, NULL);
    }
    
    /* Make sure dimensions are not too large */
    if (status) {
      if ((w > SPH_IMAGE_MAXDIM) || (h > SPH_IMAGE_MAXDIM)) {
        if (pError != NULL) {
          *pError = SPH_IMAGE_RERR_IMAGEDIM;
        }
        status = 0;
      }
    }
    
    /* Make sure input file is not interlaced */
    if (status) {
      if (imethod != PNG_INTERLACE_NONE) {
        if (pError != NULL) {
          *pError = SPH_IMAGE_RERR_INTERLACED;
        }
        status = 0;
      }
    }
    
    /* Make sure input file is not 16-bit */
    if (status) {
      if (bdepth > 8) {
        if (pError != NULL) {
          *pError = SPH_IMAGE_RERR_BITDEPTH;
        }
        status = 0;
      }
    }
    
    /* Request expansions specific to color spaces */
    if (status && (ctype == PNG_COLOR_TYPE_PALETTE)) {
      /* Palette image -- expand to RGB */
      png_set_palette_to_rgb(png_ptr);
      
    } else if (status && ((ctype == PNG_COLOR_TYPE_GRAY) ||
                          (ctype == PNG_COLOR_TYPE_GRAY_ALPHA))) {
      /* Grayscale -- expand if less than 8-bit */
      if (bdepth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
      }
    
    } else if (status && ((ctype == PNG_COLOR_TYPE_RGB) ||
                          (ctype == PNG_COLOR_TYPE_RGB_ALPHA))) {
      /* RGB -- expand if less than 8-bit */
      if (bdepth < 8) {
        png_set_expand(png_ptr);
      }
      
    } else if (status) {
      /* Unrecognized color space -- shouldn't happen */
      abort();
    }
    
    /* If there is a transparency chunk and color type doesn't already
     * have alpha channel, add alpha channel and set alpha flag */
    if (status) {
      if ((ctype != PNG_COLOR_TYPE_GRAY_ALPHA) &&
          (ctype != PNG_COLOR_TYPE_RGB_ALPHA)) {
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
          alpha_flag = 1;
          png_set_tRNS_to_alpha(png_ptr);
        }
      }
    }
    
    /* Determine the number of color channels */
    if (ctype == PNG_COLOR_TYPE_PALETTE) {
      /* Palette image has 3 or 4 channels, depending on alpha flag */
      if (alpha_flag) {
        ccount = 4;
      } else {
        ccount = 3;
      }
    
    } else if (ctype == PNG_COLOR_TYPE_GRAY) {
      /* Grayscale image has 1 or 2 channels, depending on alpha flag */
      if (alpha_flag) {
        ccount = 2;
      } else {
        ccount = 1;
      }
    
    } else if (ctype == PNG_COLOR_TYPE_RGB) {
      /* RGB image has 3 or 4 channels, depending on alpha flag */
      if (alpha_flag) {
        ccount = 4;
      } else {
        ccount = 3;
      }
    
    } else if (ctype == PNG_COLOR_TYPE_GRAY_ALPHA) {
      /* Grayscale with alpha has 2 channels */
      ccount = 2;
    
    } else if (ctype == PNG_COLOR_TYPE_RGB_ALPHA) {
      /* RGB with alpha has 4 channels */
      ccount = 4;
    
    } else {
      /* Unrecognized color type -- shouldn't happen */
      abort();
    }
    
    /* Update reading info */
    if (status) {
      png_read_update_info(png_ptr, info_ptr);
    }
    
    /* If there was any problem, free the PNG codec */
    if (!status) {
      png_destroy_read_struct(
        &png_ptr, &info_ptr, (png_infopp)NULL);
      png_ptr = NULL;
      info_ptr = NULL;
    }
  
  } else if (ftype == SPH_IMAGE_TYPE_JPEG) {
    /* @@FIXME: JPEG files not supported at the moment */
    abort();
  
  } else {
    /* Unrecognized image file type */
    abort();
  }
  
  /* Allocate image reader structure */
  if (status) {
    pr = (SPH_IMAGE_READER *) malloc(sizeof(SPH_IMAGE_READER));
    if (pr == NULL) {
      abort();
    }
    memset(pr, 0, sizeof(SPH_IMAGE_READER));
  }

  /* Initialize all general fields, including transferring the input
   * file into the structure */
  if (status) {
    pr->err_flag = 0;
    pr->pIn = pIn;
    pr->ftype = ftype;
    pr->w = w;
    pr->h = h;
    pr->scan_count = 0;
    pr->ccount = ccount;
    
    pIn = NULL;
  }
  
  /* Transfer codec into object */
  if (status && (ftype == SPH_IMAGE_TYPE_PNG)) {
    /* PNG codec */
    pr->png_ptr = png_ptr;
    pr->info_ptr = info_ptr;
    
    png_ptr = NULL;
    info_ptr = NULL;
    
  } else if (status && (ftype == SPH_IMAGE_TYPE_JPEG)) {
    /* @@FIXME: JPEG support */
    abort();
    
  } else if (status) {
    /* Unrecognized image type */
    abort();
  }
  
  /* Allocate scanline buffer */
  if (status) {
    pr->pScan = (uint32_t *) malloc(((size_t) w) * sizeof(uint32_t));
    if (pr->pScan == NULL) {
      abort();
    }
    memset(pr->pScan, 0, ((size_t) w) * sizeof(uint32_t));
  }
  
  /* Allocate data buffer */
  if (status) {
    pr->pData = (uint8_t *) malloc(((size_t) w) * ((size_t) ccount));
    if (pr->pData == NULL) {
      abort();
    }
    memset(pr->pData, 0, ((size_t) w) * ((size_t) ccount));
  }
  
  /* If failure, close the file */
  if (!status) {
    fclose(pIn);
  }
  
  /* If successful, set error to NONE */
  if (status) {
    if (pError != NULL) {
      *pError = SPH_IMAGE_RERR_NONE;
    }
  }
  
  /* Return reader object or NULL */
  return pr;
}

/*
 * sph_image_reader_newFromPath function.
 */
SPH_IMAGE_READER *sph_image_reader_newFromPath(
    const char * pPath,
          int  * pError) {
  
  int ftype = 0;
  int status = 1;
  FILE *pIn = NULL;
  SPH_IMAGE_READER *pr = NULL;
  
  /* Check path parameter */
  if (pPath == NULL) {
    abort();
  }
  
  /* Determine type from path */
  ftype = sph_path_getImageType(pPath);
  
  /* Fail if type could not be determined */
  if (ftype == -1) {
    if (pError != NULL) {
      *pError = SPH_IMAGE_RERR_FILETYPE;
    }
    status = 0;
  }
  
  /* Open the input file */
  if (status) {
    pIn = fopen(pPath, "rb");
    if (pIn == NULL) {
      if (pError != NULL) {
        *pError = SPH_IMAGE_RERR_UNKNOWN;
      }
      status = 0;
    }
  }
  
  /* Call through if no error */
  if (status) {
    pr = sph_image_reader_new(pIn, ftype, pError);
  }
  
  /* Return reader or NULL */
  return pr;
}

/*
 * sph_image_reader_close function.
 */
void sph_image_reader_close(SPH_IMAGE_READER *pr) {
  
  /* Only proceed if non-NULL parameter */
  if (pr != NULL) {
  
    /* Shut down codecs */
    if (pr->ftype == SPH_IMAGE_TYPE_PNG) {
  
      /* Establish error handler for PNG */
      if (setjmp(png_jmpbuf(pr->png_ptr))) {
        /* Careful -- local variables may be in uncertain state? */
        abort();
      }
  
      /* Free PNG structures */
      png_destroy_read_struct(
          &(pr->png_ptr),
          &(pr->info_ptr),
          (png_infopp)NULL);
    
    } else if (pr->ftype == SPH_IMAGE_TYPE_JPEG) {
      /* @@FIXME: JPEG support */
    
    } else {
      /* Unrecognized image type */
      abort();
    }
  
    /* Close file */
    fclose(pr->pIn);
    
    /* Free scanline buffer and data buffer */
    free(pr->pScan);
    free(pr->pData);
    
    /* Free structure */
    free(pr);
  }
}

/*
 * sph_image_reader_width function.
 */
int32_t sph_image_reader_width(SPH_IMAGE_READER *pr) {
  
  /* Check parameter */
  if (pr == NULL) {
    abort();
  }
  
  /* Return requested value */
  return pr->w;
}

/*
 * sph_image_reader_height function.
 */
int32_t sph_image_reader_height(SPH_IMAGE_READER *pr) {
  
  /* Check parameter */
  if (pr == NULL) {
    abort();
  }
  
  /* Return requested value */
  return pr->h;
}

/*
 * sph_image_reader_read function.
 */
uint32_t *sph_image_reader_read(SPH_IMAGE_READER *pr, int *pReadErr) {
  
  int status = 1;
  
  /* Check parameter */
  if (pr == NULL) {
    abort();
  }
  
  /* Only proceed if not in error mode */
  if (!(pr->err_flag)) {
  
    /* Check that not all scanlines have been read */
    if (pr->scan_count >= pr->h) {
      abort();
    }
  
    /* Handle based on image type */
    if (pr->ftype == SPH_IMAGE_TYPE_PNG) {
  
      /* PNG -- first of all, register error handler */
      if (setjmp(png_jmpbuf(pr->png_ptr))) {
        /* Careful -- local variables may be in uncertain state? */
        status = 0;
      }
  
      /* Read the scanline bytes */
      if (status) {
        png_read_row(
            pr->png_ptr,
            (png_bytep) pr->pData,
            NULL);
      }
  
      /* Decode the bytes */
      if (status) {
        sph_png_decodeRow(pr->pData, pr->pScan, pr->ccount, pr->w);
      }
        
      /* Increase the scanline count */
      if (status) {
        (pr->scan_count)++;
      }
    
      /* If we just read the last scanline, finish reading */
      if (status) {
        if (pr->scan_count >= pr->h) {
          png_read_end(pr->png_ptr, (png_infop)NULL);
        }
      }
      
      /* If error, set error flag if it was passed, set internal error
       * mode, and clear the buffer */
      if (!status) {
        if (pReadErr != NULL) {
          *pReadErr = 1;
        }
        memset(pr->pScan, 0, ((size_t) pr->w) * sizeof(uint32_t));
        pr->err_flag = 1;
      }
    
    } else if (pr->ftype == SPH_IMAGE_TYPE_JPEG) {
      /* @@FIXME: add JPEG support */
      abort();
  
    } else {
      /* Unrecognized image type */
      abort();
    }
  
  } else {
    /* In error mode -- set error flag if it was passed and clear the
     * buffer */
    if (pReadErr != NULL) {
      *pReadErr = 1;
    }
    memset(pr->pScan, 0, ((size_t) pr->w) * sizeof(uint32_t));
  }
  
  /* Always return pointer to scanline buffer */
  return pr->pScan;
}
