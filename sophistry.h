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
 * Each provided channel is clamped to range zero to SPHY_MAXBYTE using
 * sphy_clamp.
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
 * Each provided channel is clamped to range SPHY_MINSWORD to
 * SPHY_MAXSWORD using sphy_clamp.
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

/*
 * Unpack the alpha channel from the provided 32-bit image sample.
 * 
 * The return value will be in range zero up to and including
 * SPHY_MAXBYTE.
 * 
 * Parameters:
 * 
 *   argb - the packed image sample
 * 
 * Return:
 * 
 *   the alpha channel
 */
sphy_int sphy_a(sphy_u32 argb);

/*
 * Unpack the red channel from the provided 32-bit image sample.
 * 
 * The return value will be in range zero up to and including
 * SPHY_MAXBYTE.
 * 
 * Parameters:
 * 
 *   argb - the packed image sample
 * 
 * Return:
 * 
 *   the red channel
 */
sphy_int sphy_r(sphy_u32 argb);

/*
 * Unpack the green channel from the provided 32-bit image sample.
 * 
 * The return value will be in range zero up to and including
 * SPHY_MAXBYTE.
 * 
 * Parameters:
 * 
 *   argb - the packed image sample
 * 
 * Return:
 * 
 *   the green channel
 */
sphy_int sphy_g(sphy_u32 argb);

/*
 * Unpack the blue channel from the provided 32-bit image sample.
 * 
 * The return value will be in range zero up to and including
 * SPHY_MAXBYTE.
 * 
 * Parameters:
 * 
 *   argb - the packed image sample
 * 
 * Return:
 * 
 *   the blue channel
 */
sphy_int sphy_b(sphy_u32 argb);

/*
 * Unpack the left channel from the provided 32-bit audio sample.
 * 
 * The return value will be in range SPHY_MINSWORD up to and including
 * SPHY_MAXSWORD.
 * 
 * Parameters:
 * 
 *   stereo - the packed audio sample
 * 
 * Return:
 * 
 *   the left channel
 */
sphy_int sphy_left(sphy_u32 stereo);

/*
 * Unpack the right channel from the provided 32-bit audio sample.
 * 
 * The return value will be in range SPHY_MINSWORD up to and including
 * SPHY_MAXSWORD.
 * 
 * Parameters:
 * 
 *   stereo - the packed audio sample
 * 
 * Return:
 * 
 *   the right channel
 */
sphy_int sphy_right(sphy_u32 stereo);

/*
 * Bound a floating point value to unsigned normal range.
 * 
 * If the provided value is greater than or equal to 0.0 and less than
 * or equal to +1.0, then it is left alone.
 * 
 * If the provided value is greater than +1.0, it is set to +1.0.
 * 
 * If the provided value is less than 0.0, it is set to 0.0.
 * 
 * In all other cases (such as for a NaN), the value is set to 0.0.
 * 
 * The return value will always be in range 0.0 up to +1.0.
 * 
 * Parameters:
 * 
 *   f - the floating point value to bound
 * 
 * Return:
 * 
 *   the value bounded to unsigned normal range
 */
sphy_float sphy_boundu(sphy_float f);

/*
 * Bound a floating point value to signed normal range.
 * 
 * If the provided value is greater than or equal to -1.0 and less than
 * or equal to +1.0, then it is left alone.
 * 
 * If the provided value is greater than +1.0, it is set to +1.0.
 * 
 * If the provided value is less than -1.0, it is set to -1.0.
 * 
 * In all other cases (such as for a NaN), the value is set to 0.0.
 * 
 * The return value will always be in range -1.0 up to +1.0.
 * 
 * Parameters:
 * 
 *   f - the floating point value to bound
 * 
 * Return:
 * 
 *   the value bounded to unsigned normal range
 */
sphy_float sphy_bounds(sphy_float f);

/*
 * Clamp an integer value to range.
 * 
 * minval must be less than or equal to maxval.
 * 
 * If i is less than minval, it is set to minval.  If i is greater than
 * maxval, it is set to maxval.  Otherwise, it is left alone.
 * 
 * The return value will always be in range minval up to and including
 * maxval.
 * 
 * Parameters:
 * 
 *   i - the integer value to clamp
 * 
 *   minval - the minimum value in the clamped range
 * 
 *   maxval - the maximum value in the clamped range
 * 
 * Return:
 * 
 *   the clamped integer value
 */
sphy_int sphy_clamp(sphy_int i, sphy_int minval, sphy_int maxval);

/*
 * Quantize an unsigned normal component into an unsigned integer
 * component.
 * 
 * The provided normal component (f) is first bounded to unsigned normal
 * range (0.0 to +1.0) using sphy_boundu.
 * 
 * maxval must be in range one up to and including SPHY_MAXUWORD.  The
 * bounded unsigned normal component is then multiplied by maxval.  This
 * result is then rounded to the nearest integer.
 * 
 * Finally, the rounded result is converted to an integer and clamped to
 * the integer range zero to maxval by using sphy_clamp.
 * 
 * The return value is in range zero up to and including maxval.
 * 
 * Parameters:
 * 
 *   f - the unsigned normal component to quantize
 * 
 *   maxval - the maximum integer value
 * 
 * Return:
 * 
 *   the quantized unsigned value
 */
sphy_int sphy_quantu(sphy_float f, sphy_int maxval);

