#pragma once

#if _WIN32
inline LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
    {
        AppWindowCloseEvent l_windowEvent = AppWindowCloseEvent{AppEventType::WINDOW_CLOSE, window_native{(int8*)hwnd}};
        AppEvent_Broadcast(window_native{(int8*)hwnd}, &l_windowEvent.type);
        return 0;
    }
    case WM_SIZE:
    {
        uint32 l_width = LOWORD(lParam);
        uint32 l_height = HIWORD(lParam);

        AppWindowResizeEvent l_windowResizeEvent = AppWindowResizeEvent{AppEventType::WINDOW_RESIZE, window_native{(int8*)hwnd}, l_width, l_height};
        AppEvent_Broadcast(window_native{(int8*)hwnd}, &l_windowResizeEvent.type);
        return 0;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
};
#endif