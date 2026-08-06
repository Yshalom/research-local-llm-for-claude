#pragma once

#include "png_chunk.h"

#include <fstream>

class PngChunkIhdr : public PngChunk
{
private:
	int32_t m_width, m_height;
			
	int8_t	m_bit_depth,
			m_color_type,
			m_compression_method,
			m_filter_method,
			m_interlace_method;

public:
	static constexpr int8_t ARGB_CODE = 6, RGB_CODE = 2;

	PngChunkIhdr(std::ifstream& input, int32_t length);
	PngChunkIhdr();

	const PngChunkIhdr& operator=(PngChunkIhdr&&);

	// getters
	const char* get_type() const override;
	int32_t get_width() const;
	int32_t get_height() const;
	int8_t get_color_type() const;
	int8_t get_channel_count() const;
};
