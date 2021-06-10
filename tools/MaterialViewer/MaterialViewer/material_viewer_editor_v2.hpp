#pragma once

#include "Engine/engine.hpp"
#include "AssetCompiler/asset_compiler.hpp"
#include "qt_interface.hpp"

struct MaterialViewerEngineUnit
{
    int8 is_running;

    Token<Node> camera_node;
    Token<Node> material_node;
    Token<MeshRendererComponent> material_node_meshrenderer;

    struct SharedResources
    {
        uimax material_hash;
        uimax mesh_hash;
        int8 change_requested;
    };

    Mutex<SharedResources> shared;

    Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present engine;
    thread_t thread;
    EngineThreadSync thread_synchronization;

    struct Input
    {
        Span<int8> asset_database;
        v2ui render_size;

        inline void free()
        {
            this->asset_database.free();
        };
    } input;

    struct Exec
    {
        MaterialViewerEngineUnit* thiz;

        inline int8 operator()() const
        {
            thiz->engine = Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present::allocate(Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present::RuntimeConfiguration{
                EngineModuleCore::RuntimeConfiguration{1000000 / 60}, thiz->input.asset_database.slice, v2ui{thiz->input.render_size.x, thiz->input.render_size.y}, 0});

            iEngine<Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present> l_engine = iEngine<Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present>{thiz->engine};
            l_engine.main_loop_blocking_typed([&](const float32 p_delta) {
                thiz->thread_synchronization.on_end_of_frame();
                thiz->thread_synchronization.on_start_of_frame();

                float32 l_deltatime = l_engine.deltatime();
                uimax l_frame_count = l_engine.frame_count();
                if (l_frame_count == 1)
                {
                    thiz->camera_node = l_engine.create_node(transform_const::ORIGIN);
                    thiz->material_node = l_engine.create_node(transform{v3f{0.0f, 0.0f, 5.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3});
                    l_engine.node_add_camera(thiz->camera_node, CameraComponent::Asset{1.0f, 30.0f, 45.0f});
                }
                thiz->shared.acquire([&](SharedResources& p_shared) {
                    if (p_shared.change_requested)
                    {
                        if (token_value(thiz->material_node_meshrenderer) != -1)
                        {
                            l_engine.node_remove_meshrenderer(thiz->material_node);
                        }
                        thiz->material_node_meshrenderer = l_engine.node_add_meshrenderer(thiz->material_node, p_shared.material_hash, p_shared.mesh_hash);
                        p_shared.change_requested = 0;
                    }
                });

                l_engine.node_add_worldrotation(thiz->material_node, quat::rotate_around(v3f_const::UP, 3 * l_deltatime));

                if (thiz->thread_synchronization.ask_exit)
                {
                    thiz->engine.core.close();
                }
            });

            thiz->input.free();
            thiz->cleanup_resources();
            thiz->engine.free();

            return 0;
        };
    } exec;

    inline static MaterialViewerEngineUnit allocate()
    {
        MaterialViewerEngineUnit l_return{};
        l_return.set_sefault_values();
        l_return.shared = Mutex<SharedResources>::allocate();
        return l_return;
    };

    inline void set_sefault_values()
    {
        this->thread = NULL;
        this->thread_synchronization.reset();
        this->material_node_meshrenderer = token_build_default<MeshRendererComponent>();
        this->camera_node = token_build_default<Node>();
        this->material_node = token_build_default<Node>();
    };

    inline void free()
    {
        this->stop();
        this->shared.free();
    };

    inline void start(const Slice<int8> p_asset_database, const uint32 p_width, const uint32 p_height)
    {
        this->is_running = 1;
        this->input.asset_database = Span<int8>::allocate_slice(p_asset_database);
        this->input.render_size = v2ui{p_width, p_height};
        this->exec = Exec{this};
        this->thread = Thread::spawn_thread(this->exec);
    };

