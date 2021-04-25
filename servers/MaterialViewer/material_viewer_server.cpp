#include "MaterialViewer/material_viewer_server.hpp"

int main()
{
    SocketContext l_socket_context = SocketContext::allocate();
    MaterialViewerServer l_server = MaterialViewerServer::allocate(l_socket_context, 8000);

    l_server.mloop(l_socket_context);

    l_socket_context.free();
    memleak_ckeck();

    return 0;
};