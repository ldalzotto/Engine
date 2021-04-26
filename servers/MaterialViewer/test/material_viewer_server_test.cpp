
#include "MaterialViewer/material_viewer_server.hpp"

inline void server_test()
{
    SocketContext l_socket_context = SocketContext::allocate();

    MaterialViewerServerJSONThread l_server;
    l_server.start(l_socket_context, 8000);
    l_server.thread.sync_wait_for_allocation();

    MaterialViewerClientJSONThread l_client;
    l_client.start(&l_socket_context, 8000);
    l_client.thread.sync_wait_for_allocation();

    l_server.thread.sync_wait_for_client_detection();

    l_client.material_viewer_client.ENGINE_THREAD_START(l_socket_context, l_client.thread.client,
                                                        slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/_asset/asset/MaterialViewer_Package/asset.db"));
    MaterialViewerServerV2::GetAllMaterialsOutputJSON l_materials = l_client.material_viewer_client.GET_ALL_MATERIALS(l_socket_context, l_client.thread.client);

    l_materials.free();

    MaterialViewerServerV2::SetMaterialAndMeshInputJSON l_material_mesh_set = {};
    l_material_mesh_set.material = slice_int8_build_rawstr("material_1.json");
    l_material_mesh_set.mesh = slice_int8_build_rawstr("cube.obj");
    l_client.material_viewer_client.SET_MATERIAL_AND_MESH(l_socket_context, l_client.thread.client, l_material_mesh_set);

    Thread::wait(Thread::get_current_thread(), 1000);

    l_client.material_viewer_client.ENGINE_THREAD_STOP(l_socket_context, l_client.thread.client);

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