#ifndef SOPHISTRY_H_INCLUDED
#define SOPHISTRY_H_INCLUDED

/*
 * sophistry.h
 * 
 * Header for the Sophistry library, which defines a common image
 * scanline and audio buffer format.
 * 
 * Compilation
 * ===========
 * 
 * If <stdint.h> is not available for the local compiler, see the
 * documentation of the integer types below in this header for a
 * workaround.
 */

#include <stddef.h>

/*
 * The integer types.
 * 
 * sphy_u32 is an unsigned, 32-bit integer that is used to store packed
 * samples for both image scanlines and audio buffers.
 * 
 * sphy_int is a signed integer type that must be at least 32-bit.
 * 
 * sphy_u8 is an unsigned, 8-bit integer that is used to represent
 * bytes.
 * 
 * The definitions given here use the <stdint.h> header.  If this
 * header is not available, comment out the include below and replace
 * the uint32_t, int32_t, and uint8_t in the definitions below with
 * whatever are the appropriate types for the compiler.
 */
#include <stdint.h>
typedef uint32_t sphy_u32;    /* Unsigned integer, exactly  32-bit */
typedef int32_t  sphy_int;    /* Signed   integer, at least 32-bit */
typedef uint8_t  sphy_u8 ;    /* Unsigned integer, exactly   8-bit */

/*
 * The maximum number of samples in an object.
 * 
 * This is the upper limit on the width of a scanline, and the upper
 * limit on the number of samples in an audio buffer.
 */
#define SPHY_MAXCOUNT ((sphy_int) 268435456L)

/*
 * Structure prototypes.
 * 
 * The actual structures are defined in the implementation file.
 */
struct SPHY_SCANLINE_TAG;
typedef struct SPHY_SCANLINE_TAG SPHY_SCANLINE;

struct SPHY_AUDIOBUF_TAG;
typedef struct SPHY_AUDIOBUF_TAG SPHY_AUDIOBUF;

struct SPHY_ISRCFMT_TAG;
typedef struct SPHY_ISRCFMT_TAG SPHY_ISRCFMT;

/*
 * Status code definitions.
 */
#define SPHY_OK         ( 0)  /* Operation successful */

/*
 * Constructor for a scanline.
 * 
 * Pass the width of the scanline in pixels.  The width must be at least
 * one and at most SPHY_MAXCOUNT.
 * 
 * The returned scanline should eventually be freed with
 * sphy_scanline_free.
 * 
 * NULL is returned if memory allocation failed.
 * 
 * Parameters:
 * 
 *   width - width of the scanline in pixels
 * 
 * Return:
 * 
 *   the new scanline object, or NULL if memory allocation failed
 */
SPHY_SCANLINE *sphy_scanline_alloc(sphy_int width);

/*
 * Free an allocated scanline object.
 * 
 * The scanline object must not be used after this call.  If NULL is
 * passed, the call is ignored.
 * 
 * Parameters:
 * 
 *   psl - the scanline object to free, or NULL
 */
void sphy_scanline_free(SPHY_SCANLINE *psl);

/*
 * Get a pointer to the internal samples of the given scanline object.
 * 
 * The pointer remains valid until the scanline object is freed with
 * sphy_scanline_free.  The total number of samples is equal to the
 * width of the scanline, as determined by sphy_scanline_count.
 * 
 * Parameters:
 * 
 *   psl - the scanline object to query
 * 
 * Return:
 * 
 *   a pointer to the internal samples
 */
sphy_u32 *sphy_scanline_ptr(SPHY_SCANLINE *psl);

/*
 * Import decoded pixels from binary data into a scanline object.
 * 
 * psl is the scanline object to write the pixels into, while psrc
 * points to the bytes of data that shall be decoded.
 * 
 * offs is the zero-based index to begin writing encoded pixels to
 * within the scanline.  Passing zero means decoded pixels are written
 * starting at the first sample of the scanline.  offs must be at least
 * zero and less than the width, as determined by sphy_scanline_count.
 * 
 * src_fmt describes the format of the pixels that are encoded in the
 * binary data.
 * 
 * src_len is the length of the binary data in bytes.  It must be
 * greater than zero.  Its length must be such that there are no
 * partially encoded pixels at the end of the binary data.  Furthermore,
 * the number of pixels included in the binary data may not exceed
 * (width - offs), where width is the width of the scanline in pixels
 * and offs is the offs parameter.
 * 
 * The return value is SPHY_OK if successful, else one of the error
 * status codes.
 * 
 * Parameters:
 * 
 *   psl - the scanline object to import pixels into
 * 
 *   offs - the scanline offset to begin writing the imported pixels
 * 
 *   psrc - pointer to the binary data to decode pixels from
 * 
 *   src_len - the length of the binary data in bytes
 * 
 *   src_fmt - the format of the binary data to import from
 * 
 * Return:
 * 
 *   SPHY_OK if successful, else one of the error status codes
 */
sphy_int sphy_scanline_import(
    SPHY_SCANLINE *      psl,
    sphy_int             offs,
    const sphy_u8      * psrc,
    sphy_int             src_len,
    const SPHY_ISRCFMT * src_fmt);

/*
 * Return the width in pixels of the given scanline.
 * 
 * This is equal to the width that the scanline was allocated with.  Its
 * range is one up to and including SPHY_MAXCOUNT.
 * 
 * Parameters:
 * 
 *   psl - the scanline object to query
 * 
 * Return:
 * 
 *   the width of the scanline in pixels
 */
sphy_int sphy_scanline_count(SPHY_SCANLINE *psl);

/*
 * Constructor for an audio buffer.
 * 
 * Pass the number of samples in the audio buffer.  This count must be
 * at least one and at most SPHY_MAXCOUNT.
 * 
 * The returned audio buffer should eventually be freed with
 * sphy_audiobuf_free.
 * 
 * NULL is returned if memory allocation failed.
 * 
 * Parameters:
 * 
 *   samples - count of samples in the buffer
 * 
 * Return:
 * 
 *   the new audio buffer, or NULL if memory allocation failed
 */
SPHY_AUDIOBUF *sphy_audiobuf_alloc(sphy_int samples);

/*
 * Free an allocated audio buffer.
 * 
 * The audio buffer must not be used after this call.  If NULL is
 * passed, the call is ignored.
 * 
 * Parameters:
 * 
 *   pau - the audio buffer to free, or NULL
 */
void sphy_audiobuf_free(SPHY_AUDIOBUF *pau);

/*
 * Get a pointer to the internal samples of the given audio buffer.
 * 
 * The pointer remains valid until the audio buffer is freed with
 * sphy_audiobuf_free.  The total number of samples is equal to the
 * count of the audio buffer, as determined by sphy_audiobuf_count.
 * 
 * Parameters:
 * 
 *   pau - the audio buffer to query
 * 
 * Return:
 * 
 *   a pointer to the internal samples
 */
sphy_u32 *sphy_audiobuf_ptr(SPHY_AUDIOBUF *pau);

/*
 * Return the count of samples in the given audio buffer.
 * 
 * This is equal to the count that the audiob buffer was allocated with.
 * Its range is one up to and including SPHY_MAXCOUNT.
 * 
 * Parameters:
 * 
 *   pau - the audio buffer to query
 * 
 * Return:
 * 
 *   the count of samples in the buffer
 */
sphy_int sphy_audiobuf_count(SPHY_AUDIOBUF *pau);

#endif
