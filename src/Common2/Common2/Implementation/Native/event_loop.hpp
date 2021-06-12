#pragma once

#if _WIN32

void app_native_loop_initialize()
{
    WNDCLASS wc = {NULL};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(GetModuleHandle(NULL), IDC_ARROW);

    RegisterClass(&wc);
};

void app_native_loop_finalize(){
    // Nothing for windows
};

void app_native_loop_pool_events()
{
    MSG msg = {0};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    };
};

#endif
