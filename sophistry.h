#ifndef SOPHISTRY_H_INCLUDED
#define SOPHISTRY_H_INCLUDED

/*
 * sophistry.h
 * 
 * An image codec library based on the PNG image format.
 * 
 * All declarations have a prefix that is a case-insensitive match for
 * "sphi"
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * The maximum dimension allowed for width and height.
 * 
 * This is chosen to allow width and height to be multiplied together
 * without having to worry about overflow in signed 32-bit range.
 */
#define SPHI_MAXDIM (32000)

/*
 * Sample count constants.
 * 
 * This indicates the total number of color channels, which is always in
 * range [1..4].  The first set of constants gives the color
 * interpretations for each sample count, but note that a sample count
 * of one has two interpretations, depending on whether there is a color
 * palette (no palette means grayscale, while a palette means indexed
 * color).
 * 
 * The MIN and MAX constants give the full range of possible sample
 * count values.
 */
#define SPHI_SACOUNT_GRAY    (1)	/* Grayscale                 */
#define SPHI_SACOUNT_INDEXED (1)	/* Indexed color             */
#define SPHI_SACOUNT_GRAYA   (2)	/* Grayscale with alpha      */
#define SPHI_SACOUNT_RGB     (3)	/* Red/Green/Blue            */
#define SPHI_SACOUNT_RGBA    (4)	/* Red/Green/Blue with alpha */

#define SPHI_SACOUNT_MIN     (1)
#define SPHI_SACOUNT_MAX     (4)

/*
 * Sample depth constants.
 * 
 * These are the allowable bit depths for each color channel.  Only the
 * values given in the constants below are supported.
 */
#define SPHI_SADEPTH_1		(1 )	/*  1-bit */
#define SPHI_SADEPTH_2		(2 )	/*  2-bit */
#define SPHI_SADEPTH_4		(4 )	/*  4-bit */
#define SPHI_SADEPTH_8		(8 )	/*  8-bit */
#define SPHI_SADEPTH_16		(16)	/* 16-bit */

/*
 * Maximum palette count constants.
 * 
 * These are the maximum number of palette entries allowed for a given
 * bit depth.  16-bit depth is not supported for indexed color, so the
 * constant for that is zero, indicating no palette.
 * 
 * The MAX constant is the maximum number of palette entries of any bit
 * depth.
 */
#define SPHI_PALMAX_1   (2  )
#define SPHI_PALMAX_2   (4  )
#define SPHI_PALMAX_4   (16 )
#define SPHI_PALMAX_8   (256)
#define SPHI_PALMAX_16  (0  )

#define SPHI_PALMAX_MAX (256)

/*
 * Maximum color channel values for each bit depth.
 * 
 * Each color channel has a range of zero up to and including the
 * constant below corresponding to the selected bit depth.
 */
#define SPHI_SAMAX_1  (1    )
#define SPHI_SAMAX_2  (3    )
#define SPHI_SAMAX_4  (15   )
#define SPHI_SAMAX_8  (255  )
#define SPHI_SAMAX_16 (65535)

/*
 * Colorspace mode constants.
 * 
 * There are four sRGB modes, each with a different rendering intent,
 * which hints at how to best handle translations between color spaces.
 * 
 * Perceptual means that colors should be kept in similar perceptual
 * relationships to each other, which is usually a good choice.
 * 
 * Relative means that colors should be kept the same, and colors that
 * can't be translated should be clamped to the boundaries of the target
 * color space.  This can result in better color fidelty if most of the
 * colors are in range of the target color space, but it can also result
 * in posterization artifacts if too many colors fall outside of the
 * target range.
 * 
 * Saturation means that the saturation of colors is more important than
 * other color dimensions.  This can be useful for abstract uses of
 * color, such as in pie charts and other such diagrams.
 * 
 * Absolute means that absolute charateristics of colors should be
 * preserved as much as possible.  This may result in significant
 * perceptual distortion since the white point is not accounted for.
 * This should only be used for special cases in advanced color
 * management workflows.
 * 
 * It is up to the decoder to decide how to handle these rendering
 * intent hints, or whether to ignore them.
 * 
 * All non-sRGB color spaces (differing by gamma, chromaticity, or both)
 * use the non-sRGB mode.
 */
