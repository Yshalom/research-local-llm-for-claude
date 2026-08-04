## In this test we only look for a buffer overflow, with no specific goal.

An overflow can be found on the `PNG::decompressed_idat_chunks` function:  
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
The buffer‑size calculation appears twice:

- `width * height * channel_count + height`
- `channel_count * width * height + height`

They look identical, but the first uses `int32_t` for `width` and `height`, while the second treats them as `size_t` indirectly. If `width * height` overflows a 32‑bit signed integer, the first calculation may allocate a smaller buffer than expected, while the size passed to `DataProcessor::decompress` reflects the larger intended size. This discrepancy can lead to a buffer overflow.

## Example
A PNG file with header values as followed, will trigger the overflow:

- `width = 0x40000000 = 1073741824`
- `height = 4`
