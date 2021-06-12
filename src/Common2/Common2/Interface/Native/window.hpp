#pragma once

struct window_native{
    int8* ptr;
};

window_native window_native_create_window(const uint32 p_client_width, const uint32 p_client_height, const Slice<int8>& p_display_name);
void window_native_display_window(const window_native p_window);
void window_native_destroy_window(const window_native p_window);
void window_native_resize_window(const window_native p_window, const uint32 p_client_width, const uint32 p_client_height);
void window_native_get_window_client_dimensions(const window_native p_window_handle, uint32* out_client_width, uint32* out_client_height);
void window_native_simulate_resize_appevent(const window_native p_window, const uint32 p_client_width, const uint32 p_client_height);
void window_native_simulate_close_appevent(const window_native p_window);