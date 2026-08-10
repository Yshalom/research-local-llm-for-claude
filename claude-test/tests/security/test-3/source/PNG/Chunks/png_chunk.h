#pragma once

#include <cinttypes>

// This is abstract class for chunk of data, in png archive
class PngChunk
{
protected:
	uint32_t	m_length;

	PngChunk* m_next = nullptr;

public:
	// getter
	virtual const char* get_type() const = 0;
	uint32_t get_length() const;
	PngChunk* get_next();
	const PngChunk* get_cnext() const;
	void set_next(PngChunk*);

	// destructor
	virtual ~PngChunk();
};
