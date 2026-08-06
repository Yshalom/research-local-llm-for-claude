#include "png.h"

#include "../utils.h"
#include "../data_processor.h"

#include "Chunks/png_chunk_ancillary.h"
#include "Chunks/png_chunk_idat.h"
#include "Chunks/png_chunk_iend.h"

#include <fstream>

constexpr char PNG_SIGNATURE[] { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

void read_length_type(std::ifstream& input_file, int32_t& length, char type[4])
{
	input_file.read((char*)&length, sizeof(int32_t));
	length = change_endian(length);
	input_file.read(type, sizeof(char[4]));
}

PNG::PNG(const wchar_t* file_name)
{
	// Open the png file
	std::ifstream input_file = std::ifstream(file_name, std::ios_base::binary);
	if (!input_file.is_open())
		throw std::runtime_error("Couldn't open the file");

	// Test the file signature
	char png_signature[sizeof(PNG_SIGNATURE)];
	input_file.read(png_signature, sizeof(png_signature));
	if (memcmp(png_signature, PNG_SIGNATURE, sizeof(PNG_SIGNATURE)) != 0)
		throw std::runtime_error("Wrong file signature; the file is corrupted!");

	// ------------------- Read chunks -------------------
	int32_t length;
	char type[4];

	// Read the header
	read_length_type(input_file, length, type);
	if (_strnicmp("IHDR", type, 4) != 0)
		throw std::runtime_error("Couldn't find PNG header (IHDR); the file is corrupted!");
	m_header = PngChunkIhdr(input_file, length);

	PngChunk* chunk = &m_header;
	while (true) {
		if (input_file.eof())
			throw std::runtime_error("End-Of-File; couldn't find IEND header; the file is corrupted!");

		read_length_type(input_file, length, type);
		
		if (_strnicmp("IDAT", type, 4) == 0)
			chunk->set_next(new PngChunkIdat(input_file, length));
		else if (_strnicmp("IEND", type, 4) == 0)
		{
			chunk->set_next(new PngChunkIend(input_file, length));
			break;
		}
		else if (_strnicmp("IHDR", type, 4) == 0)
			throw std::runtime_error("Found 2 or more PNG headers (IHDR); the file is corrupted!");
		else
			chunk->set_next(new PngChunkAncillary(input_file, length, type));
		
		// finish with the current chunks, move to the next one
		chunk = chunk->get_next();
	}
}

const PngChunkIhdr& PNG::get_header() const
{
	return m_header;
}

size_t PNG::compressed_data_size() const
{
	size_t size = 0;
	// Find the size of the buffer
	for (const PngChunk* i = m_header.get_cnext(); i != nullptr; i = i->get_cnext())
	{
		// Find all the 'IDAT' chunks
		const char* type = i->get_type();
		if (_strnicmp("IDAT", type, 4) == 0)
			size += i->get_length();
	}
	return size;
}

void PNG::concat_idat_data(void* dst, size_t size) const
{
	// Concatenate all the data into the buffer
	char* copying_ptr = (char*)dst;
	for (const PngChunk* i = m_header.get_cnext(); i != nullptr; i = i->get_cnext())
	{
		// Find all the 'IDAT' chunks
		const char* type = i->get_type();
		if (_strnicmp("IDAT", type, 4) == 0)
		{
			errno_t res = memcpy_s(copying_ptr, size, static_cast<const PngChunkIdat*>(i)->get_data_cptr(), i->get_length());
			if (res)
				throw std::runtime_error("There was error collecting the data in the 'IDAT' chunks");
			// move the pointer, and reflect the size that left to write.
			copying_ptr += i->get_length();
			size -= i->get_length();
		}
	}
}

uint8_t* PNG::decompressed_idat_chunks() const
{
	int32_t width = m_header.get_width(),
		height = m_header.get_height();
	size_t channel_count = m_header.get_channel_count();

	// Concatenate all the data in the 'IDAT' chunks
	size_t compressed_size = compressed_data_size();
	void* compressed_data = malloc(compressed_size);
	if (!compressed_data)
		throw std::runtime_error("There was a problem while allocating the memory!");
	concat_idat_data(compressed_data, compressed_size);

	// Decompress the data
	uint8_t* buffer = (uint8_t*)malloc(width * height * channel_count + height);
	if (!buffer)
		throw std::runtime_error("There was a problem while allocating the memory!");

	int64_t res = DataProcessor::decompress(
		buffer,										// decompressed data buffer to write to
		channel_count * width * height + height,	// size of the buffer
		compressed_data,
		compressed_size
	);
	free(compressed_data); // done using the compressed_data buffer

	if (res != channel_count * width * height + height) {
		// There was an error while decompressing
		free(buffer);
		switch (res) {
		case -1:
			throw std::runtime_error("There's more data from pixels in the image; the file is corrupted!");
		case -2:
			throw std::runtime_error("Error on the zlib decompression process");
		default:
			throw std::runtime_error("There's less data from pixels in the image; the file is corrupted!");
		}
	}

	return buffer;
}

uint8_t paeth_predictor(int16_t a, int16_t b, int16_t c)
{
	int p = a + b - c,
		pa = abs(p - a),
		pb = abs(p - b),
		pc = abs(p - c);
	if (pa <= pb && pa <= pc)
		return a;
	if (pb <= pc)
		return b;
	return c;
}

/**
 * This function get the raw decompressed data in PNG format, and apply the filter to the rows.
 * @param data The buffer contains the raw decompressed data from all 'IDAT' chunks,
 * the filtered data will be written to the input 'data' buffer.
*/
void filter_decompressed_data(uint8_t* data, size_t width, size_t height, size_t channel_count)
{
	size_t row_length = width * channel_count + 1;

	for (size_t i = 0; i < height; i++) {
		uint8_t filter = *data;
		*data = 0; // zero the filter byte
		data++;
		for (size_t j = 0; j < width; j++) {
			for (size_t k = 0; k < channel_count; k++) {
				switch (filter)
				{
				case 0:
					break;
				case 1:
					*data += j ? *(data - channel_count) : 0; // += left-pixel
					break;
				case 2:
					*data += i ? *(data - row_length) : 0; // += up-pixel
					break;
				case 3:
					*data += ((j ? *(data - channel_count) : 0) + (i ? *(data - row_length) : 0)) / 2; // += (left-pixel + up-pixel)/2
					break;
				case 4:
					// += paeth_predictor(left-pixel, up-pixel, left-up-pixel)
					*data += paeth_predictor(
						j ? *(data - channel_count) : 0,					// left-pixel
						i ? *(data - row_length) : 0,						// up-pixel
						(j && i) ? *(data - channel_count - row_length) : 0	// left-up-pixel
					);
					break;
				default:
					throw std::runtime_error("Unknown filter byte; the file is corrupted!");
				}
				data++;
			}
		}
	}
}

void* PNG::get_pixels_as_bitmap_buffer() const
{
	size_t width = m_header.get_width();
	size_t height = m_header.get_height();
	size_t channel_count = m_header.get_channel_count();

	// When `width * 3` is not a multiplication of 4, the bitmap buffer has a stride.
	// the stride is padding the end of the line to make it a multiplication of 4.
	size_t bitmap_row_length = (width * 3 + 3) & ~3;

	// get the filtered decompressed data from all 'IDAT' chunks
	uint8_t* data = decompressed_idat_chunks();
	try {
		filter_decompressed_data(data, width, height, channel_count);
	}
	catch (const std::runtime_error& e) {
		free(data); // make sure the data buffer is freed, than rethrow the exception as before.
		throw e;
	}

	// --------------------------------------------------------------------------------------------------------------------
	// ------------------------------ copy & layout the decompressed data into bitmap buffer ------------------------------
	// --------------------------------------------------------------------------------------------------------------------
	uint8_t* bitmap_buffer = (uint8_t*)malloc(bitmap_row_length * height);
	if (!bitmap_buffer)
		throw std::runtime_error("There was a problem while allocating the memory!");
	const uint8_t* src = data;
	for (size_t i = 0; i < height; i++) {
		uint8_t* dst = bitmap_buffer + bitmap_row_length * (height - i - 1); // The bitmap buffer is written bottom to top
		src++; // get rid of the filter byte
		for (size_t j = 0; j < width; j++) {
			// Here we assume pictures are either 'RGB' or 'RGBA' modes only!
			if (channel_count == 3 || src[3] == 255) {
				// RGB -> BGR
				dst[2] = src[0];
				dst[1] = src[1];
				dst[0] = src[2];
			}
			else /* channel_count == 4 and alpha != 255 */ {
				// RGBA -> BGR
				int32_t alpha = src[3],
					background = 225 + 30 * ((i >> 5 & 1) ^ (j >> 5 & 1));
				dst[2] = (alpha * src[0] + (255 - alpha) * background) / 255;
				dst[1] = (alpha * src[1] + (255 - alpha) * background) / 255;
				dst[0] = (alpha * src[2] + (255 - alpha) * background) / 255;
			}

			// move to the next pixel
			dst += 3;
			src += channel_count;
		}
	}

	// release the raw decompressed buffer
	free(data);

	return bitmap_buffer;
}
