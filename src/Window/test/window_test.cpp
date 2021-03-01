#include "Window/window.hpp"

inline void glfwwindow()
{
    Token(v2::Window) l_window = v2::WindowAllocator::allocate(100, 150, slice_int8_build_rawstr("TEST"));
    v2::Window& l_allocated_window = v2::WindowAllocator::get_window(l_window);
    assert_true(!l_allocated_window.is_closing);
    assert_true(l_allocated_window.Width == 100);
    assert_true(l_allocated_window.Height == 150);

    v2::WindowAllocator::get_window(l_window).close();
    assert_true(v2::WindowAllocator::get_window(l_window).is_closing);
    v2::WindowAllocator::free(l_window);
};

inline void headlesswindow()
{
    Token(v2::Window) l_window = v2::WindowAllocator::allocate_headless(NULL, 100, 150);
    v2::Window& l_allocated_window = v2::WindowAllocator::get_window(l_window);
    assert_true(!l_allocated_window.is_closing);
    assert_true(l_allocated_window.Width == 100);
    assert_true(l_allocated_window.Height == 150);

    v2::WindowAllocator::get_window(l_window).close();
    assert_true(v2::WindowAllocator::get_window(l_window).is_closing);
    v2::WindowAllocator::free_headless(l_window);
};

int main()
{
    glfwwindow();
    headlesswindow();

    memleak_ckeck();
};