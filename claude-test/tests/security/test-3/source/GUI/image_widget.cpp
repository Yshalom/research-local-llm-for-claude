#include <stdexcept>

#include "image_widget.h"

#include "../PNG/png.h"
#include "../PNG/Chunks/png_chunk_ihdr.h"

HBITMAP CreateImageBitmap(const PNG& png)
{
	size_t width = png.get_header().get_width(),
		height = png.get_header().get_height();
	// When `width * 3` is not a multiplication of 4, the bitmap buffer has a stride.
	// the stride is padding the end of the line to make it a multiplication of 4.
	size_t buffer_length = ((width * 3 + 3) & ~3) * height;

	void* pixels = png.get_pixels_as_bitmap_buffer();
	void* image;

	// config the bitmap header
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = height;
	bmi.bmiHeader.biPlanes = 1; // must be 1
	bmi.bmiHeader.biBitCount = 24; // 3 channels (RGB), each one 8-bit -> 24 bits per pixel
	bmi.bmiHeader.biCompression = BI_RGB; // RGB format

	// Create bitmap
	HBITMAP hBitmap = CreateDIBSection(
		nullptr,		// mo HDC
		&bmi,			// bitmap header config
		DIB_RGB_COLORS,
		&image,
		nullptr,
		0);
	if (!hBitmap)
		throw std::runtime_error("ImageWidget: Couldn't create bitmap");

	memcpy_s(image, buffer_length, pixels, buffer_length);
	free(pixels);

	return hBitmap;
}

HWND CreateIwWindow(HINSTANCE hInstance, HWND parent_hWnd, size_t width, size_t height)
{
	HWND hWnd = CreateWindowExW(
		0,                     // no extended style
		L"STATIC",             // class name = "STATIC"
		nullptr,               // no windows name
		WS_CHILD | WS_VISIBLE | SS_BITMAP, // style [bitmap image]
		0, 0,                // x, y inside the client area
		width, height,              // width, height
		parent_hWnd,                  // parent window
		NULL, // control ID (any value)
		hInstance,
		nullptr);

	if (!hWnd) {
		throw std::runtime_error("ImageWidget: Failed to create window");
	}

	return hWnd;
}

ImageWidget::ImageWidget(HWND parent_hWnd, PCWSTR png_path)
	:m_parent_hWnd(parent_hWnd)
{
	// Open the png file
	try {
		PNG png(png_path);
		m_hBitmap = CreateImageBitmap(png);
		m_hWnd = CreateIwWindow(GetModuleHandle(nullptr), parent_hWnd, png.get_header().get_width(), png.get_header().get_height());

		// Send the bitmap to the window
		SendMessage(m_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)m_hBitmap);
	}
	catch (const std::runtime_error& error) {
		MessageBoxA(nullptr, error.what(), "Error - PNG decoding", MB_OK | MB_ICONERROR);
		throw std::runtime_error("Couldn't construct the image");
	}
}

ImageWidget::~ImageWidget()
{
	DestroyWindow(m_hWnd);
	DeleteObject(m_hBitmap);
}
