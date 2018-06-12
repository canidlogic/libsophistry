#ifndef SOPHISTRY_H_INCLUDED
#define SOPHISTRY_H_INCLUDED

/*
 * sophistry.h
 * 
 * Header for the Sophistry library, which defines a 32-bit image and
 * audio sample format.
 * 
 * See the README.md document for further information.
 */

#include <stddef.h>

/*
 * The basic type declarations.
 * 
 * If <stdint.h> is not supported by the compiler, then comment that
 * line out and rewrite the typedefs so they use the appropriate types
 * for the compiler.
 * 
 * If some of the standard definitions used by the typedefs are missing,
 * then rewrite the typedefs to use the definitions appropriate for the
 * compiler.
 */
#include <stdint.h>
typedef uint32_t sphy_u32  ;  /* Unsigned integer, exactly  32-bit */
typedef int32_t  sphy_int  ;  /* Signed   integer, at least 32-bit */
typedef float    sphy_float;  /* Floating-point type               */

/*
 * The basic limits.
 */
#define SPHY_MAXBYTE  (              255 )  /* Max unsigned  8-bit */
#define SPHY_MAXUWORD ((sphy_int)  65535L)  /* Max unsigned 16-bit */
#define SPHY_MAXSWORD ((sphy_int)  32767L)  /* Max   signed 16-bit */
#define SPHY_MINSWORD ((sphy_int) -32768L)  /* Min   signed 16-bit */

/*
 * Pack ARGB channels into a 32-bit image sample.
 * 
 * Each provided channel is clamped to the range zero up to and
 * including SPHY_MAXBYTE.  That is, if a channel value is greater than
 * SPHY_MAXBYTE, it will be written as SPHY_MAXBYTE, while if it is less
 * than zero, it will be written as zero.
 * 
 * Parameters:
 * 
 *   a - the alpha channel
 * 
 *   r - the red channel
 * 
 *   g - the green channel
 * 
 *   b - the blue channel
 * 
 * Return:
 * 
 *   the packed 32-bit image sample
 */
sphy_u32 sphy_argb(sphy_int a, sphy_int r, sphy_int g, sphy_int b);

/*
 * Pack left and right channels into a 32-bit image sample.
 * 
 * Each provided channel is clamped to the range SPHY_MINSWORD up to and
 * including SPHY_MAXSWORD.  That is, if a channel value is greater than
 * SPHY_MAXSWORD, it will be written as SPHY_MAXSWORD, while if it is
 * less than SPHY_MINSWORD, it will be written as SPHY_MINSWORD.
 * 
 * The packed channel values will be converted to unsigned range zero up
 * to and including SPHY_MAXUWORD by adding 32768 to each clamped
 * channel value.
 * 
 * Parameters:
 * 
 *   left - the left channel
 * 
 *   right - the right channel
 * 
 * Return:
 * 
 *   the packed 32-bit audio sample
 */
sphy_u32 sphy_stereo(sphy_int left, sphy_int right);

#endif