#define SPHI_CMODE_SRGB_PERCEPTUAL (1)
#define SPHI_CMODE_SRGB_RELATIVE   (2)
#define SPHI_CMODE_SRGB_SATURATION (3)
#define SPHI_CMODE_SRGB_ABSOLUTE   (4)
#define SPHI_CMODE_NON_SRGB        (5)

/*
 * A multiplier used for storing exact decimal values.
 * 
 * Certain fields specify that fractional values (such as 1.23456) are
 * multiplied by this constant so that they can be stored as an integer
 * (123456).
 * 
 * This multiplier is from the PNG specification.
 */
#define SPHI_MULTIPLIER (100000)

/*
 * The sRGB gamma and chromaticity values.
 * 
 * These are represented as fractions multiplied by SPHI_MULTIPLIER to
 * store as integers.  They are from the PNG specification.
 * 
 * The "X" and "Y" values are actually the (lowercase) x and y values
 * from xyY color representation.  They are in uppercase here because of
 * the convention that constant names are in all uppercase.
 */
#define SPHI_SRGB_GAMMA   (45455)
#define SPHI_SRGB_WHITE_X (31270)
#define SPHI_SRGB_WHITE_Y (32900)
#define SPHI_SRGB_RED_X   (64000)
#define SPHI_SRGB_RED_Y   (33000)
#define SPHI_SRGB_GREEN_X (30000)
#define SPHI_SRGB_GREEN_Y (60000)
#define SPHI_SRGB_BLUE_X  (15000)
#define SPHI_SRGB_BLUE_Y  (6000 )

/*
 * The predefined configuration constants.
 * 
 * These are used with sphi_initInfo to select a sensible
 * preconfiguration of the SPHI_METAINFO structure.
 * 
 * All preconfigurations have interlacing, falsecolor, and background
 * set to false, the colorspace set to sRGB perceptual, the pixel aspect
 * set to 1:1 (square pixels) in relative units, and the bit depth set
 * to 8-bit.
 * 
 * Palette-based preconfigurations initialize each palette entry to zero
 * for alpha, red, green, and blue, meaning fully transparent.  The
 * client should write the appropriate palette values into the
 * structure.
 */
#define SPHI_PREDEF_RGB    (1)	/* RGB with no alpha    */
#define SPHI_PREDEF_RGBA   (2)	/* RGB with alpha       */
#define SPHI_PREDEF_GRAY   (3)	/* Grayscale            */
#define SPHI_PREDEF_GRAYA  (4)	/* Grayscale with alpha */
#define SPHI_PREDEF_PAL256 (5)	/* Indexed color        */

/*
 * SPHI_PALENTRY
 * -------------
 * 
 * Structure for storing a color palette entry.
 */
typedef struct {
	
	/*
	 * The red channel,   with range [0, 255].
	 */
	uint8_t r;
	
	/*
	 * The green channel, with range [0, 255].
	 */
	uint8_t g;
	
	/*
	 * The blue channel,  with range [0, 255].
	 */
	uint8_t b;
	
	/*
	 * The alpha channel, with range [0, 255].
	 */
	uint8_t a;

} SPHI_PALENTRY;

/*
 * SPHI_METAINFO
 * -------------
 * 
 * Structure for storing the core metadata for an image.
 */
