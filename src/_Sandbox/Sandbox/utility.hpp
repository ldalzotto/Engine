#pragma once

struct SandboxUtility
{
    inline static Token(MeshRendererComponent) add_meshrenderer_component_to_node_with_database(Engine& p_engine, Token(Node) p_node, const hash_t p_material_id, const hash_t p_mesh_id){
        Token(MeshRendererComponent) l_mesh_renderer = RenderMiddleWare_AllocationComposition::allocate_meshrenderer_database_and_load_dependecies(
            p_engine.scene_middleware.render_middleware, p_engine.renderer_ressource_allocator, p_engine.asset_database,
            MeshRendererComponent::AssetDependencies{p_material_id, p_mesh_id}, p_node);
        p_engine.scene.add_node_component_by_value(p_node, MeshRendererComponentAsset_SceneCommunication::construct_nodecomponent(l_mesh_renderer));
        return l_mesh_renderer;
    };

    inline static void append_frame_number_to_string(String& p_string, const uimax p_frame_number)
    {
        Slice<int8> l_uimax_str_buffer = SliceN<int8, ToString::uimaxstr_size>{}.to_slice();
        p_string.append(ToString::auimax(p_frame_number, l_uimax_str_buffer));
        p_string.append(slice_int8_build_rawstr(".jpg"));
    };
};