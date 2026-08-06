#include "png_chunk_ihdr.h"

#include <stdexcept>
#include "../../data_processor.h"

#include "../../utils.h"

constexpr size_t CHUNK_LENGTH = 13;

PngChunkIhdr::PngChunkIhdr(std::ifstream& input, int32_t length)
{
	// test length
	m_length = length;
	if (m_length != CHUNK_LENGTH)
		throw std::runtime_error("Wrong header chunk length; the file is corrupted!");

	// read header data
		// 4-bytes
	int32_t v;
	input.read((char*)&v, sizeof(v));
	m_width = change_endian(v);
	input.read((char*)&v, sizeof(v));
	m_height = change_endian(v);
		// 1-byte
	input.read((char*)&m_bit_depth, sizeof(m_bit_depth));
	input.read((char*)&m_color_type, sizeof(m_color_type));
	input.read((char*)&m_compression_method, sizeof(m_compression_method));
	input.read((char*)&m_filter_method, sizeof(m_filter_method));
	input.read((char*)&m_interlace_method, sizeof(m_interlace_method));
		// CRC
	input.seekg(4, std::ios_base::cur);

	// Test for supported configuration
	if (m_bit_depth != 8)
		throw std::runtime_error("Lack of support: This program support only 8-bit-depth images!");
	if (m_color_type != ARGB_CODE && m_color_type != RGB_CODE)
		throw std::runtime_error("Lack of support: This program support only RGB and ARGB images!");
	if (m_compression_method != 0)
		throw std::runtime_error("Lack of support: This program doesn't support compressed images!");
	if (m_filter_method != 0)
		throw std::runtime_error("Lack of support: This program doesn't support filtered images!");
}
PngChunkIhdr::PngChunkIhdr()
	:m_width(-1)
	,m_height(-1)
	,m_bit_depth(-1)
	,m_color_type(-1)
	,m_compression_method(-1)
	,m_filter_method(-1)
	,m_interlace_method(-1)
{}

const PngChunkIhdr& PngChunkIhdr::operator=(PngChunkIhdr&& other)
{
	m_length = other.m_length;
	m_next = other.m_next;
	m_width = other.m_width;
	m_height = other.m_height;
	m_bit_depth = other.m_bit_depth;
	m_color_type = other.m_color_type;
	m_compression_method = other.m_compression_method;
	m_filter_method = other.m_filter_method;
	m_interlace_method = other.m_interlace_method;

	return *this;
}

const char* PngChunkIhdr::get_type() const
{
	return "IHDR";
}

int32_t PngChunkIhdr::get_width() const
{
	return m_width;
}

int32_t PngChunkIhdr::get_height() const
{
	return m_height;
}

int8_t PngChunkIhdr::get_color_type() const
{
	return m_color_type;
}

int8_t PngChunkIhdr::get_channel_count() const
{
	return m_color_type == PngChunkIhdr::ARGB_CODE ? 4 : 3; // Assume ARGB/RGB mode only
}
