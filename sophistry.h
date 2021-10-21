#ifndef SOPHISTRY_H_INCLUDED
#define SOPHISTRY_H_INCLUDED

/*
 * sophistry.h
 * 
 * See the Sophistry manual in the doc directory for further
 * information.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Structure prototypes */
struct SPH_IMAGE_WRITER_TAG;
typedef struct SPH_IMAGE_WRITER_TAG SPH_IMAGE_WRITER;

struct SPH_IMAGE_READER_TAG;
typedef struct SPH_IMAGE_READER_TAG SPH_IMAGE_READER;

/* Maximum value for width and height dimensions of an image */
#define SPH_IMAGE_MAXDIM (1000000)

/* Image file type definitions */
#define SPH_IMAGE_TYPE_PNG  (1)   /* PNG file */

/* Image down-conversion types */
#define SPH_IMAGE_DOWN_NONE (0)   /* No down-conversion */
#define SPH_IMAGE_DOWN_RGB  (1)   /* RGB down-conversion */
#define SPH_IMAGE_DOWN_GRAY (2)   /* Grayscale down-conversion */

/* Image errors */
#define SPH_IMAGE_ERR_UNKNOWN   (-1) /* Unknown error */
#define SPH_IMAGE_ERR_NONE       (0) /* No error */
#define SPH_IMAGE_ERR_INTERLACED (1) /* Interlaced or progressive */
#define SPH_IMAGE_ERR_BITDEPTH   (2) /* Bit depth too high */
#define SPH_IMAGE_ERR_IMAGEDIM   (3) /* Dimensions too large */
#define SPH_IMAGE_ERR_FILETYPE   (4) /* Can't determine file type */
#define SPH_IMAGE_ERR_OPEN       (5) /* Can't open file */
#define SPH_IMAGE_ERR_READDATA   (6) /* Error reading data */

/*
 * A structure holding a parsed ARGB color.
 */
typedef struct {
  
  /*
   * The alpha channel value.
   * 
   * The alpha channel is assumed to be non-premultiplied with respect
   * to the RGB channels.
   * 
   * Valid range is [0, 255].
   */
  int a;
  
  /*
   * The red channel value.
   * 
   * Valid range is [0, 255].
   */
  int r;
  
  /*
   * The green channel value.
   * 
   * Valid range is [0, 255].
   */
  int g;
  
  /*
   * The blue channel value.
   * 
   * Valid range is [0, 255].
   */
  int b;
  
} SPH_ARGB;

/*
 * Given a parsed ARGB color, pack it into an unsigned 32-bit integer.
 * 
 * The eight most significant bits of the returned integer will be the
 * alpha channel value, then eight bits for red, then eight for green,
 * and finally the eight least significant bits are the blue channel.
 * 
 * If any channel value in the structure is out of range, it will be
 * clamped to the range [0, 255] before being packed.
 * 
 * Parameters:
 * 
 *   pc - pointer to the parsed ARGB color
 * 
 * Return:
 * 
 *   the packed ARGB color
 */
uint32_t sph_argb_pack(const SPH_ARGB *pc);

/*
 * Given a packed ARGB color, unpack it into a parsed ARGB color
 * structure.
 * 
 * The eight most significant bits of the passed color are the alpha
 * channel value, then eight bits for red, then eight for green, and
 * finally the eight least significant bits are the blue channel.
 * 
 * Parameters:
 * 
 *   c - the packed ARGB color
 * 
 *   pc - pointer to the parsed ARGB structure to receive the results
 */
void sph_argb_unpack(uint32_t c, SPH_ARGB *pc);

/*
 * Given a parsed ARGB color, down-convert it to RGB.
 * 
 * First, all channels are clamped to range [0, 255].
 * 
 * There are then three cases, depending on the alpha channel value.  If
 * the alpha channel value is 255, then the RGB channels receive no
 * further modification.  If the alpha channel value is zero, then the
 * the alpha, red, green, and blue channels are all set to 255.
 * 
 * If the alpha channel value is in range [1, 254], then each RGB
 * channel is transformed as follows:
 * 
 *   result = 255 + ((alpha * (v - 255)) / 255)
 * 
 * Results are clamped to range [0, 255], and then the alpha channel is
 * set to 255.  This approximates mixing a partially transparent pixel
 * against an opaque white background, though it does not take into
 * account the gamma encoding of the color channels.
 * 
 * After this operation, the alpha channel will always have a value of
 * 255, meaning fully opaque.
 * 
 * Parameters:
 * 
 *   pc - the parsed ARGB color to down-convert to RGB
 */