    inline void stop()
    {

        if (this->is_running)
        {
            this->thread_synchronization.exit();
            Thread::wait_for_end_and_terminate(this->thread, -1);
            this->set_sefault_values();
            this->is_running = 0;
        }
    };

    inline void restart(const Slice<int8> p_asset_database, const uint32 p_width, const uint32 p_height)
    {
        this->stop();
        this->start(p_asset_database, p_width, p_height);
    };

    inline void set_new_material(const hash_t p_new_material)
    {
        this->shared.acquire([&](SharedResources& p_shared) {
            p_shared.change_requested = 1;
            p_shared.material_hash = p_new_material;
        });
    };

    inline void set_new_mesh(const hash_t p_new_mesh)
    {
        this->shared.acquire([&](SharedResources& p_shared) {
            p_shared.change_requested = 1;
            p_shared.mesh_hash = p_new_mesh;
        });
    };

  private:
    inline void cleanup_resources()
    {
        iEngine<Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present> l_engine = iEngine<Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present>{this->engine};
        l_engine.remove_node(this->camera_node);
        l_engine.remove_node(this->material_node);
        this->material_node_meshrenderer = token_build_default<MeshRendererComponent>();
    };
};

enum class UiTag : uimax
{
    DB_Button = 1,
    DB_FileSelection = 2,
    Material_List = 3,
    Meshes_List = 4
};

template <class ElementType> struct GUIListTyped
{
    GUIList& list;

    inline void add_item(const Slice<int8>& p_name, const Token<ElementType> p_value)
    {
        this->list.list->add_item(p_name, token_build_from<int8>(p_value));
    };

    inline void clear(Pool<ElementType>& p_elements)
    {
        for (loop(i, 0, this->list.list->get_size()))
        {
            p_elements.release_element(token_build_from<ElementType>(this->list.list->get_value(i)));
        }
        this->list.list->clear();
    };
};

struct Elements
{
    _GUIWidget* root;
    _GUIPushButton* db_button;
    _GUILabel* db_label;
    _GUIFileDialog* db_filedialog;
    _GUILabel* db_status;
    _GUIListWidget* material_list;
    _GUIListWidget* meshes_list;

    inline static Elements load(_GUIWidget* p_from)
    {
        Elements l_return;
        l_return.root = p_from;
        l_return.db_button = (_GUIPushButton*)(p_from)->find_child(slice_int8_build_rawstr("DB_button"));
        l_return.db_label = (_GUILabel*)(p_from)->find_child(slice_int8_build_rawstr("DB_label"));
        l_return.db_status = (_GUILabel*)(p_from)->find_child(slice_int8_build_rawstr("DB_status"));
        l_return.material_list = (_GUIListWidget*)(p_from)->find_child(slice_int8_build_rawstr("materials_view"));
        l_return.meshes_list = (_GUIListWidget*)(p_from)->find_child(slice_int8_build_rawstr("meshes_view"));
        return l_return;
    };
};

struct App
{
    GUIEventHandler event_handler;

    Elements main_elements;
    Pool<GUIButton> buttons;
    Pool<GUIFileDialog> file_dialogs;
    Pool<GUIList> lists;

    Token<GUIButton> db_button;
    Token<GUIList> materials_selection;
    Token<GUIList> meshes_selection;

    Pool<Span<int8>> asset_paths;
    Token<Span<int8>> selected_material;
    Token<Span<int8>> selected_mesh;

    MaterialViewerEngineUnit engine_unit;

    inline static App allocate()
    {
        App l_return;
        _GUIWidget* l_root = _GUIWidget::load_design_sheet(slice_int8_build_rawstr("E:/SoftwareProjects/qt_uis/my_widget.ui"));
        l_return.main_elements = Elements::load(l_root);
        l_return.buttons = Pool<GUIButton>::allocate(0);
        l_return.file_dialogs = Pool<GUIFileDialog>::allocate(0);
        l_return.lists = Pool<GUIList>::allocate(0);
        l_return.engine_unit = MaterialViewerEngineUnit::allocate();
        l_return.asset_paths = Pool<Span<int8>>::allocate(0);
        l_return.selected_material = token_build_default<Span<int8>>();
        l_return.selected_mesh = token_build_default<Span<int8>>();
        return l_return;
    };

