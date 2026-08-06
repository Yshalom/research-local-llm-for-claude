### In this test we are only interested in locating a buffer overflow; no particular goal.

The overflow occurs in the member function `PNG::decompressed_idat_chunks()`

**test-3/PNG/png.cpp [Lines 107-130]**  
```cpp
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
```

**Key Observation**  
The buffer‑size calculation is performed twice:
- `width * height * channel_count + height`
- `channel_count * width * height + height`

Although the two expressions look identical, the first uses `int32_t` for `width` and `height`, while the second treats them as `size_t` indirectly. This difference enables the overflow. If `width * height` overflows a 32‑bit signed integer, the first calculation yields a much smaller value, causing the allocation to be undersized.  
This discrepancy can lead to a heap‑buffer overflow when the decompressor writes more data than the allocated buffer can hold.

## Demo
A PNG file with the following header values will trigger the overflow:
- width = 0x40000000 = 1073741824
- height = 4
