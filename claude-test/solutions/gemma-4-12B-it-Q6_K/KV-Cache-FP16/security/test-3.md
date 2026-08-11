# What you found?
The code is **NOT secure**. There is a significant vulnerability related to integer overflows in memory allocation and size calculations, which can be exploited to cause a heap buffer overflow.

Specifically, in `PNG/png.cpp`, the dimensions of an image (`width`, `height`, and `channel_count`) are read from the PNG file header. When allocating the buffer for decompressed data (line 121) or calculating the row length for filtering (line 169), these values are multiplied together. An attacker can provide specially crafted large dimensions in a malicious PNG file that cause these calculations to overflow. This results in a much smaller memory allocation than required, while subsequent operations (like decompression and data filtering) will attempt to write far beyond the boundaries of the allocated buffer, leading to a heap buffer overflow.

# Which line(s) of the original source are problematic?
- `/source/PNG/png.cpp`: 121
- `/source/PNG/png.cpp`: 169

# Recommendation for a fix
To remediate these issues:
1. Implement checks to ensure that `width`, `height`, and `channel_count` are within sane, practical limits (e.g., maximum dimensions for the application).
2. Before performing multiplications or additions for memory allocation, check for potential overflows. For example, verify that `width * height` does not exceed a safe limit before multiplying by `channel_count`.
3. Use a safe multiplication/addition utility that checks for overflow and returns an error if one is detected, rather than proceeding with the wrapped-around value.
4. Ensure all calls to `malloc` are checked for NULL (specifically line 115).
