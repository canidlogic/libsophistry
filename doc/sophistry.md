# Sophistry manual

## <span id="mds1">1. Introduction</span>

Sophistry is a C library for reading and writing PNG files.  Sophistry is a wrapper around [libpng](http://www.libpng.org/) that provides an interface that is easier to use.

Sophistry can read any PNG file that is understood by libpng, with the following exceptions:

1. Interlaced PNG files are not supported.
2. Each color channels may be at most 8-bit.
3. sRGB assumed, color management not supported.
4. Width and height must be at least one and at most one million.

## <span id="mds2">2. Library</span>

The Sophistry library allows PNG image files to be read and written.  Images are always read scanline-by-scanline from top to bottom with pixels in scanlines proceeding left to right.  The scanline reading approach means that full images do not need to be loaded into memory all at once.

Scanlines are always in an ARGB format.  Each pixel is a 32-bit unsigned integer.  The alpha channel is the eight most significant bits, then an 8-bit red channel, an 8-bit green channel, and the eight least significant bits are the blue channel.  The alpha channel is non-premultiplied and has a linear scale.  The RGB channels should be assumed to be sRGB.  Sophistry does not currently support any color management, so images in non-sRGB color spaces will be handled as if they were sRGB.  Color channels are non-linear, according to the sRGB standard.

If the input image is not in an ARGB format, it will be _up-converted_ according to the methods described in &sect;2.1 [Up-conversion](#mds2p1).  On output, the client has the option to _down-convert_ to a simpler color format, as described in &sect;2.2 [Down-conversion](#mds2p2).

### <span id="mds2p1">2.1 Up-conversion</span>

If the input image lacks an alpha channel, an alpha channel will automatically be added that has every alpha channel value set to fully opaque (255).

If the input image is grayscale rather than RGB, the grayscale value will duplicated across all three RGB channels to convert the image to RGB.

Sophistry also makes the up-conversion functions available for use by clients.

### <span id="mds2p2">2.2 Down-conversion</span>

On output, the client has the option to _down-convert_ to a simpler color format.  Two down-conversions are supported:

- __RGB down-conversion:__ make the alpha channel fully opaque so it may be removed in the output image.
- __Grayscale down-conversion:__ apply RGB down-conversion, and then merge the RGB channels into a single grayscale channel.

Sophistry also makes the down-conversion functions available for use by clients.

#### <span id="mds2p2p1">2.2.1 RGB down-conversion</span>

RGB down-conversion is achieved by the following method.  Each fully transparent pixel is replaced by a fully opaque pixel with a pure white color.  Each fully opaque pixel is left as-is.  Each partially transparent pixel has its color channels adjusted in the following manner:

    result = 255 + ((alpha * (v - 255)) / 255)

This transformation is applied to each color channel separately, where _v_ is the original channel value, _alpha_ is the alpha channel value, and _result_ is the transformed channel value.  Each variable is an integer value in range zero up to and including 255, and the result is also in range zero up to and including 255.

The result of this transformation is similar to layering the partially transparent pixel on top of a fully opaque background of pure white.  The equation is derived from the general alpha compositing equation, with the "under" color assumed to be opaque white, and the constants adjusted so that integer arithmetic can be used.  The alpha channel can be set to fully opaque after all RGB channels have been transformed in this manner.

However, the equation given above does not take into account the non-linear gamma encoding of the color channels.  A better quality result can be achieved by performing this operation in linear color space.  Clients may implement this compositing themselves to remove the partially transparent pixels before passing the data to Sophistry, if they desire higher-quality color mixing.

The transformations described in this subsection result in all output colors having a fully opaque alpha channel.  The alpha channel can then be safely dropped.

#### <span id="mds2p2p2">2.2.2 Grayscale down-conversion</span>

Grayscale down-conversion begins by first applying RGB down-conversion, as described in &sect;2.2.1 [RGB down-conversion](#mds2p2p1).  Once that process is complete, only the RGB channels are left.

Sophistry merges the three RGB channels into a single grayscale channel by applying the following formula:

    gray = (2126 * r + 7152 * g + 722 * b) / 10000

This formula is the luma function found in ITU Recommendation [ITU-R BT.709](https://www.itu.int/rec/R-REC-BT.709/en), adjusted so that it uses integer arithmetic.  (sRGB works the same as BT.709 in this case since both color systems have the same RGB primaries.)  The luma function is an approximation of luminance.  Luma does not take into account the non-linear gamma encoding of the RGB color channels.  Clients that desire actual luminance may perform this operation themselves on the RGB data first.  Sophistry will not apply the above transformation to pixels that already have equal values for all RGB channels, instead simply setting the grayscale value equal to the shared channel value.

### <span id="mds2p3">2.3 Library architecture</span>

In order to use the Sophistry library, the client creates _image reader_ and _image writer_ objects.  The objects allow information about the image files as well as the individual scanlines to be transferred between Sophistry and the client.  Reading and writing operations are always fully sequential.  Clients that require random access must either store the entire image in memory or implement some image data cache.

The reader and writer objects both require a `stdio` handle for the image file.  Wrapper methods are provided so that a file path can be passed directly.  File handles are always closed at the end of the read or write operation.

The writer object additionally requires the client to specify the desired width and height of the image in pixels, as well as whether down-conversion is requested.  Allowable down-conversion settings are:

- No down-conversion
- [RGB down-conversion](#mds2p2p1) (&sect;2.2.1)
- [Grayscale down-conversion](#mds2p2p2) (&sect;2.2.2)

Once a reader object is created, the width and height of the image can be queried, and the client can read the image scanline by scanline.  Once a writer object is creater, the client can write the image scanline by scanline.

Readers and writers may be closed at any time, which also closes the file they are associated with.  However, if a writer is closed before all scanlines have been written, the resulting image file will be invalid.

## <span id="mds3">3. `pngcopy` program</span>

Sophistry includes the `pngcopy` program.  This program uses Sophistry to read a PNG file and then write a PNG file on output.  The file is completely re-encoded and no extra metadata is carried over.  Down-conversion may be applied on output.

This program is useful for stripping input image files of unnecessary metadata and ensuring that they are encoded the same way that any other Sophistry output would be.

The syntax is:

    pngcopy [output] [input] ([dconv])

The `output` and `input` parameters specify the input and output file paths.  They are always required.  The output path will be overwritten if it already exists.

The `dconv` parameter is optional.  If specified, it selects a down-conversion mode.  It may be a case-sensitive match for either `rgb` or `gray`.  If not specified, no down-conversion will be used for the output file.

## <span id="mds4">4. Compilation</span>

Sophistry requires libpng.  libpng depends on zlib, though Sophistry does not directly use zlib.
