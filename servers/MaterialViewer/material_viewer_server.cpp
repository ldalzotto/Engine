#include "MaterialViewer/Include/server.hpp"

int main()
{
    SocketContext l_socket_context = SocketContext::allocate();
    MaterialViewerServerJSONWebsocketThread l_server;
    // MaterialViewerServerJSONThread l_server;
    l_server.start(l_socket_context, 8000);

    while (1)
    {
    }

    l_socket_context.free();
    memleak_ckeck();

    return 0;
};