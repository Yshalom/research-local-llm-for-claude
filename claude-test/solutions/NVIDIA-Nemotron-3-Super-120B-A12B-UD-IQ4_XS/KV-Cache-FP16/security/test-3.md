# Security Analysis of PNG Decoder Source Code

## What you found?
The source code contains multiple security vulnerabilities, primarily integer overflows that can lead to buffer undersizing and out-of-bounds memory reads. Specifically:
1. Integer overflow in memory size calculations for allocating buffers based on image dimensions (width, height) and channel count.
2. Out-of-bounds read when processing greyscale PNG images due to incorrect assumption about pixel format.
3. Potential mismatch in Unicode/DragQueryFile usage causing incorrect file path extraction under certain build configurations.

## Which line(s) of the original source are problematic?
- **Integer overflow in `decompressed_idat_chunks()`**: Line 121 in `PNG/png.cpp`: `malloc(width * height * channel_count + height);`
- **Integer overflow in bitmap buffer allocation**: Line 230 in `PNG/png.cpp`: `malloc(bitmap_row_length * height);` where `bitmap_row_length = (width * 3 + 3) & ~3;` (line 215).
- **Integer overflow in `filter_decompressed_data()`**: Line 169 in `PNG/png.cpp`: `size_t row_length = width * channel_count + 1;`.
- **Integer overflow in IDAT chunk data allocation**: Line 115 in `PNG/png.cpp`: `malloc(compressed_size);` where `compressed_size` is sum of chunk lengths.
- **Out-of-bounds read for greyscale images**: Lines 239-252 in `PNG/png.cpp`: accesses `src[1]`, `src[2]`, `src[3]` when `channel_count == 1`, reading beyond the allocated buffer.
- **Potential Unicode mismatch in drag‑and‑drop handling**: Lines 165 and 177 in `GUI/home_page.cpp`: `DragQueryFileW` used for count but `DragQueryFile` (without explicit W/A) used for path extraction; if `UNICODE` is not defined, this may cause incorrect parsing.

## Recommendation for a fix
- **Prevent integer overflow**: Before allocating memory, check for overflow using safe multiplication (e.g., if (width > SIZE_MAX / (channel_count * height)) handle error). Use libraries like `safeint` or implement checks for each multiplication/addition.
- **Validate image dimensions**: Reject unreasonably large width/height values that could cause excessive memory allocation (mitigates DoS).
- **Fix greyscale handling**: In `get_pixels_as_bitmap_buffer()`, properly handle `channel_count == 1` (greyscale) by converting grey to RGB (replicate grey to R,G,B) or support greyscale bitmaps. Ensure buffer accesses stay within bounds.
- **Use consistent Unicode functions**: Replace `DragQueryFile` with `DragQueryFileW` (or `DragQueryFileA` depending on build) to match the count function, or wrap with `_T` macros.
- **Consider using size‑checked allocation functions**: Use `_malloc_dbg` or custom wrappers that verify allocation size against expected maximum based on PNG limits (width,height ≤ 2^31‑1 per spec).
- **Add validation of compressed data size**: Ensure `compressed_size` does not exceed a reasonable threshold before allocation.