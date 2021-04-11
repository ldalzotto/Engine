#pragma once

#include "QTCommon/qt_common.hpp"
#include "Engine/engine.hpp"
#include "AssetCompiler/asset_compiler.hpp"

struct MaterialViewerEngineUnit
{
    int8 is_running;
    Token<EngineExecutionUnit> engine_execution_unit;

    Token<Node> camera_node;
    Token<Node> material_node;
    Token<MeshRendererComponent> material_node_meshrenderer;

    struct SharedRessources
    {
        uimax material_hash;
        uimax mesh_hash;
        int8 change_requested;
    };

    MutexNative<SharedRessources> shared;

    inline static MaterialViewerEngineUnit allocate()
    {
        MaterialViewerEngineUnit l_return{};
        l_return.set_sefault_values();
        l_return.shared = MutexNative<SharedRessources>::allocate();
        return l_return;
    };

    inline void set_sefault_values()
    {
        this->engine_execution_unit = token_build_default<EngineExecutionUnit>();
        this->material_node_meshrenderer = token_build_default<MeshRendererComponent>();
        this->camera_node = token_build_default<Node>();
        this->material_node = token_build_default<Node>();
    };

    inline void free(EngineRunnerThread& p_engine_runner)
    {
        this->stop(p_engine_runner);
        this->shared.free();
    };

    inline void start(EngineRunnerThread& p_engine_runner, const Slice<int8> p_asset_database, const uint32 p_width, const uint32 p_height)
    {
        this->is_running = 1;
        this->engine_execution_unit = p_engine_runner.allocate_engine_execution_unit(
            p_asset_database, p_width, p_height, EngineExternalStepCallback{(void*)this, (EngineExternalStepCallback::cb_t)MaterialViewerEngineUnit::step},
            EngineExecutionUnit::CleanupCallback{(void*)this, (EngineExecutionUnit::CleanupCallback::cb_t)MaterialViewerEngineUnit::cleanup_ressources});
    };

    inline void stop(EngineRunnerThread& p_engine_runner)
    {
        if (this->is_running)
        {
            p_engine_runner.free_engine_execution_unit_sync(this->engine_execution_unit);
            this->set_sefault_values();
        }
    };

