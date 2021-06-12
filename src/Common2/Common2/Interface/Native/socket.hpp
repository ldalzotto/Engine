#pragma once

enum class socket_native_type : int8
{
    TCP = 0
};

struct socket_native
{
    int8* ptr;

    int8 is_valid();
    void invalidate();
};

struct socket_addrinfo
{
    int8* ptr;

    int8 is_null()
    {
        return this->ptr == NULL;
    };
};

int32 socket_native_get_last_error();

socket_addrinfo socket_addrinfo_get(const socket_native_type p_type, const int32 p_port);
socket_addrinfo socket_addrinfo_next(socket_addrinfo p_addrinfo);

socket_native socket_native_allocate(socket_addrinfo p_addrinfo);
void socket_native_set_address_reusable(socket_native p_socket);
void socket_native_set_non_blocking(socket_native p_socket);
void socket_native_bind(socket_native p_socket, socket_addrinfo p_addrinfo);
int8 socket_native_connect(socket_native p_socket, socket_addrinfo p_addrinfo);
void socket_native_close(socket_native p_socket);
// If the socket returns an error that tells that the call should have blocked
int8 socket_native_is_error_is_blocking_type(const int32 p_error);
// If the socket returns an error that tells that there is still data to receive
int8 socket_native_is_error_is_msgsize_type(const int32 p_error);
