#include "data_processor.h"
#include <zlib.h>

static constexpr unsigned int CHUNK_SIZE = 0x0800000; // 8MB

inline unsigned int min(size_t a, unsigned int b) {
    if (a < b)
        return a;
    return b;
}

int64_t DataProcessor::decompress(void* dst_buffer, size_t dst_max_size, void* src_buffer, size_t src_size)
{
    if (!dst_buffer || !src_buffer || dst_max_size == 0 || src_size == 0) {
        return -1;
    }

    z_stream zstream;
    zstream.zalloc = Z_NULL;
    zstream.zfree = Z_NULL;
    zstream.opaque = Z_NULL;

    // Initialize inflation
    if (inflateInit(&zstream) != Z_OK) {
        return -2;
    }

    // -----------------------------------------------------------------------
    //   Pointers that walk through the input / output buffers.
    // -----------------------------------------------------------------------
    Bytef* src_ptr = static_cast<Bytef*>(src_buffer);
    size_t src_rem = src_size; // bytes still to read

    Bytef* dst_ptr = static_cast<Bytef*>(dst_buffer);
    size_t dst_rem = dst_max_size; // bytes still available

    // -----------------------------------------------------------------------
    //   Main decompression loop – keep feeding chunks until we either
    //   consume the whole input, fill the output, or hit an error.
    // -----------------------------------------------------------------------
    while (src_rem > 0 && dst_rem > 0) {
        // Determine the size of the next chunk (never larger than CHUNK_SIZE
        unsigned int in_chunk = min(src_rem, CHUNK_SIZE);
        unsigned int out_chunk = min(dst_rem, CHUNK_SIZE);

        // Plug the chunk into the zlib stream
        zstream.avail_in = in_chunk;
        zstream.next_in = src_ptr;
        zstream.avail_out = out_chunk;
        zstream.next_out = dst_ptr;

        // Perform the actual compression step
        int ret = inflate(&zstream, Z_NO_FLUSH);

        // How many bytes did we actually consume?
        unsigned int consumed_in = in_chunk - zstream.avail_in;   // from src
        unsigned int consumed_out = out_chunk - zstream.avail_out;  // written to dst

        // Move the walking pointers forward
        src_ptr += consumed_in;
        src_rem -= consumed_in;

        dst_ptr += consumed_out;
        dst_rem -= consumed_out;

        // Interpret the return code
        if (ret == Z_STREAM_END) {
            // Calculate how many bytes were actually written
            size_t bytes_written = dst_max_size - dst_rem;
            inflateEnd(&zstream);
            return bytes_written;
        }

        // Check if the error was specifically due to running out of buffer space
        // Z_BUF_ERROR often occurs when there is no more room in the output buffer
        if (ret == Z_BUF_ERROR && consumed_out == 0) {
            // No progress was made and the output buffer is full -> caller ran out of space.
            inflateEnd(&zstream);
            return -1; // Requirement: Return -1 if no more space left
        }

        if (ret != Z_OK) {
            // Any other zlib error (data error, stream error, etc.)
            inflateEnd(&zstream);
            return -2;
        }

        // If we get here we made progress and can continue with the next chunk.
    }

    // If we get here, the buffer is too small from containing the data
    return -1;
}
