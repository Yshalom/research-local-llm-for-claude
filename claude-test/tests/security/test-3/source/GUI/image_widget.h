#pragma once

#include <windows.h>

class ImageWidget
{
public:
	ImageWidget(HWND parent_hWnd, PCWSTR png_path);

	~ImageWidget();

private:
	HWND m_hWnd;
	HWND m_parent_hWnd;
	HBITMAP m_hBitmap;
};
