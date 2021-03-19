#pragma once

#include "./qt_include.hpp"

#include "Engine/engine.hpp"
#include "./qt_utility.hpp"

#include "AssetCompiler/asset_compiler.hpp"

struct EngineThread
{
    Engine engine;

    // TODO -> we must think of a better way to handle engine thread synchronisation
    int8 engine_spawned;
    int8 stop_at_end_of_frame;
    int8 is_stopped;

    String database_path;
    uint32 width;
    uint32 height;

    int8* thread_input_ptr;
    Thread::MainInput thread_input;
    thread_t thread;

    Token(Node) camera_node;
    Token(Node) material_node;
    Token(MeshRendererComponent) material_node_meshrenderer;

    struct SharedRessources
    {
        uimax material_hash;
        uimax mesh_hash;
        int8 change_requested;
    };

    Mutex<SharedRessources> shared;

    inline static EngineThread allocate()
    {
        EngineThread l_return{};
        l_return.database_path = String::allocate(0);
        l_return.material_node_meshrenderer = tk_bd(MeshRendererComponent);
        l_return.camera_node = tk_bd(Node);
        l_return.material_node = tk_bd(Node);
        return l_return;
    };

    inline void start(const Slice<int8> p_asset_database, const uint32 p_width, const uint32 p_height)
    {
        this->database_path.append(p_asset_database);
        this->width = p_width;
        this->height = p_height;
        this->thread_input_ptr = (int8*)this;
        this->thread_input = Thread::MainInput{EngineThread::main, Slice<int8*>::build_begin_end(&this->thread_input_ptr, 0, 1)};
        this->thread = Thread::spawn_thread(this->thread_input);
    };

    inline void free()
    {
        this->kill();
        this->database_path.free();
    }

    inline int8 is_running()
    {
        return this->thread != NULL;
    };

    // TODO -> we must think of a better way to handle engine thread synchronisation
    inline void wait_for_engine_spawned()
    {
        while (!this->engine_spawned)
        {
        }
    };

    // TODO -> we must think of a better way to handle engine thread synchronisation
    inline void wait_for_atleast_a_single_frame()
    {
        uimax l_old_frame = this->engine.clock.framecount;
        while (l_old_frame == this->engine.clock.framecount)
        {
        }
    };

    inline void kill()
    {
        this->engine.close();
        if (this->is_running())
        {
            Thread::wait_for_end_and_terminate(this->thread, -1);
            this->thread = NULL;
        }
    };

    inline void set_new_material(const hash_t p_new_material)
    {
        this->shared.acquire([&](SharedRessources& p_shared) {
            p_shared.change_requested = 1;
            p_shared.material_hash = p_new_material;
        });
    };

    inline void set_new_mesh(const hash_t p_new_mesh)
    {
        this->shared.acquire([&](SharedRessources& p_shared) {
            p_shared.change_requested = 1;
            p_shared.mesh_hash = p_new_mesh;
        });
    };

    inline static int8 main(const Slice<int8*>& p_args)
    {
        EngineThread* thiz = (EngineThread*)p_args.get(0);

        EngineConfiguration l_engine_config{};
        l_engine_config.asset_database_path = thiz->database_path.to_slice();
        l_engine_config.render_size = v2ui{thiz->width, thiz->height};

        thiz->engine = SpawnEngine(l_engine_config);

        thiz->engine_spawned = 1;

        struct s_engine_loop
        {
            EngineThread* thread;

            inline void step(const EngineExternalStep p_step, Engine& p_engine) const
            {
                engine_loop(p_step, p_engine, thread);
            };
        };

        thiz->engine.main_loop(s_engine_loop{thiz});

        RemoveNode(thiz->engine, thiz->camera_node);
        RemoveNode(thiz->engine, thiz->material_node);
        DestroyEngine(thiz->engine);

        return 0;
    };

