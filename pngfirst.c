/*
 * pngfirst.c
 * 
 * Compile with -lpng
 */

#include <setjmp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Include <stdio.h> and <stddef.h> before png.h! */
#include "png.h"

#define IMAGE_WIDTH 320
#define IMAGE_HEIGHT 240
#define PIXEL_BYTES 3   /* bytes per pixel */

int main(int argc, char *argv[]) {
  
  int status = 1;
  FILE *fp = NULL;
  int png_init = 0;
  int i = 0;
  unsigned char *pRow = NULL;
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  
  /* Ignore arguments */
  (void) argc;
  (void) argv;
  
  /* Open output file */
  fp = fopen("first_out.png", "wb");
  if (fp == NULL) {
    fprintf(stderr, "Can't open output file!\n");
    status = 0;
  }
  
  /* Allocate scanline buffer and clear it */
  if (status) {
    pRow = (unsigned char *) malloc(IMAGE_WIDTH * PIXEL_BYTES);
    if (pRow == NULL) {
      abort();
    }
    memset(pRow, 0, IMAGE_WIDTH * PIXEL_BYTES);
  }
  
  /* Write a pattern into the row */
  if (status) {
    for(i = 0; i < IMAGE_WIDTH; i++) {
      pRow[(i * PIXEL_BYTES)] = (unsigned char) (i % 256);
      pRow[(i * PIXEL_BYTES) + 1] = (unsigned char) (i % 256);
      pRow[(i * PIXEL_BYTES) + 2] = (unsigned char) (i % 256);
    }
  }
  
  /* Initialize PNG library */
  if (status) {
    png_ptr = png_create_write_struct(
                PNG_LIBPNG_VER_STRING,
                NULL, NULL, NULL);  /* Default error handling */
    if (png_ptr == NULL) {
      fprintf(stderr, "Can't initialize PNG structure!\n");
      abort();
    }
    
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
      fprintf(stderr, "Can't initialize PNG info structure!\n");
      abort();
    }
    
    png_init = 1;
  }
  
  /* For PNG library errors, put a setjmp handler here that prints a
   * message, clears status, and then proceeds through rest of procedure
   * with error status */
  if (status) {
    if (setjmp(png_jmpbuf(png_ptr))) {
      /* Careful -- local variables may be in uncertain state? */
      fprintf(stderr, "PNG library error!\n");
      status = 0;
      png_init = 1;   /* Make sure this variable is set properly */
    }
  }
  
  /* Initialize PNG I/O */
  if (status) {
    png_init_io(png_ptr, fp);
  }
  
  /* Initialize PNG info structure */
  if (status) {
    png_set_IHDR(png_ptr, info_ptr,
        IMAGE_WIDTH, IMAGE_HEIGHT,  /* Width and height */
        8,                          /* Bits per channel */
        PNG_COLOR_TYPE_RGB,   /* Other choices: RGBA, Gray, Gray+A */
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);
  }
  
  /* Write the PNG headers */
  if (status) {
    png_write_info(png_ptr, info_ptr);
  }
  
  /* Write each scanline */
  if (status) {
    for(i = 0; i < IMAGE_HEIGHT; i++) {
      png_write_row(png_ptr, (png_bytep) pRow);
    }
  }
  
  /* Finish writing the PNG file */
  if (status) {
    png_write_end(png_ptr, info_ptr);
  }
  
  /* Close file if open */
  if (fp != NULL) {
    fclose(fp);
    fp = NULL;
  }
  
  /* Free PNG structures if initialized */
  if (png_init) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    png_ptr = NULL;
    info_ptr = NULL;
    png_init = 0;
  }
  
  /* Free scanline buffer if allocated */
  if (pRow != NULL) {
    free(pRow);
    pRow = NULL;
  }
  
  /* Invert status and return */
  if (status) {
    status = 0;
  } else {
    status = 1;
  }
  return status;
}
