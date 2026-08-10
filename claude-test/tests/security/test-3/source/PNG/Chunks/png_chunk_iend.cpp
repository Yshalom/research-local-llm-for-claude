#include "png_chunk_iend.h"

#include <stdexcept>

constexpr size_t CHUNK_LENGTH = 0;

PngChunkIend::PngChunkIend(std::ifstream& input, uint32_t length)
{
	// test length
	m_length = length;
	if (m_length != CHUNK_LENGTH)
		throw std::runtime_error("Wrong end chunk length!");

	if (input.eof())
		throw std::runtime_error("End-Of-File; couldn't find IEND header; the file is corrupted!");

	// CRC
	input.seekg(4, std::ios_base::cur);
}

const char* PngChunkIend::get_type() const
{
	return "IEND";
}
