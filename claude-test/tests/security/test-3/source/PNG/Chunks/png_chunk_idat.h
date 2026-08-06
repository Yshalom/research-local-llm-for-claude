#pragma once

#include "png_chunk.h"

#include <fstream>

class PngChunkIdat : public PngChunk
{
private:
	void* m_data;
	bool m_auto_free;

public:
	PngChunkIdat(std::ifstream& input, int32_t length);
	~PngChunkIdat();

	PngChunkIdat(const PngChunkIdat&) = delete;
	PngChunkIdat(PngChunkIdat&&) = delete;
	PngChunkIdat& operator=(const PngChunkIdat&) = delete;
	PngChunkIdat& operator=(PngChunkIdat&&) = delete;

	// getters
	const char* get_type() const override;
	const void* get_data_cptr() const;

	// setters
	/**
	 * @brief This function replace the data buffer in the IDAT chunk.
	 * Usage for this function could be concatenate all the IDAT data, and distribute
	 * the concatenated buffer into the IDAT chunks, or replace the pixels in the image.
	 * Remember that the IDAT chunks contain zlib-compressed data.
	 * @param new_data this buffer will be set as the new data
	 * @param the length of the buffer (also the length of the IDAT chunk)
	 * @param auto_free controle whether this class is responsible to free the buffer.
	*/
	void set_data(void* new_data, size_t length, bool auto_free = true);
};