    inline void setup()
    {
        this->event_handler.callback = handle_event;
        this->event_handler.closure = (int8*)this;
        this->db_button = GUIButton::register_(this->main_elements.db_button, (uimax)UiTag::DB_Button, this->buttons, this->event_handler);

        this->main_elements.db_filedialog = _GUIFileDialog::allocate((_GUIWidget*)this->main_elements.root);
        GUIFileDialog::register_(this->main_elements.db_filedialog, (uimax)UiTag::DB_FileSelection, this->file_dialogs, this->event_handler);
        this->main_elements.db_filedialog->close();

        this->materials_selection = GUIList::register_(this->main_elements.material_list, (uimax)UiTag::Material_List, this->lists, this->event_handler);
        this->meshes_selection = GUIList::register_(this->main_elements.meshes_list, (uimax)UiTag::Meshes_List, this->lists, this->event_handler);

        this->main_elements.db_status->set_text(slice_int8_build_rawstr("Waiting for asset database."));
    };

    inline void free()
    {
#if __DEBUG
        assert_true(!this->asset_paths.has_allocated_elements());
#endif
        this->buttons.free();
        this->file_dialogs.free();
        this->lists.free();

        this->engine_unit.free();
    };

  private:
    inline static void handle_event(const GUIEvent& p_event, int8* p_args, int8* p_thiz)
    {
        ((App*)p_thiz)->_handle_event(p_event, p_args);
    };

