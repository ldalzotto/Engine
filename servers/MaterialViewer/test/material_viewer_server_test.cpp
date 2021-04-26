
#include "MaterialViewer/material_viewer_server.hpp"

inline void server_test()
{
    SocketContext l_socket_context = SocketContext::allocate();

    MaterialViewerServerThread l_server;
    l_server.start(l_socket_context, 8000);
    l_server.thread.sync_wait_for_allocation();

    MaterialViewerClientThread l_client;
    l_client.start(&l_socket_context, 8000);
    l_client.thread.sync_wait_for_allocation();

    l_server.thread.sync_wait_for_client_detection();

    l_client.material_viewer_client.ENGINE_THREAD_START(l_socket_context, l_client.thread.client,
                                                        slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/_asset/asset/MaterialViewer_Package/asset.db"));
    MaterialViewerClient::GetAllMaterialsReturn l_materials = l_client.material_viewer_client.GET_ALL_MATERIALS(l_socket_context, l_client.thread.client);

    l_materials.free();

    l_client.free(l_socket_context);
    l_server.free(l_socket_context);

    l_socket_context.free();
};

int main()
{
    server_test();

    memleak_ckeck();

    return 0;
};