void sph_argb_downRGB(SPH_ARGB *pc);

/*
 * Given a parsed ARGB color, down-convert it to grayscale.
 * 
 * First, sph_argb_downRGB() is run on the color to down-convert it to
 * RGB.
 * 
 * Then, if the RGB channels do not all have the same value, they are
 * all replaced with a grayscale value computed as follows:
 * 
 *   gray = (2126 * r + 7152 * g + 722 * b) / 10000
 * 
 * This formula is an integer version of the luma function found in the
 * ITU-R BT.709 recommendation, which is equivalent to sRGB in this case
 * because it has the same RGB primaries.
 * 
 * This is not the highest-quality grayscale function possible, because
 * it does not take gamma encoding into account.  If the provided color
 * is already grayscale, it will not be changed by this function.
 * 
 * After this operation, the alpha channel will always have a value of
 * 255 (meaning fully opaque), and the RGB channels will all always have
 * the equal values.
 * 
 * Parameters:
 * 
 *   pc - the parsed ARGB color to down-convert to grayscale
 */
void sph_argb_downGray(SPH_ARGB *pc);

/*
 * Allocate a new image writer object, given a handle.
 * 
 * pOut is the handle to the image file to write.  The handle must be
 * open for writing, and it should refer to a normal disk file, or
 * undefined behavior occurs.  The image writer takes ownership of the
 * file handle, and it will automatically close the file handle when the
 * writer object is closed.
 * 
 * ftype is the type of image file to write.  It must be one of the
 * SPH_IMAGE_TYPE constants.  (Currently, only the PNG type is
 * supported.)
 * 
 * w and h are the dimensions of the image, in pixels.  Each value must
 * be at least one and no greater than SPH_IMAGE_MAXDIM.
 * 
 * dconv is the type of down-conversion requested.  It must be one of
 * the SPH_IMAGE_DOWN constants.  Pass zero or SPH_IMAGE_DOWN_NONE if no
 * down-conversion is requested.  JPEG files must use either RGB or
 * grayscale down-conversion.
 * 
 * q is reserved for a compression quality value.  It is not currently
 * used and should be set to zero.
 * 
 * After an image writer is allocated, call sph_image_writer_write() to
 * write each scanline.  Then, close the image writer.  All image writer
 * objects should eventually be closed with sph_image_writer_close().
 * 
 * Parameters:
 * 
 *   pOut - the handle to the output file
 * 
 *   ftype - the type of image to write
 * 
 *   w - the width of the image in pixels
 * 
 *   h - the height of the image in pixels
 * 
 *   dconv - the down-conversion requested
 * 
 *   q - reserved, set to zero
 * 
 * Return:
 * 
 *   the new image writer object
 */
SPH_IMAGE_WRITER *sph_image_writer_new(
    FILE    * pOut,
    int       ftype,
    int32_t   w,
    int32_t   h,
    int       dconv,
    int       q);

/*
 * A wrapper around sph_image_writer_new() that takes a file path.
 * 
 * pPath is the path to the image file that should be written.  If a
 * file currently exists at that path, it will be overwritten.  If the
 * path can not be opened for writing, the function fails and returns
 * NULL.
 * 
 * The last characters in the file path must be a case-insensitive match
 * for one of the following:
 * 
 *   .PNG
 * 
 * This is used to automatically determine the type of image file that
 * should be written (currently only PNG supported).  If the end of the
 * path does not match one of the above strings, then the function fails
 * and returns NULL.
 * 
 * The parameters w h dconv and q are passed through as-is.
 * 
 * If pError is provided, then it will always be filled in upon return
 * either with an error code (if the function fails) or with zero
 * (SPH_IMAGE_ERR_NONE) if the function is successful.
 * 
 * See sph_image_writer_new() for further information.
 * 
 * Parameters:
 * 
 *   pPath - the path to the image file to write
 * 
 *   w - the width of the image in pixels
 * 
 *   h - the height of the image in pixels
 * 
 *   dconv - the down-conversion requested
 * 
 *   q - reserved, set to zero
 * 
 *   pError - pointer to error return, or NULL
 * 
 * Return:
 * 
 *   the new image writer object, or NULL
 */
