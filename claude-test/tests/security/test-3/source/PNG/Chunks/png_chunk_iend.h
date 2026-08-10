#pragma once

#include "png_chunk.h"

#include <fstream>

class PngChunkIend : public PngChunk
{
private:

public:
	PngChunkIend(std::ifstream& input, uint32_t length);

	// getters
	const char* get_type() const override;
};