  private:
    inline static void engine_loop(const EngineExternalStep p_step, Engine& p_engine, EngineThread* thiz)
    {
        if (p_step == EngineExternalStep::BEFORE_UPDATE)
        {
            float32 l_deltatime = DeltaTime(p_engine);
            uimax l_frame_count = FrameCount(p_engine);
            if (l_frame_count == 1)
            {
                thiz->camera_node = CreateNode(p_engine, transform_const::ORIGIN);
                thiz->material_node = CreateNode(p_engine, transform{v3f{0.0f, 0.0f, 5.0f}, quat_const::IDENTITY, v3f_const::ONE});
                NodeAddCamera(p_engine, thiz->camera_node, CameraComponent::Asset{1.0f, 30.0f, 45.0f});
            }
                thiz->shared.acquire([&](SharedRessources& p_shared) {
                    if (p_shared.change_requested)
                    {
                        if (tk_v(thiz->material_node_meshrenderer) != -1)
                    {
                        NodeRemoveMeshRenderer(p_engine, thiz->material_node);
                    }
                    thiz->material_node_meshrenderer = NodeAddMeshRenderer(p_engine, thiz->material_node, p_shared.material_hash, p_shared.mesh_hash);
                    p_shared.change_requested = 0;
                }
            });

            NodeAddWorldRotation(p_engine, thiz->material_node, quat::rotate_around(v3f_const::UP, 3 * l_deltatime));
        }
        else if (p_step == EngineExternalStep::END_OF_FRAME)
        {
            if (thiz->stop_at_end_of_frame)
            {
                thiz->is_stopped = 1;
                while (thiz->stop_at_end_of_frame)
                {
                }
                thiz->is_stopped = 0;
            }
        }
    };
};

struct MaterialViewerWindow
{

    struct View
    {
        QString database_file;
        QString selected_material;
        QString slected_mesh;
    } view;

    QWidget* root;

    struct Widgets
    {
        QVBoxLayout* main_layout;

        QLayoutWidget<QHBoxLayout, QWidget> db_layout;
        QPushButton* db_file_selection;
        QFileDialog* db_file_dialog;
        QLabel* db_file_label;

        QLayoutWidget<QVBoxLayout, QGroupBox> selected_material_root;
        QListWidget* selected_material;

        QLayoutWidget<QVBoxLayout, QGroupBox> selected_mesh_root;
        QListWidget* selected_mesh;
    } widgets;

    typedef void (*cb)(MaterialViewerWindow* p_window, void* p_closure);

    struct Callbacks
    {
        void* closure;
        cb on_database_selected;
        cb on_material_selected;
        cb on_mesh_selected;
    } callbacks;

    inline void allocate(const Callbacks& p_callbacks)
    {
        this->allocate_widgets();
        this->setup_widget_layout();
        this->setup_widget_events(p_callbacks);
    };

  private:
    inline void allocate_widgets()
    {
        this->root = new QWidget();
        this->widgets.db_file_selection = new QPushButton("DB");
        this->widgets.db_file_label = new QLabel();

        this->widgets.selected_material_root = QLayoutWidget<QVBoxLayout, QGroupBox>::allocate();
        this->widgets.selected_material_root.widget->setTitle("material");
        this->widgets.selected_material = new QListWidget();

        this->widgets.selected_mesh_root = QLayoutWidget<QVBoxLayout, QGroupBox>::allocate();
        this->widgets.selected_mesh_root.widget->setTitle("mesh");
        this->widgets.selected_mesh = new QListWidget();

        this->widgets.main_layout = new QVBoxLayout(this->root);

        this->widgets.db_layout = QLayoutWidget<QHBoxLayout, QWidget>::allocate();
    };

    inline void setup_widget_layout()
    {
        this->root->setLayout(this->widgets.main_layout);

        QLayoutBuilder l_builder;

        l_builder.bind_layout(this->widgets.db_layout);
        l_builder.add_widget_2(this->widgets.db_file_selection, this->widgets.db_file_label);

        l_builder.bind_layout(this->widgets.selected_material_root);
        l_builder.add_widget(this->widgets.selected_material);

        l_builder.bind_layout(this->widgets.selected_mesh_root);
        l_builder.add_widget(this->widgets.selected_mesh);

        l_builder.bind_layout(this->widgets.main_layout);
        l_builder.add_widget_3(this->widgets.db_layout.widget, this->widgets.selected_material_root.widget, this->widgets.selected_mesh_root.widget);
    };

    inline void setup_widget_events(const Callbacks& p_callbacks)
    {
        this->callbacks = p_callbacks;

        QObject::connect(this->widgets.db_file_selection, &QPushButton::released, [&]() {
            this->widgets.db_file_dialog = new QFileDialog(this->root);
            this->widgets.db_file_dialog->setNameFilters({"DB files (*.db)", "Any files (*)"});
            QObject::connect(this->widgets.db_file_dialog, &QFileDialog::fileSelected, [&, this](const QString& p_file) {
                this->on_database_file_selected(p_file);
            });
            this->widgets.db_file_dialog->open();
        });

        QObject::connect(this->widgets.selected_material, &QListWidget::currentItemChanged, [&](QListWidgetItem* current, QListWidgetItem* previous) {
            if (current)
            {
                this->on_material_selected(current->text());
            }
        });

        QObject::connect(this->widgets.selected_mesh, &QListWidget::currentItemChanged, [&](QListWidgetItem* current, QListWidgetItem* previous) {
            if (current)
            {
                this->on_mesh_selected(current->text());
            }
        });
    };

