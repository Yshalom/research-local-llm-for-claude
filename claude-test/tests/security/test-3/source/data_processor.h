#pragma once

#include <cinttypes>

/**
 * @brief A utility class to handle zlib functions.
 *
 * Dependency: zlib
 */
static class DataProcessor {
public:
    /**
     * @brief Decompresses data from src_buffer into dst_buffer.
     *
     * @param dst_buffer Pointer to the destination buffer.
     * @param dst_max_size The maximum capacity of the destination buffer.
     * @param src_buffer Pointer to the compressed source buffer.
     * @param src_size The size of the compressed source data.
     *
     * @return how many bytes were written, upon success.
     * @return -1 if the destination buffer is too small (dst_max_size reached).
     * @return -2 if a zlib internal error occurs (e.g., corrupted data).
     */
    static int64_t decompress(void* dst_buffer, size_t dst_max_size, void* src_buffer, size_t src_size);
};