typedef struct {
	
	/*
	 * Image width in pixels.
	 * 
	 * This must be at least one and no greater than SPHI_MAXDIM.
	 */
	int32_t width;
	
	/*
	 * Image height in pixels.
	 * 
	 * This must be at least one and no greater than SPHI_MAXDIM.
	 */
	int32_t height;
	
	/*
	 * The sample count, which is the total number of color channels.
	 * 
	 * This must be in range SPHI_SACOUNT_MIN to SPHI_SACOUNT_MAX.  See
	 * The SPHI_SACOUNT constants for the interpretation of each value.
	 * 
	 * See sadepth for some restrictions on how sample counts can be
	 * combined with other parameters.
	 */
	int8_t sacount;
	
	/*
	 * The sample depth, which is the total number of bits for each
	 * color channel.
	 * 
	 * Only the values covered by the SPHI_SADEPTH constants are
	 * allowed.
	 * 
	 * In addition, only certain combinations of sacount, sadepth, and
	 * palcount (see later) are valid.  The rules are:
	 * 
	 * (1) 1-bit, 2-bit, and 4-bit depths may only be used with a
	 *     sacount of one (grayscale or indexed).
	 * 
	 * (2) 16-bit depth may only be used with a palcount of zero (no
	 *     palette).
	 */
	int8_t sadepth;
	
	/*
	 * The number of color palette entries, or zero if there is no color
	 * palette.
	 * 
	 * If palcount is non-zero, then sacount must be one.  palcount must
	 * be in range zero up to and including the SPHI_PALMAX constant for
	 * the selected bit depth.  (Note that the maximum for 16-bit depth
	 * is zero, indicating no palette is allowed at that depth.)
	 */
	int16_t palcount;
	
	/*
	 * The color palette, if applicable.
	 * 
	 * The palcount field determines how many of these records are
	 * actually used.  If palcount is zero, then there is no palette and
	 * none of these records are used.
	 * 
	 * Unused palette records have an undefined value.
	 * 
	 * Note that it is slightly more efficient when encoding PNG files
	 * if color palette entries that are fully or partially transparent
	 * come before color palette entries that are fully opaque in the
	 * palette.
	 */
	SPHI_PALENTRY pal[SPHI_PALMAX_MAX];
	
	/*
	 * Flag indicating whether the codec is handling pixels in
	 * interlaced order.
	 * 
	 * If true, then the pixel order is interlaced and more than one
	 * pass will be made to transfer the pixel data.  If false, then the
	 * pixel order is not interlaced and exactly one pass will be made
	 * to transfer the pixel data.
	 * 
	 * The client should make no assumptions about the interlaced or
	 * non-interlaced order of pixels.  Instead, the client should
	 * follow the pass specifications provided by the codec during pixel
	 * data transfer operations.  The only thing that may be reliably
	 * inferred from this flag is whether there is a single pass or
	 * whether there is more than one pass.
	 */
	bool interlace;
	
	/*
	 * Flag indicating whether there is a "false color" that will be
	 * interpreted as transparent.
	 * 
	 * This flag is only allowed if palcount is zero and sacount is
	 * neither two (grayscale with alpha) nor four (RGB with alpha).  In
	 * all other cases, this flag must be false.  (Note that the other
	 * cases include alpha channels, so a false color is unnecessary.)
	 * 
	 * This flag activates the false color fields below.  If an encoded
	 * color value exactly matches the false color channel values, then
	 * that pixel is treated as fully transparent.
	 */
	bool falsecolor;
	
	/*
	 * The red value of the false color.
	 * 
	 * This field is only relevant if the falsecolor flag is set -- see
	 * that field for further information.  Else, this field is
	 * undefined and ignored.
	 * 
	 * If sacount is one (grayscale, since indexed color is not allowed
	 * when the falsecolor flag is set), then false_r, false_g, and
	 * false_b must have the same value, which is the grayscale value.
	 * 
	 * The range of this value is zero up to and including the
	 * SPHI_SAMAX constant corresponding to the sample bit depth for
	 * this image.
	 */
	uint16_t false_r;
	
	/*
	 * The green value of the false color.
	 * 
	 * This field is only relevant if the falsecolor flag is set -- see
	 * that field for further information.  Else, this field is
	 * undefined and ignored.
	 * 
	 * If sacount is one (grayscale, since indexed color is not allowed
	 * when the falsecolor flag is set), then false_r, false_g, and
	 * false_b must have the same value, which is the grayscale value.
	 * 
	 * The range of this value is zero up to and including the
	 * SPHI_SAMAX constant corresponding to the sample bit depth for
	 * this image.
	 */
	uint16_t false_g;
	
	/*
	 * The blue value of the false color.
	 * 
	 * This field is only relevant if the falsecolor flag is set -- see
	 * that field for further information.  Else, this field is
	 * undefined and ignored.
	 * 
	 * If sacount is one (grayscale, since indexed color is not allowed
	 * when the falsecolor flag is set), then false_r, false_g, and
	 * false_b must have the same value, which is the grayscale value.
	 * 
	 * The range of this value is zero up to and including the
	 * SPHI_SAMAX constant corresponding to the sample bit depth for
	 * this image.
	 */
	uint16_t false_b;
	
	/*
	 * Flag indicating whether there is a default background color for
	 * the image.
	 * 
	 * This is a hint to clients that wish to display an image for what
	 * background color the image displays well against.  Clients are
	 * free to override this specification if they have other ideas
	 * about what the background should be.
	 * 
	 * This flag activates the background color fields defined below.
	 */
	bool background;
	
	/*
	 * The red value of the default background color.
	 * 
	 * This field is only relevant if the background flag is set and
	 * palcount is zero (indicating no palette).  Else, this field is
	 * undefined and ignored.
	 * 
	 * If sacount is one (grayscale, since palcount is zero when this
	 * field is active), then back_r, back_g, and back_b must have the
	 * same value, which is the grayscale value.
	 * 
	 * The range of this value is zero up to and including the
	 * SPHI_SAMAX constant corresponding to the sample bit depth for
	 * this image.
	 */
	uint16_t back_r;
	
	/*
	 * The green value of the default background color.
	 * 
	 * This field is only relevant if the background flag is set and
	 * palcount is zero (indicating no palette).  Else, this field is
	 * undefined and ignored.
	 * 
	 * If sacount is one (grayscale, since palcount is zero when this
	 * field is active), then back_r, back_g, and back_b must have the
	 * same value, which is the grayscale value.
	 * 
	 * The range of this value is zero up to and including the
	 * SPHI_SAMAX constant corresponding to the sample bit depth for
	 * this image.
	 */
	uint16_t back_g;
	
	/*
	 * The blue value of the default background color.
	 * 
	 * This field is only relevant if the background flag is set and
	 * palcount is zero (indicating no palette).  Else, this field is
	 * undefined and ignored.
	 * 
	 * If sacount is one (grayscale, since palcount is zero when this
	 * field is active), then back_r, back_g, and back_b must have the
	 * same value, which is the grayscale value.
	 * 
	 * The range of this value is zero up to and including the
	 * SPHI_SAMAX constant corresponding to the sample bit depth for
	 * this image.
	 */
	uint16_t back_b;
	
	/*
	 * The palette index of the color to use as a default background
	 * color.
	 * 
	 * This field is only relevant if the background flag is set and
	 * palcount is greater than zero (indicating a palette).  Else, this
	 * field is undefined and ignored.
	 * 
	 * The range of this value is zero up to but excluding palcount.
	 * Partially transparent palette entries are treated as if they were
	 * fully opaque when selected as the default background color.
	 */
	uint8_t back_i;
	
	/*
	 * The colorspace mode of this image.
	 * 
	 * This must be one of the SPHI_CMODE constants.  If one of the sRGB
	 * constants is selected, all the gamma and chromaticity fields must
	 * have the corresponding SPHI_SRGB constant value.  If the non-sRGB
	 * constant is selected, at least one of the gamma and chromaticity
	 * fields must have a value that differs from the corresponding
	 * SPHI_SRGB constant.
	 */
	uint8_t cmode;
	
	/*
	 * The encoding gamma value of this image.
	 * 
	 * Assuming that display component values and image component values
	 * are in range 0.0 to 1.0, the display component value raised to
	 * gamma yields the image component value.
	 * 
	 * Note that this is the encoding gamma, not the decoding gamma.
	 * For example, a monitor that has a decoding gamma of 2.2 has an
	 * encoding gamma of (1 / 2.2) ~= 0.45455
	 * 
	 * The actual encoding gamma is multiplied by SPHI_MULTIPLIER and
	 * then stored as an integer.  Hence, the encoding gamma of
	 * (1 / 2.2) will be stored in this field as 45455.
	 * 
	 * If cmode selects an sRGB mode, this field must be
	 * SPHI_SRGB_GAMMA.
	 */
	int32_t gamma;
	
	/*
	 * The x chromaticity of the white point.
	 * 
	 * This is represented in the xyY space, with Y assumed to be 1.0
	 * for white.  The actual value is multiplied by SPHI_MULTIPLIER and
	 * stored in this field as an integer.
	 * 
	 * If cmode selects an sRGB mode, this field must be
	 * SPHI_SRGB_WHITE_X.
	 */
	int32_t white_x;
	
	/*
	 * The y chromaticity of the white point.
	 * 
	 * This is represented in the xyY space, with Y assumed to be 1.0
	 * for white.  The actual value is multiplied by SPHI_MULTIPLIER and
	 * stored in this field as an integer.
	 * 
	 * If cmode selects an sRGB mode, this field must be
	 * SPHI_SRGB_WHITE_Y.
	 */
	int32_t white_y;
	
	/*
	 * The x chromaticity of red.
	 * 
	 * This is represented in the xyY space, with Y such that when red,
	 * green, and blue channels are at full intensity (1.0) -- meaning
	 * white -- Y will be 1.0.  The actual value is multiplied by
	 * SPHI_MULTIPLIER and stored in this field as an integer.
	 * 
	 * If cmode selects an sRGB mode, this field must be
	 * SPHI_SRGB_RED_X.
	 */
	int32_t red_x;
	
	/*
	 * The y chromaticity of red.
	 * 
	 * This is represented in the xyY space, with Y such that when red,
	 * green, and blue channels are at full intensity (1.0) -- meaning
	 * white -- Y will be 1.0.  The actual value is multiplied by
	 * SPHI_MULTIPLIER and stored in this field as an integer.
	 * 
	 * If cmode selects an sRGB mode, this field must be
	 * SPHI_SRGB_RED_Y.
	 */
	int32_t red_y;
	
	/*
	 * The x chromaticity of green.
	 * 
	 * This is represented in the xyY space, with Y such that when red,
	 * green, and blue channels are at full intensity (1.0) -- meaning
	 * white -- Y will be 1.0.  The actual value is multiplied by
	 * SPHI_MULTIPLIER and stored in this field as an integer.
	 * 
	 * If cmode selects an sRGB mode, this field must be
	 * SPHI_SRGB_GREEN_X.
	 */
	int32_t green_x;
	
	/*
	 * The y chromaticity of green.
	 * 
	 * This is represented in the xyY space, with Y such that when red,
	 * green, and blue channels are at full intensity (1.0) -- meaning
	 * white -- Y will be 1.0.  The actual value is multiplied by
	 * SPHI_MULTIPLIER and stored in this field as an integer.
	 * 
	 * If cmode selects an sRGB mode, this field must be
	 * SPHI_SRGB_GREEN_Y.
	 */
	int32_t green_y;
	
	/*
	 * The x chromaticity of blue.
	 * 
	 * This is represented in the xyY space, with Y such that when red,
	 * green, and blue channels are at full intensity (1.0) -- meaning
	 * white -- Y will be 1.0.  The actual value is multiplied by
	 * SPHI_MULTIPLIER and stored in this field as an integer.
	 * 
	 * If cmode selects an sRGB mode, this field must be
	 * SPHI_SRGB_BLUE_X.
	 */
	int32_t blue_x;
	
	/*
	 * The y chromaticity of blue.
	 * 
	 * This is represented in the xyY space, with Y such that when red,
	 * green, and blue channels are at full intensity (1.0) -- meaning
	 * white -- Y will be 1.0.  The actual value is multiplied by
	 * SPHI_MULTIPLIER and stored in this field as an integer.
	 * 
	 * If cmode selects an sRGB mode, this field must be
	 * SPHI_SRGB_BLUE_Y.
	 */
	int32_t blue_y;
	
	/*
	 * True if pixel aspect fields (see below) are in absolute units,
	 * false if in relative units.
	 * 
	 * If in absolute units, the unit is pixels per meter.  If relative,
	 * then only the ratio is significant.
	 */
	bool absolute;
	
	/*
	 * Horizontal pixel aspect ratio.
	 * 
	 * This must be greater than zero.  If the absolute flag is set, the
	 * unit is pixels per meter.  Else, only the ratio between pxas_h
	 * and pxas_v is relevant.
	 * 
	 * Note that this is the aspect ratio of individual pixels, not of
	 * the entire image.  If pxas_h and pxas_v are equal, then the
	 * pixels will be square.
	 */
	int32_t pxas_h;
	
	/*
	 * Vertical pixel aspect ratio.
	 * 
	 * This must be greater than zero.  If the absolute flag is set, the
	 * unit is pixels per meter.  Else, only the ratio between pxas_h
	 * and pxas_v is relevant.
	 * 
	 * Note that this is the aspect ratio of individual pixels, not of
	 * the entire image.  If pxas_h and pxas_v are equal, then the
	 * pixels will be square.
	 */
	int32_t pxas_v;
	
} SPHI_METAINFO;