    inline void restart(EngineRunnerThread& p_engine_runner, const Slice<int8> p_asset_database, const uint32 p_width, const uint32 p_height)
    {
        this->stop(p_engine_runner);
        this->start(p_engine_runner, p_asset_database, p_width, p_height);
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

  private:
    inline static void step(const EngineExternalStep p_step, Engine& p_engine, MaterialViewerEngineUnit* thiz)
    {
        if (p_step == EngineExternalStep::BEFORE_UPDATE)
        {
            float32 l_deltatime = DeltaTime(p_engine);
            uimax l_frame_count = FrameCount(p_engine);
            if (l_frame_count == 1)
            {
                thiz->camera_node = CreateNode(p_engine, transform_const::ORIGIN);
                thiz->material_node = CreateNode(p_engine, transform{v3f{0.0f, 0.0f, 5.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3});
                NodeAddCamera(p_engine, thiz->camera_node, CameraComponent::Asset{1.0f, 30.0f, 45.0f});
            }
            thiz->shared.acquire([&](SharedRessources& p_shared){
              if (p_shared.change_requested)
              {
                  if (token_value(thiz->material_node_meshrenderer) != -1)
                  {
                      NodeRemoveMeshRenderer(p_engine, thiz->material_node);
                  }
                  thiz->material_node_meshrenderer = NodeAddMeshRenderer(p_engine, thiz->material_node, p_shared.material_hash, p_shared.mesh_hash);
                  p_shared.change_requested = 0;
              }
            });


            NodeAddWorldRotation(p_engine, thiz->material_node, quat::rotate_around(v3f_const::UP, 3 * l_deltatime));
        }
    };

    inline static void cleanup_ressources(Engine& p_engine, MaterialViewerEngineUnit* thiz)
    {
        RemoveNode(p_engine, thiz->camera_node);
        RemoveNode(p_engine, thiz->material_node);
        thiz->material_node_meshrenderer = token_build_default<MeshRendererComponent>();
        thiz->is_running = 0;
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
        QLabel* db_status_label;

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
        this->widgets.db_file_selection->setMaximumWidth(20);
        this->widgets.db_file_label = new QLabel();
        this->widgets.db_file_label->setWordWrap(1);
        this->widgets.db_status_label = new QLabel();
        // this->widgets.db_file_label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

        this->widgets.db_layout = QLayoutWidget<QHBoxLayout, QWidget>::allocate();
        // this->widgets.db_layout.layout.exp
        // this->widgets.db_layout.layout->size

        this->widgets.selected_material_root = QLayoutWidget<QVBoxLayout, QGroupBox>::allocate();
        this->widgets.selected_material_root.widget->setTitle("material");
        this->widgets.selected_material = new QListWidget();

        this->widgets.selected_mesh_root = QLayoutWidget<QVBoxLayout, QGroupBox>::allocate();
        this->widgets.selected_mesh_root.widget->setTitle("mesh");
        this->widgets.selected_mesh = new QListWidget();

        this->widgets.main_layout = new QVBoxLayout(this->root);
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
        l_builder.add_widget(this->widgets.db_layout.widget);
        l_builder.add_widget(this->widgets.db_status_label);
        l_builder.add_widget(this->widgets.selected_material_root.widget);
        l_builder.add_widget(this->widgets.selected_mesh_root.widget);
    };

    inline void setup_widget_events(const Callbacks& p_callbacks)
    {
        this->callbacks = p_callbacks;

        this->widgets.db_status_label->setText("Waiting for asset database.");
        QObject::connect(this->widgets.db_file_selection, &QPushButton::released, [&]() {
            this->widgets.db_file_dialog = new QFileDialog(this->root);
            this->widgets.db_file_dialog->setNameFilters({"DB files (*.db)", "Any files (*)"});
            QObject::connect(this->widgets.db_file_dialog, &QFileDialog::fileSelected, [&, this](const QString& p_file) {
                if (this->on_database_file_selected(p_file))
                {
                    this->widgets.db_status_label->setText("Database recognized !");
                    this->widgets.db_status_label->setStyleSheet("QLabel { color: green; }");
                }
                else
                {
                    this->widgets.db_status_label->setText("Database invalid format !");
                    this->widgets.db_status_label->setStyleSheet("QLabel { color: red; }");
                }
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

    inline int8 on_database_file_selected(const QString& p_file)
    {
        this->view = View{};
        this->view.database_file = p_file;
        this->widgets.db_file_label->setText(this->view.database_file);

        QByteArray l_database_file_path_arr = this->view.database_file.toLocal8Bit();
        Slice<int8> l_database_file_path = slice_int8_build_rawstr(l_database_file_path_arr.data());
        DatabaseConnection l_connection = DatabaseConnection::allocate(l_database_file_path);
        if (DatabaseConnection_is_valid_silent(l_connection))
        {
            if (this->callbacks.on_database_selected)
            {
                this->callbacks.on_database_selected(this, this->callbacks.closure);
            }

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
                for (loop(i, 0, l_mesh_paths.data.Size))
                {
                    Span<int8>& l_path = l_mesh_paths.data.get(i);
                    QString l_str = QString::fromLocal8Bit(l_path.Memory, l_path.Capacity);
                    this->widgets.selected_mesh->addItem(l_str);
                }
                l_mesh_paths.free();
            }

            l_asset_metadata_database.free(l_connection);
            l_connection.free();

            return 1;
        }

        return 0;
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

struct MaterialViewerEditor
{
    EngineRunnerThread engine_runner;
    MaterialViewerEngineUnit material_viewer_engine_unit;
    MaterialViewerWindow material_viewer_window;

    inline void allocate()
    {
        this->material_viewer_engine_unit = MaterialViewerEngineUnit::allocate();
        this->engine_runner = EngineRunnerThread::allocate();
        this->engine_runner.start();

        MaterialViewerWindow::Callbacks l_callbacks;
        l_callbacks.closure = this;
        l_callbacks.on_database_selected = [](auto, void* p_editor) {
            MaterialViewerEditor* thiz = (MaterialViewerEditor*)p_editor;
            thiz->material_viewer_engine_unit.restart(thiz->engine_runner, slice_int8_build_rawstr(thiz->material_viewer_window.view.database_file.toLocal8Bit().data()), 400, 400);
        };
        l_callbacks.on_material_selected = [](auto, void* p_editor) {
            MaterialViewerEditor* thiz = (MaterialViewerEditor*)p_editor;
            thiz->try_to_update_material_or_mesh();
        };
        l_callbacks.on_mesh_selected = [](auto, void* p_editor) {
            MaterialViewerEditor* thiz = (MaterialViewerEditor*)p_editor;
            thiz->try_to_update_material_or_mesh();
        };
        this->material_viewer_window.allocate(l_callbacks);
    };

    inline void free()
    {
        this->material_viewer_engine_unit.free(this->engine_runner);
        this->engine_runner.free();
    };

    inline QWidget* root()
    {
        return this->material_viewer_window.root;
    };

  private:
    inline void try_to_update_material_or_mesh()
    {
        if (this->material_viewer_window.view.selected_material.isEmpty() || this->material_viewer_window.view.slected_mesh.isEmpty())
        {
            return;
        }

        this->material_viewer_engine_unit.set_new_material(HashSlice(slice_int8_build_rawstr(this->material_viewer_window.view.selected_material.toLocal8Bit().data())));
        this->material_viewer_engine_unit.set_new_mesh(HashSlice(slice_int8_build_rawstr(this->material_viewer_window.view.slected_mesh.toLocal8Bit().data())));
    };
};