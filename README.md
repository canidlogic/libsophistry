# libsophistry
This library defines a 32-bit image and audio sample format.  Functions are provided to pack and unpack channels from the 32-bit samples, and convert between various component formats.

The 32-bit image sample format consists of an alpha (A), red (R), green (G), and blue (B) channel packed into a 32-bit unsigned integer.  Each color channel is an unsigned 8-bit integer, in range zero to 255.  The order of channels is ARGB from most significant byte to least significant byte within the 32-bit unsigned integer.

The 32-bit audio sample format consists of a left channel and a right channel packed into a 32-bit unsigned integer.  Each channel is stored as an unsigned 16-bit integer, in range zero to 65535.  However, the channel values are converted to signed range -32768 to +32767 when the sample is unpacked.  The left channel is stored in the most significant 16 bits, while the right channel is stored in the least significant 16 bits.

libsophistry also provides functions to convert between sample component formats.  The supported component formats are:

Format           | Type    | Minimum value | Maximum value
-----------------|---------|---------------|--------------
Unsigned integer | Integer | 0             | +MAXVAL
Signed integer   | Integer | -MAXVAL-1     | +MAXVAL
Unsigned normal  | Float   | 0.0           | +1.0
Signed normal    | Float   | -1.0          | +1.0

MAXVAL can have any value from one up to 65535.  libsophistry also supports converting between integer formats with different MAXVAL values.

libsophistry does not specify the exact meaning of the RGB channels, the interpretation of the alpha channel, nor the exact meaning of the audio sample values.  libsophistry limits itself to only the low-level packing and unpacking operations, as well as the basic component format conversions.

## Compilation
If <stdint.h> is not available or if not all the standard definitions are present, then there may be compilation errors related to the basic type definitions.  If this is the case, then adjust the sophistry.h header type declarations to remove the <stdint.h> header if the compiler does not support it, and rewrite the declarations to use the types appropriate for the particular compiler.
