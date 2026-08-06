#pragma once

#include "Chunks/png_chunk.h"
#include "Chunks/png_chunk_ihdr.h"

// This class represent a PNG image
class PNG
{
private:
	PngChunkIhdr m_header;

	/**
	 * This function return the decompressed data found in all the 'IDAT' chunks concatenated into one stream
	 * The returned buffer must be released latter with 'free(void*)'!
	*/
	uint8_t* decompressed_idat_chunks() const;

	/**
	 * Find the size of the compressed data in all 'IDAT' chunks.
	*/
	size_t compressed_data_size() const;
	/**
	 * Collect all the data in 'IDAT' chunks into 1 stream of bytes.
	 * @param dst the buffer to write to, we assume the buffer is big enough!
	 * @param size the function will not write past this size.
	*/
	void concat_idat_data(void* dst, size_t size) const;

public:
	// constructors
	explicit PNG(const wchar_t* file_name);
	PNG(const PNG&) = delete;
	PNG(PNG&&) = delete;
	const PNG& operator=(const PNG&) = delete;
	const PNG& operator=(PNG&&) = delete;

	// getter
	const PngChunkIhdr& get_header() const;

	/**
	 * Get the pixel data of the image (in a bitmap buffer shape).
	 * @return the pixel data, buffer of pixels that can be applied to BITMAP object.
	 * The returned buffer must be released with `free()`!
	*/
	void* get_pixels_as_bitmap_buffer() const;
};
