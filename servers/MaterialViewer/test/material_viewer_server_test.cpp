
#include "MaterialViewer/Include/server.hpp"

inline void material_viewer_execution_test(){
    // TODO -> write test that run the execution unit only.
};

inline void client_test()
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
                                                        slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/_asset/asset/MaterialViewerServer/asset.db"));

    Engine& l_server_engine = l_server.material_viewer_server_v2.engine_thread.engines.get(l_server.material_viewer_server_v2.material_viewer_unit.engine_execution_unit).engine;

    {
        MaterialViewerDomain::GetAllMaterialsOutputJSON l_materials = l_client.material_viewer_client.GET_ALL_MATERIALS(l_socket_context, l_client.thread.client);

        assert_true(l_materials.materials.Size == 4);
        int l_material_counter = 0;
        for (loop(i, 0, l_materials.materials.Size))
        {

            if (l_materials.materials.get(i).slice.compare(slice_int8_build_rawstr("material_1.json")) || l_materials.materials.get(i).slice.compare(slice_int8_build_rawstr("material_2.json")) ||
                l_materials.materials.get(i).slice.compare(slice_int8_build_rawstr("material_3.json")) || l_materials.materials.get(i).slice.compare(slice_int8_build_rawstr("material_4.json")))
            {
                l_material_counter += 1;
            };
        }
        assert_true(l_material_counter == l_materials.materials.Size);

        l_materials.free();
    }

    {
        MaterialViewerDomain::GetAllMeshesOutputJSON l_meshes = l_client.material_viewer_client.GET_ALL_MESH(l_socket_context, l_client.thread.client);

        assert_true(l_meshes.meshes.Size == 5);
        int l_material_counter = 0;
        for (loop(i, 0, l_meshes.meshes.Size))
        {

            if (l_meshes.meshes.get(i).slice.compare(slice_int8_build_rawstr("cone.obj")) || l_meshes.meshes.get(i).slice.compare(slice_int8_build_rawstr("cube.obj")) ||
                l_meshes.meshes.get(i).slice.compare(slice_int8_build_rawstr("cylinder.obj")) || l_meshes.meshes.get(i).slice.compare(slice_int8_build_rawstr("icosphere.obj")) ||
                l_meshes.meshes.get(i).slice.compare(slice_int8_build_rawstr("sphere.obj")))
            {
                l_material_counter += 1;
            };
        }
        assert_true(l_material_counter == l_meshes.meshes.Size);

        l_meshes.free();
    }

    {
        MaterialViewerDomain::SetMaterialAndMeshInputJSON l_material_mesh_set = {};
        l_material_mesh_set.material = slice_int8_build_rawstr("material_1.json");
        l_material_mesh_set.mesh = slice_int8_build_rawstr("cube.obj");
        l_client.material_viewer_client.SET_MATERIAL_AND_MESH(l_socket_context, l_client.thread.client, l_material_mesh_set);
    }

    l_client.material_viewer_client.ENGINE_THREAD_STOP(l_socket_context, l_client.thread.client);

    l_client.free(l_socket_context);
    l_server.free(l_socket_context);

    l_socket_context.free();
};

int main()
{
    material_viewer_execution_test();
    client_test();

    memleak_ckeck();

    return 0;
};