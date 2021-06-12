#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

template <class ReturnType> inline static ReturnType socket_native_error_handler(const ReturnType p_return)
{
#if __DEBUG
    if (p_return != 0)
    {
        abort();
    }
#endif
    return p_return;
};

int8 socket_native::is_valid()
{
    return ((SOCKET)this->ptr) != INVALID_SOCKET;
};

void socket_native::invalidate()
{
    this->ptr = (int8*)INVALID_SOCKET;
};

socket_addrinfo socket_addrinfo_get(const socket_native_type p_type, const int32 p_port)
{
    socket_addrinfo l_addr_info;

    SliceN<int8, ToString::int32str_size> p_port_str_raw;
    Slice<int8> l_port_str = slice_from_slicen(&p_port_str_raw);
    ToString::aint32(p_port, l_port_str);

#if __DEBUG
    assert_true(p_type == socket_native_type::TCP);
#endif

    addrinfo l_typed_addr_info{};
    l_typed_addr_info.ai_family = AF_INET;
    l_typed_addr_info.ai_socktype = SOCK_STREAM;
    l_typed_addr_info.ai_protocol = IPPROTO_TCP;
    l_typed_addr_info.ai_flags = AI_PASSIVE;

    socket_native_error_handler(getaddrinfo(NULL, l_port_str.Begin, &l_typed_addr_info, (addrinfo**)&l_addr_info.ptr));

    return l_addr_info;
};

socket_addrinfo socket_addrinfo_next(socket_addrinfo p_addrinfo)
{
    addrinfo* l_addr_info = (addrinfo*)p_addrinfo.ptr;
    return socket_addrinfo{(int8*)l_addr_info};
};

socket_native socket_native_allocate(socket_addrinfo p_addrinfo)
{
    addrinfo* l_addr_info = (addrinfo*)p_addrinfo.ptr;

    socket_native l_socket;
    l_socket.ptr = (int8*)socket(l_addr_info->ai_family, l_addr_info->ai_socktype, l_addr_info->ai_protocol);

#if __DEBUG
    assert_true((SOCKET)l_socket.ptr != INVALID_SOCKET);
#endif

    return l_socket;
};

void socket_native_set_address_reusable(socket_native p_socket)
{
    int32 l_reuse_addres = 1;
    socket_native_error_handler(setsockopt((SOCKET)p_socket.ptr, SOL_SOCKET, SO_REUSEADDR, (const int8*)&l_reuse_addres, sizeof(l_reuse_addres)));
};

void socket_native_set_non_blocking(socket_native p_socket)
{
    uint32 l_mode = 1; // non-blocking
    socket_native_error_handler(ioctlsocket((SOCKET)p_socket.ptr, FIONBIO, (u_long*)&l_mode));
};

void socket_native_bind(socket_native p_socket, socket_addrinfo p_addrinfo)
{
    addrinfo* l_addr_info = (addrinfo*)p_addrinfo.ptr;
    socket_native_error_handler(bind((SOCKET)p_socket.ptr, l_addr_info->ai_addr, (int32)l_addr_info->ai_addrlen));
};

int8 socket_native_connect(socket_native p_socket, socket_addrinfo p_addrinfo)
{
    addrinfo* l_addr_info = (addrinfo*)p_addrinfo.ptr;
    return connect((SOCKET)p_socket.ptr, l_addr_info->ai_addr, (int32)l_addr_info->ai_addrlen) != 0;
};

void socket_native_close(socket_native p_socket)
{
    closesocket((SOCKET)p_socket.ptr);
};