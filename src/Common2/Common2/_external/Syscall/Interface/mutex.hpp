#pragma once

struct mutex_native {
    void* ptr;
};

mutex_native mutex_native_allocate();
void mutex_native_lock(const mutex_native p_mutex);
void mutex_native_unlock(const mutex_native p_mutex);
void mutex_native_free(mutex_native& p_mutex);