    inline void _handle_event(const GUIEvent& p_event, int8* p_args)
    {

        switch (p_event.element_type)
        {
        case GUIEvent::SourceElementType::BUTTON:
        {
            switch (p_event.type)
            {
            case GUIEvent::Type::JUST_PRESSED:
            {
                Token<GUIButton> l_button_token = token_build_from<GUIButton>(p_event.token);
                GUIButton& l_button = this->buttons.get(l_button_token);
                switch ((UiTag)l_button.tag)
                {
                case UiTag::DB_Button:
                {
                    this->main_elements.db_filedialog->open();
                }
                break;
                }
            }
            break;
            }
        }
        break;
        case GUIEvent::SourceElementType::FILE_DIALOG:
        {
            switch (p_event.type)
            {
            case GUIEvent::Type::FILE_SELECTED:
            {
                Token<GUIFileDialog> l_file_dialog_token = token_build_from<GUIFileDialog>(p_event.token);
                GUIFileDialog& l_file_dialog = this->file_dialogs.get(l_file_dialog_token);
                const Slice<int8> l_selected_file = *(const Slice<int8>*)p_args;
                this->main_elements.db_label->set_text(l_selected_file);

                switch ((UiTag)l_file_dialog.tag)
                {
                case UiTag::DB_FileSelection:
                {
                    DatabaseConnection l_connection = DatabaseConnection::allocate(l_selected_file);
                    if (DatabaseConnection_is_valid_silent(l_connection))
                    {
                        this->main_elements.db_status->set_text(slice_int8_build_rawstr("Database recognized !"));
                        this->engine_unit.restart(l_selected_file, 400, 400);

                        // Add meshes and materials

                        AssetMetadataDatabase l_asset_metadata_database = AssetMetadataDatabase::allocate(l_connection);
                        {
                            AssetMetadataDatabase::Paths l_material_paths = l_asset_metadata_database.get_all_path_from_type(l_connection, AssetType_Const::MATERIAL_NAME);

                            GUIListTyped<Span<int8>> l_material_list_gui = GUIListTyped<Span<int8>>{this->lists.get(this->materials_selection)};
                            this->clear_material_list();
                            for (loop(i, 0, l_material_paths.data_v2.get_size()))
                            {
                                Slice<int8> l_path = l_material_paths.data_v2.get(i);
                                l_material_list_gui.add_item(l_path, this->asset_paths.alloc_element(Span<int8>::allocate_slice(l_path)));
                            }
                            l_material_paths.free();
                        }

                        {
                            AssetMetadataDatabase::Paths l_mesh_paths = l_asset_metadata_database.get_all_path_from_type(l_connection, AssetType_Const::MESH_NAME);

                            GUIListTyped<Span<int8>> l_mesh_list_gui = GUIListTyped<Span<int8>>{this->lists.get(this->meshes_selection)};
                            this->clear_meshes_list();
                            for (loop(i, 0, l_mesh_paths.data_v2.get_size()))
                            {
                                Slice<int8> l_path = l_mesh_paths.data_v2.get(i);
                                l_mesh_list_gui.add_item(l_path, this->asset_paths.alloc_element(Span<int8>::allocate_slice(l_path)));
                            }
                            l_mesh_paths.free();
                        }

                        l_asset_metadata_database.free(l_connection);
                    }
                    else
                    {
                        this->main_elements.db_status->set_text(slice_int8_build_rawstr("Database invalid format !"));
                    }

                    l_connection.free();
                }
                break;
                }
            }
            break;
            }
        }
        break;
        case GUIEvent::SourceElementType::LIST:
        {
            switch (p_event.type)
            {
            case GUIEvent::Type::LIST_SELECTION_CHANGED:
            {
                Token<GUIList> l_list_token = token_build_from<GUIList>(p_event.token);
                GUIList& l_list = this->lists.get(l_list_token);
                GUIListSelectionChangeInput* l_input = (GUIListSelectionChangeInput*)p_args;
                switch ((UiTag)l_list.tag)
                {
                case UiTag::Material_List:
                {
                    this->selected_material = token_build_from<Span<int8>>(l_input->current_element);
                    this->on_material_or_mesh_selected();
                }
                break;
                case UiTag::Meshes_List:
                {
                    this->selected_mesh = token_build_from<Span<int8>>(l_input->current_element);
                    this->on_material_or_mesh_selected();
                }
                break;
                }
            }
            break;
            }
        }
        break;
        case GUIEvent::SourceElementType::APPLICATION:
        {
            switch (p_event.type)
            {
            case GUIEvent::Type::JUST_EXIT:
            {
                this->clear_material_list();
                this->clear_meshes_list();
            }
            break;
            }
        }
        break;
        }
    };

    inline void on_material_or_mesh_selected()
    {
        if (!token_equals(this->selected_material, token_build_default<Span<int8>>()) && !token_equals(this->selected_mesh, token_build_default<Span<int8>>()))
        {
            this->engine_unit.set_new_material(HashFunctions::hash(this->asset_paths.get(this->selected_material).slice));
            this->engine_unit.set_new_mesh(HashFunctions::hash(this->asset_paths.get(this->selected_mesh).slice));
        }
    };

    inline void clear_material_list()
    {
        for (loop(i, 0, this->main_elements.material_list->get_size()))
        {
            Token<Span<int8>> l_asset_path = token_build_from<Span<int8>>(this->main_elements.material_list->get_value(i));
            this->asset_paths.get(l_asset_path).free();
            this->asset_paths.release_element(l_asset_path);
        }
        this->main_elements.material_list->clear();
    };

    inline void clear_meshes_list()
    {
        for (loop(i, 0, this->main_elements.meshes_list->get_size()))
        {
            Token<Span<int8>> l_asset_path = token_build_from<Span<int8>>(this->main_elements.meshes_list->get_value(i));
            this->asset_paths.get(l_asset_path).free();
            this->asset_paths.release_element(l_asset_path);
        }
        this->main_elements.meshes_list->clear();
    };
};