#include "png_chunk_idat.h"

PngChunkIdat::PngChunkIdat(std::ifstream& input, uint32_t length)
{
	// length
	m_length = length;
	// data
	m_data = malloc(length);
	m_auto_free = true;
	if (!m_data)
		throw std::runtime_error("There was a problem while allocating the memory!");
	input.read((char*)m_data, length);

	if (input.eof())
		throw std::runtime_error("End-Of-File; couldn't find IEND header; the file is corrupted!");

	// CRC
	input.seekg(4, std::ios_base::cur);
}

PngChunkIdat::~PngChunkIdat()
{
	if (m_auto_free)
		free(m_data);
	m_auto_free = false;
}

const char* PngChunkIdat::get_type() const
{
	return "IDAT";
}

const void* PngChunkIdat::get_data_cptr() const
{
	return m_data;
}

void PngChunkIdat::set_data(void* new_data, size_t length, bool auto_free)
{
	if (m_auto_free)
		free(m_data);
	m_data = new_data;
	m_length = length;
	m_auto_free = auto_free;
}
