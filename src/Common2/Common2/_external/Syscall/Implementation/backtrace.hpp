#pragma once

void backtrace_capture(void* p_backtrace_array, const uimax p_backtrace_array_size)
{
    CaptureStackBackTrace(0, (DWORD)p_backtrace_array_size, (PVOID*)p_backtrace_array, NULL);
};
