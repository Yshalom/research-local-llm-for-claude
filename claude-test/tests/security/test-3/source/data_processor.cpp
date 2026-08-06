#include "data_processor.h"
#include <zlib.h>

int64_t DataProcessor::decompress(void* dst_buffer, size_t dst_max_size, void* src_buffer, size_t src_size)
{
    if (!dst_buffer || !src_buffer || dst_max_size == 0 || src_size == 0) {
        return -1;
    }

    z_stream zstream;
    zstream.zalloc = Z_NULL;
    zstream.zfree = Z_NULL;
    zstream.opaque = Z_NULL;
    zstream.avail_in = static_cast<uInt>(src_size);
    zstream.next_in = static_cast<Bytef*>(src_buffer);
    zstream.avail_out = static_cast<uInt>(dst_max_size);
    zstream.next_out = static_cast<Bytef*>(dst_buffer);

    // Initialize inflation
    // windowBits = 15 is standard for zlib format
    if (inflateInit(&zstream) != Z_OK) {
        return -2;
    }

    int ret = inflate(&zstream, Z_FINISH);

    // Check if we reached the end of the stream successfully
    if (ret == Z_STREAM_END) {
        // Calculate how many bytes were actually written
        // avail_out tracks how much space is LEFT in the buffer.
        size_t bytes_written = dst_max_size - zstream.avail_out;
        inflateEnd(&zstream);
        return bytes_written;
    }

    // Check if the error was specifically due to running out of buffer space
    // Z_BUF_ERROR often occurs when there is no more room in the output buffer
    if (ret == Z_BUF_ERROR && zstream.avail_out == 0) {
        inflateEnd(&zstream);
        return -1; // Requirement: Return -1 if no more space left
    }

    // Any other error (data corruption, etc.)
    inflateEnd(&zstream);
    return -2;
}