/*
 * Check whether a provided metadata information structure is valid.
 * 
 * If true is returned, then all the constraints have been satisfied.
 * If false is returned, then the structure has invalid field values, or
 * a particular combination of field values is not allowed.
 * 
 * See the structure documentation for the details of the fields and
 * their constraints.
 * 
 * Parameters:
 * 
 *   pmi - pointer to the metadata information structure
 * 
 * Return:
 * 
 *   true if valid, false if invalid
 * 
 * Faults:
 * 
 *   - If pmi is NULL
 */
bool sphi_validInfo(const SPHI_METAINFO *pmi);

/*
 * Initialize a provided metadata information structure with a sensible
 * configuration.
 * 
 * If the structure is already initialized, all its current values are
 * reset by this function.
 * 
 * Although not required, a good strategy is to initialize a metadata
 * information structure with this function, and then only adjust what's
 * not covered by the predefined configuration.
 * 
 * predef must be one of the SPHI_PREDEF constants.  width and height
 * must both be in range one up to and including SPHI_MAXDIM.
 * 
 * The metadata information structure will be valid according to
 * sphi_validInfo after calling this function.
 * 
 * Parameters:
 * 
 *   pmi - pointer to the metadata information structure to initialize
 * 
 *   predef - one of the predefined configuration constants
 * 
 *   width - the width of the image in pixels
 * 
 *   height - the height of the image in pixels
 * 
 * Faults:
 * 
 *   - If pmi is NULL
 * 
 *   - If predef is not a recognized constant
 * 
 *   - If width is not in range [1, SPHI_MAXDIM]
 * 
 *   - If height is not in range [1, SPHI_MAXDIM]
 */
void sphi_initInfo(
		SPHI_METAINFO * pmi   ,
		int32_t         predef,
		int32_t         width ,
		int32_t         height );

#endif
