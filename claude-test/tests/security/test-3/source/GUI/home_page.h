#pragma once

#include <windows.h>
#include <memory>

#include "image_widget.h"

// There is one HomePage existing at a time!
class HomePage {
public:
    HomePage();

    ~HomePage();
private:
    HWND m_hWnd; // Window handle
    std::unique_ptr<ImageWidget> m_image;

    // Static window procedure
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    // set image
    bool set_image(PCWSTR path);
    
    // Menu handlers
    void on_menu_clicked(WORD command_id);

    // Files dragged handler
    void on_files_dragged(HDROP hDrop);

    // WNDCLASS registering
    static void RegisterWindowsClass();
    static void UnregisterWindowsClass();
};