SPH_IMAGE_WRITER *sph_image_writer_newFromPath(
    const char    * pPath,
          int32_t   w,
          int32_t   h,
          int       dconv,
          int       q,
          int     * pError);

/*
 * Close a given image writer object.
 * 
 * The file handle within the object is also closed.  If a writer object
 * is closed before all scanlines have been written, the state of the
 * output file will be invalid.
 * 
 * If NULL is passed, the call is ignored.
 * 
 * Parameters:
 * 
 *   pw - the image writer object, or NULL
 */
void sph_image_writer_close(SPH_IMAGE_WRITER *pw);

/*
 * Get a pointer to the scanline buffer of an image writer.
 * 
 * The scanline buffer is cleared to fully transparent black pixels when
 * the writer object is created and after every write operation.
 * 
 * Use sph_image_writer_write() to write the contents of the scanline
 * buffer to the image file.
 * 
 * The scanline buffer has one 32-bit unsigned integer for each pixel.
 * The width in pixels of the scanline buffer is the width of the image
 * that was specified to sph_image_writer_new().
 * 
 * In each pixel, the eight most significant bits are alpha channel,
 * then eight bits for red, eight bits for green, and the eight least
 * significant bits are the blue channel.  The alpha channel has a
 * linear scale and is non-premultiplied with respect to the RGB
 * channels.  The RGB channels are non-linear and the sRGB color space
 * should be assumed.
 * 
 * The pointer remains valid until the image writer object is closed.
 * 
 * Parameters:
 * 
 *   pw - the image writer object
 * 
 * Return:
 * 
 *   a pointer to the scanline buffer
 */
uint32_t *sph_image_writer_ptr(SPH_IMAGE_WRITER *pw);

/*
 * Transfer a scanline to the given image writer object.
 * 
 * pw is the image writer object.  A fault occurs if all scanlines have
 * already been written.  Scanlines should be written from top to
 * bottom.
 * 
 * The data to write is held in the image writer object's scanline
 * buffer.  Use sph_image_writer_ptr() to get a pointer to the buffer.
 * 
 * The contents of the scanline buffer are unmodified by this function.
 * 
 * Once all scanlines have been written (the total number of scanlines
 * must equal the height specified to sph_image_writer_new()), the image
 * writer object may be closed with sph_image_writer_close().  If the
 * image writer object is closed before all scanlines have been written,
 * the output file will be invalid.
 * 
 * Parameters:
 * 
 *   pw - the image writer object
 */
void sph_image_writer_write(SPH_IMAGE_WRITER *pw);

/*
 * Allocate a new image reader object, given a handle.
 * 
 * pIn is the handle to the image file to read.  The handle must be open
 * for reading, and it should refer to a normal disk file, or undefined
 * behavior occurs.  The image reader takes ownership of the file 
 * handle, and it will automatically close the file handle when the 
 * reader object is closed.
 * 
 * ftype is the type of image file to read.  It must be one of the
 * SPH_IMAGE_TYPE constants.  (Currently only PNG is supported.)
 * 
 * After an image reader is allocated, sph_image_reader_width() and
 * sph_image_reader_height() can retrieve the dimensions of the file,
 * while sph_image_reader_read() can read the scanlines of the image.
 * Close the image reader with sph_image_reader_close() when done.
 * 
 * If there is an error, NULL is returned.  If pError is not NULL, then
 * an error code will be written there on failure, or a zero error
 * (SPH_IMAGE_ERR_NONE) on success.  If an error occurs, the file
 * handle will be closed by this function.
 * 
 * Parameters:
 * 
 *   pIn - the handle to the input file
 * 
 *   ftype - the type of image to read
 * 
 *   pError - pointer to the error return, or NULL
 * 
 * Return:
 * 
 *   the new image reader object, or NULL
 */
SPH_IMAGE_READER *sph_image_reader_new(
    FILE * pIn,
    int    ftype,
    int  * pError);

