#include <stdexcept>

#include "home_page.h"

constexpr PCWSTR INITILIZE_DIR_FOR_OPEN_FILE = L"../pictures";

// WNDCLASS registration
constexpr wchar_t HOME_PAGE_WNDCLASSW_NAME[] = L"HomePage";
void HomePage::RegisterWindowsClass()
{
    // Register class
    WNDCLASSW wc = { 0 };
    wc.lpszClassName = HOME_PAGE_WNDCLASSW_NAME;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpfnWndProc = HomePage::WndProc;
    wc.style = CS_HREDRAW | CS_VREDRAW;

    if (!RegisterClassW(&wc)) {
        throw std::runtime_error("Failed to register window class");
    }
}
void HomePage::UnregisterWindowsClass()
{
    UnregisterClassW(HOME_PAGE_WNDCLASSW_NAME, GetModuleHandle(NULL));
}

enum MenuCommandID
{
    MENU_COMMAND_FILE_OPEN = 1001,
    MENU_COMMAND_FILE_CLOSE
};


HMENU CreateHpMenu()
{
    HMENU hMenu = CreateMenu();
    HMENU hFileMenu = CreatePopupMenu();

    AppendMenuW(hFileMenu, MF_STRING, MENU_COMMAND_FILE_OPEN, L"Open");
    AppendMenuW(hFileMenu, MF_STRING, MENU_COMMAND_FILE_CLOSE, L"Close");

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"File");

    return hMenu;
}

HWND CreateHpWindow(HINSTANCE hInstance, HMENU hMenu, HomePage* _this)
{
    HWND hWnd = CreateWindowW(
        L"HomePage", // lpClassName
        L"PNG - decoder", // lpWindowName
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // dwStyle
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // x,y,w,h
        nullptr, // hWndParent
        hMenu, // hMenu
        hInstance, // hInstance
        nullptr// lpParam
    );

    // Save the HomePage class itself, for future identification
    SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)_this);

    if (!hWnd) {
        throw std::runtime_error("Failed to create window");
    }

    return hWnd;
}

#include <windows.h>
#include <string>

bool OpenPngFile(HWND hWnd, PWSTR path, size_t path_len)
{
    // Set up OPENFILENAME structure
    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;                 // Parent window (can be nullptr)
    ofn.lpstrFile = path;
    ofn.nMaxFile = path_len;       // Size in characters
    ofn.lpstrFilter = L"PNG Files (*.png)\0*.png\0";
    ofn.nFilterIndex = 1;                      // Start with first filter
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST |
        OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrInitialDir = INITILIZE_DIR_FOR_OPEN_FILE;

    // Show dialog
    if (GetOpenFileNameW(&ofn))
        return true;
    // the operation was canceled
    return false;
}

// Constructor implementation
HomePage::HomePage()
    :m_hWnd(nullptr)
{
    RegisterWindowsClass();
    HMENU hMenu = CreateHpMenu();
    m_hWnd = CreateHpWindow(GetModuleHandle(NULL), hMenu, this);

    DragAcceptFiles(m_hWnd, TRUE);
}

// Window procedure
LRESULT CALLBACK HomePage::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    // Get the HomePage class associated with the hWnd
    HomePage* _this = (HomePage*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

    // Callback switch
    switch (message) {
    case WM_COMMAND:
        if (HIWORD(wParam) == 0) { // Menu command
            _this->on_menu_clicked(LOWORD(wParam));
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_DROPFILES: 
        _this->on_files_dragged((HDROP)wParam);

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

bool HomePage::set_image(PCWSTR path)
{
    try {
        m_image.reset(new ImageWidget(m_hWnd, path));
        return true;
    }
    catch (const std::runtime_error& error) {
        MessageBoxA(nullptr, error.what(), "Error - ImageWidget", MB_OK | MB_ICONERROR);
        return false;
    }
}

// Menu handler
void HomePage::on_menu_clicked(WORD commandId) {
    switch (commandId) {
    case MENU_COMMAND_FILE_OPEN: // Open
    {
        wchar_t file_path[MAX_PATH]{ 0 };
        if (OpenPngFile(m_hWnd, file_path, MAX_PATH))
            // open the image and show with ImageWidget
            set_image(file_path);
    }
        break;

    case MENU_COMMAND_FILE_CLOSE: // Close
        DestroyWindow(m_hWnd);
        break;
    }
}

// Files dragged handler
void HomePage::on_files_dragged(HDROP hDrop)
{
    UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);

    // If there are more than 1 file or none at all
    if (count != 1)
    {
        MessageBoxW(m_hWnd, L"You need to drag exactly 1 file", L"File Drag Warning", MB_OK | MB_ICONEXCLAMATION);
        DragFinish(hDrop);          // clean up
        return;
    }

    // extract the file full path
    TCHAR file_path[MAX_PATH];
    DragQueryFile(hDrop, 0, file_path, MAX_PATH);
    DragFinish(hDrop);          // clean up
    size_t file_path_len = lstrlenW(file_path);

    // If the file is not "*.png"
    if (lstrcmpiW(file_path + file_path_len - 4, L".png") != 0)
    {
        MessageBoxW(m_hWnd, L"Currently we work only with PNG files", L"Wrong File Type", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    // open the image and show with ImageWidget
    set_image(file_path);
}

HomePage::~HomePage()
{
    // cleanup windows class
    UnregisterWindowsClass();

    // Menu is automatically destroyed because it's assigned to a window.
}
