# include "png_chunk.h"

uint32_t PngChunk::get_length() const
{
	return m_length;
}

PngChunk* PngChunk::get_next()
{
	return m_next;
}

const PngChunk* PngChunk::get_cnext() const
{
	return m_next;
}

void PngChunk::set_next(PngChunk* next)
{
	if (m_next)
		delete(m_next);
	m_next = next;
}

PngChunk::~PngChunk()
{
	if (m_next)
		delete(m_next);
}