/*
 * A wrapper around sph_image_reader_new() that takes a file path.
 * 
 * pPath is the path to the image file that should be read.  If the path
 * can not be opened for reading, the function fails and returns NULL.
 * 
 * The last characters in the file path must be a case-insensitive match
 * for one of the following:
 * 
 *   .PNG
 * 
 * This is used to automatically determine the type of image file that
 * should be written (currently only PNG supported).  If the end of the
 * path does not match one of the above strings, then the function fails
 * and returns NULL.
 * 
 * The pError parameter is passed through as-is.  This function may
 * return SPH_IMAGE_ERR_FILETYPE error if the file extension couldn't
 * be recognized.
 * 
 * See sph_image_reader_new() for further information.
 * 
 * Parameters:
 * 
 *   pPath - the path to the image file to read
 * 
 *   pError - pointer to the error return, or NULL
 * 
 * Return:
 * 
 *   the new image reader object, or NULL
 */
SPH_IMAGE_READER *sph_image_reader_newFromPath(
    const char * pPath,
          int  * pError);

/*
 * Close a given image reader object.
 * 
 * The file handle within the object is also closed.  The image reader
 * may be closed at any time.
 * 
 * If NULL is passed, the call is ignored.
 * 
 * Parameters:
 * 
 *   pr - the image reader object, or NULL
 */
void sph_image_reader_close(SPH_IMAGE_READER *pr);

/*
 * Get the width of the image in pixels.
 * 
 * The range is [1, SPH_IMAGE_MAXDIM].
 * 
 * Parameters:
 * 
 *   pr - the image reader object
 * 
 * Return:
 * 
 *   the width in pixels
 */
int32_t sph_image_reader_width(SPH_IMAGE_READER *pr);

/*
 * Get the height of the image in pixels.
 * 
 * The range is [1, SPH_IMAGE_MAXDIM].
 * 
 * Parameters:
 * 
 *   pr - the image reader object
 * 
 * Return:
 * 
 *   the height in pixels
 */
int32_t sph_image_reader_height(SPH_IMAGE_READER *pr);

/*
 * Read the next scanline of the image.
 * 
 * The return value is a pointer to the scanline buffer.  The buffer has
 * a number of pixels equal to the width of the image (as determined by
 * sph_image_reader_width()).  Each pixel is an unsigned 32-bit integer
 * where the eight most significant bits are the alpha channel, then
 * eight bits for red, eight bits for green, and the eight least
 * significant bits are blue.  The alpha channel has a linear scale and
 * is non-premultiplied with respect to the RGB channels.  The RGB
 * channels are non-linear and the sRGB color space should be assumed.
 * 
 * The client may modify the buffer.  The pointer remains valid until
 * the next call to sph_image_reader_read() or until the reader object
 * is closed (whichever occurs first).
 * 
 * Each time this function is called, it reads another scanline from the
 * image.  A fault occurs if it tries to read more than the total number
 * of scanlines in the image (see sph_image_reader_height()).  Scanlines
 * are read from top to bottom, and within scanlines pixels go from left
 * to right.
 * 
 * If there is a read error, NULL will be returned.  All subsequent
 * reads from the file after a read error occurs will also result in
 * read errors.
 * 
 * pError, if provided, will be set to an error code if there is an
 * error, or zero (SPH_IMAGE_ERR_NONE) if there was no error.
 * 
 * Parameters:
 * 
 *   pr - the image reader object
 * 
 *   pError - pointer to the error code return, or NULL
 * 
 * Return:
 * 
 *   pointer to the scanline buffer, or NULL if read error
 */
uint32_t *sph_image_reader_read(SPH_IMAGE_READER *pr, int *pError);

/*
 * Given an SPH_IMAGE_ERR error code, return a string describing the
 * error.
 * 
 * SPH_IMAGE_ERR_NONE returns a string indicating no error.  All
 * unrecognized codes and SPH_IMAGE_ERR_UNKNOWN return a string saying
 * that an unknown image error occurred.  Each recognized code has its
 * own descriptive string.
 * 
 * The error strings are all in English, first letter capitalized, no
 * punctuation at the end of the string, and no line break.
 * 
 * Parameters:
 * 
 *   code - the image error code to look up
 * 
 * Return:
 * 
 *   a string description of the image error code
 */
const char *sph_image_errorString(int code);

#endif
