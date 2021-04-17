
#include "Engine/engine.hpp"

namespace MaterialSceneShocase_Const
{
const hash_t material_1 = HashSlice(slice_int8_build_rawstr("material_1.json"));
const hash_t material_2 = HashSlice(slice_int8_build_rawstr("material_2.json"));
const hash_t material_3 = HashSlice(slice_int8_build_rawstr("material_3.json"));
const hash_t material_4 = HashSlice(slice_int8_build_rawstr("material_4.json"));

const hash_t cone = HashSlice(slice_int8_build_rawstr("cone.obj"));
const hash_t cylinder = HashSlice(slice_int8_build_rawstr("cylinder.obj"));
const hash_t icosphere = HashSlice(slice_int8_build_rawstr("icosphere.obj"));
const hash_t sphere = HashSlice(slice_int8_build_rawstr("sphere.obj"));

const SliceN<hash_t, 4> materials_arr = {material_1, material_2, material_3, material_4};
const Slice<hash_t> materials_slice = slice_from_slicen(&materials_arr);

const SliceN<hash_t, 3> meshes_arr = {cone, cylinder, icosphere};
const Slice<hash_t> meshes_slice = slice_from_slicen(&meshes_arr);
} // namespace MaterialSceneShocase_Const

struct MaterialSceneShocase
{

    struct RandomAxisRotationNode
    {
        Token<Node> node;
        v3fn axis;
        float32 rot_speed;
    };

    Token<Node> camera_node;
    Token<Node> root_node;
    Vector<RandomAxisRotationNode> random_axis_rotation_nodes;

    inline static MaterialSceneShocase allocate()
    {
        srand(clock_currenttime_mics());
        return MaterialSceneShocase{token_build_default<Node>(), token_build_default<Node>(), Vector<RandomAxisRotationNode>::allocate(0)};
    };

    inline void free()
    {
        this->random_axis_rotation_nodes.free();
    };

    inline void cleanup(Engine& p_engine)
    {
        RemoveNode(p_engine, this->root_node);
        RemoveNode(p_engine, this->camera_node);
    };

    inline void step(const EngineExternalStep p_step, Engine& p_engine)
    {
        if (p_step == EngineExternalStep::BEFORE_UPDATE)
        {
            float32 l_delta_time = DeltaTime(p_engine);
            uimax l_frame = FrameCount(p_engine);

            if (l_frame == 1)
            {
                quat l_rot = m33f::lookat(v3f{7.0f, 7.0f, 7.0f}, v3f{0.0f, 0.0f, 0.0f}, v3f_const::UP).to_rotation();
                this->camera_node = CreateNode(p_engine, transform{v3f{7.0f, 7.0f, 7.0f}, l_rot, v3f_const::ONE.vec3});
                NodeAddCamera(p_engine, camera_node, CameraComponent::Asset{1.0f, 30.0f, 45.0f});

                this->root_node = CreateNode(p_engine, transform_const::ORIGIN);
            }

            if (l_frame >= 2 && l_frame <= 25)
            {
                float32 l_coord_0 = (float32)((rand() % 700) - 350) / 100.0f;
                float32 l_coord_1 = (float32)((rand() % 700) - 350) / 100.0f;
                float32 l_coord_2 = (float32)((rand() % 700) - 350) / 100.0f;
                transform l_node_transform = transform{v3f{l_coord_0, l_coord_1, l_coord_2}, quat_const::IDENTITY, v3f{0.5f, 0.5f, 0.5f}};

                Token<Node> l_node = CreateNode(p_engine, l_node_transform, this->root_node);
                NodeAddMeshRenderer(p_engine, l_node, MaterialSceneShocase_Const::materials_slice.get(rand() % (MaterialSceneShocase_Const::materials_slice.Size)),
                                    MaterialSceneShocase_Const::meshes_slice.get(rand() % (MaterialSceneShocase_Const::meshes_slice.Size)));

                RandomAxisRotationNode l_random_axis_rotation_node;
                l_random_axis_rotation_node.node = l_node;

                float32 l_axis_coord_0 = (float32)((rand() % 1000)) / 1000.0f;
                float32 l_axis_coord_1 = (float32)((rand() % 1000)) / 1000.0f;
                float32 l_axis_coord_2 = (float32)((rand() % 1000)) / 1000.0f;
                l_random_axis_rotation_node.axis = v3f{l_axis_coord_0, l_axis_coord_1, l_axis_coord_2}.normalize();
                l_random_axis_rotation_node.rot_speed = (float32)((rand() % 1000)) / 1000.0f;
                this->random_axis_rotation_nodes.push_back_element(l_random_axis_rotation_node);
            }

            {
                quat l_origin_rotation = quat::rotate_around(v3f_const::UP, 0.25f * l_delta_time);
                NodeAddWorldRotation(p_engine, this->root_node, l_origin_rotation);
            }

            for (loop(i, 0, this->random_axis_rotation_nodes.Size))
            {
                auto& l_r = this->random_axis_rotation_nodes.get(i);
                quat l_rotation = quat::rotate_around(l_r.axis, Math_const::PI * l_r.rot_speed * l_delta_time);
                NodeAddWorldRotation(p_engine, l_r.node, l_rotation);
            }
        }
    };
};

int32 main(int argc, char** argv)
{


#if __DEBUG
    String l_asset_db_file = String::allocate_elements_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("asset.db"));
#else
    String l_app_path = String::allocate_elements(slice_int8_build_rawstr(argv[0]));
    uimax l_index;
    while (Slice_find(l_app_path.to_slice_with_null_termination(), slice_int8_build_rawstr("\\"), &l_index))
    {
        l_app_path.Memory.get(l_index) = '/';
    }

    Path::move_up(&l_app_path);

    String l_asset_db_file = String::allocate_elements_2(l_app_path.to_slice(), slice_int8_build_rawstr("asset.db"));
    l_app_path.free();
#endif
    EngineConfiguration l_engine_configuration;
    l_engine_configuration.asset_database_path = l_asset_db_file.to_slice();
    l_engine_configuration.headless = 0;
    l_engine_configuration.render_size = v2ui{400, 400};
    l_engine_configuration.render_target_host_readable = 0;
    Engine l_engine = Engine::allocate(l_engine_configuration);

    MaterialSceneShocase l_sandbox = MaterialSceneShocase::allocate();
    EngineRunner::main_loop(l_engine, l_sandbox);
    l_sandbox.cleanup(l_engine);
    l_engine.free();
    l_sandbox.free();
    l_asset_db_file.free();
    memleak_ckeck();
}