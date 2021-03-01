#pragma once

#include "Common2/common2.hpp"
#include "GLFW/glfw3.h"

namespace v2
{

using WindowHandle = char*;

struct Window
{
    WindowHandle Handle;
    uint16 Width;
    uint16 Height;

    int8 is_closing;

    struct ResizeEvent
    {
        int8 ask;
        uint16 new_width;
        uint16 new_height;

        static ResizeEvent build_default();
    } resize_event;

    static Window build(const WindowHandle p_handle, const uint16 p_width, const uint16 p_height);

    void close();

    int8 asks_for_resize() const;
    void on_resized(const uint16 p_width, const uint16 p_height);
    void consume_resize_event();
};

Pool<Window> app_windows = Pool<Window>::allocate(0);

int8 glfw_initialized = 0;
struct GLFWWindow_to_Window
{
    GLFWwindow* glfw_window;
    Token(Window) window;
};

Vector<GLFWWindow_to_Window> glfw_to_window = Vector<GLFWWindow_to_Window>::allocate(0);

struct WindowAllocator
{
    static Token(Window) allocate(const uint16 p_width, const uint16 p_height, const Slice<int8>& p_name);
    static Token(Window) allocate_headless(const WindowHandle p_handle, const uint16 p_width, const uint16 p_height);
    static void free(const Token(Window) p_window);
    static void free_headless(const Token(Window) p_window);

    static Window& get_window(const Token(Window) p_window);

    static void on_glfw_window_resized(GLFWwindow* p_window, int p_width, int p_height);
    static void on_glfw_window_closed(GLFWwindow* p_window);
};

inline Window::ResizeEvent Window::ResizeEvent::build_default()
{
    return ResizeEvent{0, 0, 0};
};

inline Window Window::build(const WindowHandle p_handle, const uint16 p_width, const uint16 p_height)
{
    return Window{p_handle, p_width, p_height, 0, ResizeEvent::build_default()};
};

inline void Window::close()
{
    this->is_closing = 1;
};

inline int8 Window::asks_for_resize() const
{
    return this->resize_event.ask;
};

inline void Window::on_resized(const uint16 p_width, const uint16 p_height)
{
    this->resize_event.ask = 1;
    this->resize_event.new_width = p_width;
    this->resize_event.new_height = p_height;
};

inline void Window::consume_resize_event()
{
    this->resize_event.ask = 0;
    this->Width = this->resize_event.new_width;
    this->Height = this->resize_event.new_height;
};

inline Token(Window) WindowAllocator::allocate(const uint16 p_width, const uint16 p_height, const Slice<int8>& p_name)
{
    if (!glfw_initialized)
    {
        glfwInit();
        glfw_initialized = 1;
    }
    GLFWwindow* l_window = glfwCreateWindow(p_width, p_height, (int8*)p_name.Begin, NULL, NULL);
    glfwSetWindowSizeCallback(l_window, WindowAllocator::on_glfw_window_resized);
    glfwSetWindowCloseCallback(l_window, WindowAllocator::on_glfw_window_closed);

    Token(Window) l_allocated_window = allocate_headless((WindowHandle)l_window, p_width, p_height);

    glfw_to_window.push_back_element(GLFWWindow_to_Window{l_window, l_allocated_window});

    return l_allocated_window;
};

inline Token(Window) WindowAllocator::allocate_headless(const WindowHandle p_handle, const uint16 p_width, const uint16 p_height)
{
    return app_windows.alloc_element(Window::build(p_handle, p_width, p_height));
};

inline void WindowAllocator::free(const Token(Window) p_window)
{
    free_headless(p_window);

    for (loop(i, 0, glfw_to_window.Size))
    {
        if (tk_eq(glfw_to_window.get(i).window, p_window))
        {
            glfwDestroyWindow(glfw_to_window.get(i).glfw_window);
            glfw_to_window.erase_element_at_always(i);
        }
    }

    if (glfw_to_window.get_size() == 0)
    {
        glfw_to_window.free();
        glfwTerminate();
    };
};

inline void WindowAllocator::free_headless(const Token(Window) p_window)
{
    app_windows.release_element(p_window);

    if (!app_windows.has_allocated_elements())
    {
        app_windows.free();
    };
};

inline Window& WindowAllocator::get_window(const Token(Window) p_window)
{
    return app_windows.get(p_window);
};

inline void WindowAllocator::on_glfw_window_resized(GLFWwindow* p_window, int p_width, int p_height)
{
    for (loop(i, 0, glfw_to_window.Size))
    {
        if (glfw_to_window.get(i).glfw_window == p_window)
        {
            app_windows.get(glfw_to_window.get(i).window).on_resized(p_width, p_height);
        }
    }
};

inline void WindowAllocator::on_glfw_window_closed(GLFWwindow* p_window)
{
    for (loop(i, 0, glfw_to_window.Size))
    {
        if (glfw_to_window.get(i).glfw_window == p_window)
        {
            app_windows.get(glfw_to_window.get(i).window).close();
        }
    }
};

} // namespace v2