    inline void on_database_file_selected(const QString& p_file)
    {
        this->view = View{};
        this->view.database_file = p_file;
        this->widgets.db_file_label->setText(this->view.database_file);
        if (this->callbacks.on_database_selected)
        {
            this->callbacks.on_database_selected(this, this->callbacks.closure);
        }

        QByteArray l_database_file_path_arr = this->view.database_file.toLocal8Bit();
        Slice<int8> l_database_file_path = slice_int8_build_rawstr(l_database_file_path_arr.data());
        DatabaseConnection l_connection = DatabaseConnection::allocate(l_database_file_path);
        AssetMetadataDatabase l_asset_metadata_database = AssetMetadataDatabase::allocate(l_connection);
        {
            AssetMetadataDatabase::Paths l_material_paths = l_asset_metadata_database.get_all_path_from_type(l_connection, AssetType_Const::MATERIAL_NAME);

            this->widgets.selected_material->clear();
            for (loop(i, 0, l_material_paths.data.Size))
            {
                Span<int8>& l_path = l_material_paths.data.get(i);
                QString l_str = QString::fromLocal8Bit(l_path.Memory, l_path.Capacity);
                this->widgets.selected_material->addItem(l_str);
            }
            l_material_paths.free();
        }
        {
            AssetMetadataDatabase::Paths l_mesh_paths = l_asset_metadata_database.get_all_path_from_type(l_connection, AssetType_Const::MESH_NAME);

            this->widgets.selected_mesh->clear();
            for(loop(i, 0, l_mesh_paths.data.Size))
            {
                Span<int8>& l_path = l_mesh_paths.data.get(i);
                QString l_str = QString::fromLocal8Bit(l_path.Memory, l_path.Capacity);
                this->widgets.selected_mesh->addItem(l_str);
            }
            l_mesh_paths.free();
        }


        l_asset_metadata_database.free(l_connection);
        l_connection.free();
    };

    inline void on_material_selected(const QString& p_file)
    {
        this->view.selected_material = p_file;
        if (this->callbacks.on_material_selected)
        {
            this->callbacks.on_material_selected(this, this->callbacks.closure);
        }
    };

    inline void on_mesh_selected(const QString& p_file)
    {
        this->view.slected_mesh = p_file;
        if (this->callbacks.on_mesh_selected)
        {
            this->callbacks.on_mesh_selected(this, this->callbacks.closure);
        }
    };
};

// TODO -> adding test
struct MaterialViewerEditor
{
    EngineThread engine_thread;
    MaterialViewerWindow material_viewer;

    inline void allocate()
    {
        this->engine_thread = EngineThread::allocate();

        MaterialViewerWindow::Callbacks l_callbacks;
        l_callbacks.closure = this;
        l_callbacks.on_database_selected = [](auto, void* p_editor) {
            MaterialViewerEditor* thiz = (MaterialViewerEditor*)p_editor;
            if (thiz->engine_thread.is_running())
            {
                thiz->engine_thread.free();
                thiz->engine_thread = EngineThread::allocate();
            }
            thiz->engine_thread.start(slice_int8_build_rawstr(thiz->material_viewer.view.database_file.toLocal8Bit().data()), 400, 400);
        };
        l_callbacks.on_material_selected = [](auto, void* p_editor) {
            MaterialViewerEditor* thiz = (MaterialViewerEditor*)p_editor;
            thiz->try_to_update_material_or_mesh();
        };
        l_callbacks.on_mesh_selected = [](auto, void* p_editor) {
            MaterialViewerEditor* thiz = (MaterialViewerEditor*)p_editor;
            thiz->try_to_update_material_or_mesh();
        };
        this->material_viewer.allocate(l_callbacks);
    };

    inline void free()
    {
        this->engine_thread.free();
    };

    inline QWidget* root()
    {
        return this->material_viewer.root;
    };

  private:
    inline void try_to_update_material_or_mesh()
    {
        if (this->material_viewer.view.selected_material.isEmpty() || this->material_viewer.view.slected_mesh.isEmpty())
        {
            return;
        }

        this->engine_thread.set_new_material(HashSlice(slice_int8_build_rawstr(this->material_viewer.view.selected_material.toLocal8Bit().data())));
        this->engine_thread.set_new_mesh(HashSlice(slice_int8_build_rawstr(this->material_viewer.view.slected_mesh.toLocal8Bit().data())));
    };
};