/*
 * Quantize a signed normal component into a signed integer component.
 * 
 * The provided normal component (f) is first bounded to signed normal
 * range (-1.0 to +1.0) using sphy_bounds.
 * 
 * maxval must be in range one up to and including SPHY_MAXSWORD.  The
 * bounded signed normal component is then multiplied by maxval.  This
 * result is then rounded to the nearest integer.
 * 
 * Finally, the rounded result is converted to an integer and clamped to
 * the integer range -maxval to +maxval by using sphy_clamp.
 * 
 * The return value is in range -maxval up to and including +maxval.
 * 
 * Note that in the common two's-complement representation of signed
 * integers, the negative range goes one further than the positive range
 * (that is, (-maxval-1) up to +maxval).  This conversion function never
 * generates the "one further" negative value of (-maxval-1), so that
 * the range of values is always symmetric around zero.
 * 
 * Parameters:
 * 
 *   f - the signed normal component to quantize
 * 
 *   maxval - the maximum integer value
 * 
 * Return:
 * 
 *   the quantized signed value
 */
sphy_int sphy_quants(sphy_float f, sphy_int maxval);

/*
 * Normalize an unsigned integer component into a normal float.
 * 
 * The provided integer component (i) is first clamped to range zero to
 * maxval by using sphy_clamp.  maxval must be in range one up to and
 * including SPHY_MAXUWORD.
 * 
 * The integer component is then converted to a float value and divided
 * by maxval.  Finally, the float value is bounded to unsigned range
 * (0.0 to +1.0) using sphy_boundu.
 * 
 * The return value is in range 0.0 to +1.0.
 * 
 * Parameters:
 * 
 *   i - the unsigned integer component to normalize
 * 
 *   maxval - the maximum integer value
 * 
 * Return:
 * 
 *   the normalized unsigned value
 */
sphy_float sphy_normu(sphy_int i, sphy_int maxval);

/*
 * Normalize a signed integer component into a normal float.
 * 
 * The provided integer component (i) is first clamped to range -maxval
 * to maxval by using sphy_clamp.  maxval must be in range one up to and
 * including SPHY_MAXSWORD.
 * 
 * The integer component is then converted to a float value and divided
 * by maxval.  Finally, the float value is bounded to signed range
 * (-1.0 to +1.0) using sphy_bounds.
 * 
 * The return value is in range -1.0 to +1.0.
 * 
 * Note that in the common two's-complement representation of signed
 * integers, the negative range goes one further than the positive range
 * (that is, (-maxval-1) up to +maxval).  This conversion function
 * treats the "one further" negative value of (-maxval-1) as if it were
 * -maxval, so that the range of values is always symmetric around zero.
 * 
 * Parameters:
 * 
 *   i - the signed integer component to normalize
 * 
 *   maxval - the maximum integer value
 * 
 * Return:
 * 
 *   the normalized signed value
 */
sphy_float sphy_norms(sphy_int i, sphy_int maxval);

/*
 * Requantize an unsigned integer component to a different unsigned
 * integer range.
 * 
 * omaxval is the original maxval that the provided integer component
 * (i) is quantized to.  nmaxval is the new maxval that the return value
 * should be quantized to.  Both omaxval and nmaxval must be in range
 * one up to and including SPHY_MAXUWORD.
 * 
 * First, the provided integer component (i) is clamped to range zero to
 * +omaxval by using sphy_clamp.
 * 
 * Second, the integer is converted to a float value and divided by
 * omaxval to convert to an unsigned normal in range 0.0 to 1.0.
 * 
 * Third, the result of this operation is sent to sphy_quantu using
 * nmaxval to convert to an unsigned integer in range zero to nmaxval.
 * 
 * The return value is in range zero to nmaxval.
 * 
 * As a special case, if omaxval and nmaxval are equal to each other,
 * the return value is simply i clamped to range zero to nmaxval using
 * sphy_clamp.
 * 
 * Parameters:
 * 
 *   i - the unsigned integer component to requantize
 * 
 *   omaxval - the original maximum integer value of i
 * 
 *   nmaxval - the new maximum integer value for the return value
 * 
 * Return:
 * 
 *  the requantized integer component
 */
sphy_int sphy_requantu(sphy_int i, sphy_int omaxval, sphy_int nmaxval);

/*
 * Requantize a signed integer component to a different signed integer
 * range.
 * 
 * omaxval is the original maxval that the provided integer component
 * (i) is quantized to.  nmaxval is the new maxval that the return value
 * should be quantized to.  Both omaxval and nmaxval must be in range
 * one up to and including SPHY_MAXSWORD.
 * 
 * First, the provided integer component (i) is clamped to range
 * -omaxval to +omaxval by using sphy_clamp.
 * 
 * Second, the integer is converted to a float value and divided by
 * omaxval to convert to a signed normal in range -1.0 to 1.0.
 * 
 * Third, the result of this operation is sent to sphy_quants using
 * nmaxval to convert to a signed integer in range -nmaxval to +nmaxval.
 * 
 * The return value is in range -nmaxval to +nmaxval.
 * 
 * Note that in the common two's-complement representation of signed
 * integers, the negative range goes one further than the positive range
 * (that is, (-omaxval-1) up to +omaxval).  This conversion function
 * treats the "one further" negative value of (-omaxval-1) as if it were
 * -omaxval, so that the range of values is always symmetric around
 * zero.  The return value will never return a "one further" negative
 * value of (-nmaxval-1).
 * 
 * As a special case, if omaxval and nmaxval are equal to each other,
 * the return value is simply i clamped to range -nmaxval to nmaxval
 * using sphy_clamp.
 * 
 * Parameters:
 * 
 *   i - the signed integer component to requantize
 * 
 *   omaxval - the original maximum integer value of i
 * 
 *   nmaxval - the new maximum integer value for the return value
 * 
 * Return:
 * 
 *  the requantized integer component
 */
sphy_int sphy_requants(sphy_int i, sphy_int omaxval, sphy_int nmaxval);

#endif
