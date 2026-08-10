#pragma once

#include "png_chunk.h"

#include <fstream>

class PngChunkAncillary : public PngChunk
{
private:
	char m_type[4];

public:
	PngChunkAncillary(std::ifstream& input, uint32_t length, const char type[4]);

	const char* get_type() const override;
};
