#include "png_chunk_ancillary.h"

PngChunkAncillary::PngChunkAncillary(std::ifstream& input, int32_t length, const char type[4])
{
	// length
	m_length = length;
	// type
	memcpy_s(m_type, sizeof(m_type), type, sizeof(m_type));
	// data
	input.seekg(length, std::ios_base::cur);
	// CRC
	input.seekg(4, std::ios_base::cur);
}

const char* PngChunkAncillary::get_type() const
{
	return m_type;
}
