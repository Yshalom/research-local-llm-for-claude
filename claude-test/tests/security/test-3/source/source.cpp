#include <windows.h>
#include <stdexcept>

#include "GUI/home_page.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    // Set the global hInstance

    // Create the home page window
    HomePage homePage;

    // Message loop (running on main thread)
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        try {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        catch (const std::runtime_error& error) {
            MessageBoxA(nullptr, error.what(), "Error - HomePage", MB_OK | MB_ICONERROR);
        }
        catch (...) {
            MessageBoxW(nullptr, L"Fatal Error", L"Error", MB_OK | MB_ICONERROR);
            return 1;
        }
    }
    return 0;
}
