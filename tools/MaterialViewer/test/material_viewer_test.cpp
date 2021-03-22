
#include "MaterialViewer/material_viewer_window.hpp"

struct qt_test
{
    QApplication app;
    QMainWindow* main_window;

    inline static qt_test allocate(int argc, char* argv[])
    {
        return qt_test{QApplication(argc, argv), NULL};
    };

    inline int start(QWidget* p_central_widget)
    {
        return qt_app_start(this->app, p_central_widget, &this->main_window);
    };

    inline void close()
    {
        this->main_window->close();
    };

    inline void free()
    {
        this->app.processEvents();
        this->app.exit(0);
    };
};

inline void material_viewer(int argc, char* argv[])
{
    qt_test qt = qt_test::allocate(argc, argv);

    MaterialViewerEditor l_material_viewer_editor{};
    l_material_viewer_editor.allocate();

    QTimer l_main_loop = QTimer();
    l_main_loop.setInterval(0);

    uimax l_frame_count = 0;
    QObject::connect(&l_main_loop, &QTimer::timeout, [&]() {
        if (l_frame_count == 1)
        {
            l_material_viewer_editor.material_viewer.widgets.db_file_selection->click();
        }
        else if (l_frame_count == 2)
        {
            assert_true(tk_eq(l_material_viewer_editor.material_viewer_engine_unit.material_node, tk_bd(Node)));
            assert_true(tk_eq(l_material_viewer_editor.material_viewer_engine_unit.camera_node, tk_bd(Node)));
            assert_true(tk_eq(l_material_viewer_editor.material_viewer_engine_unit.material_node_meshrenderer, tk_bd(MeshRendererComponent)));

            QFileDialog* fd = l_material_viewer_editor.material_viewer.widgets.db_file_dialog;
            Span<int8> l_file_path = Span<int8>::allocate_slice_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("asset.db"));
            fd->fileSelected(QString::fromLocal8Bit(l_file_path.slice.Begin, l_file_path.slice.Size));
            assert_true(slice_int8_build_rawstr(l_material_viewer_editor.material_viewer.view.database_file.toLocal8Bit().data()).compare(l_file_path.slice));
            fd->close();
            l_file_path.free();

            EngineExecutionUnit* l_unit =
                l_material_viewer_editor.engine_runner.sync_wait_for_engine_execution_unit_to_be_allocated(l_material_viewer_editor.material_viewer_engine_unit.engine_execution_unit);

            assert_true(l_material_viewer_editor.material_viewer.widgets.selected_mesh->count() == 3);
            assert_true(l_material_viewer_editor.material_viewer.widgets.selected_material->count() == 3);

            l_material_viewer_editor.material_viewer.widgets.selected_mesh->setCurrentRow(0);
            l_material_viewer_editor.material_viewer.widgets.selected_material->setCurrentRow(0);

            l_unit->sync_wait_for_one_whole_frame_at_end_of_frame([&]() {
                assert_true(slice_int8_build_rawstr(l_material_viewer_editor.material_viewer.view.selected_material.toLocal8Bit().data()).compare(slice_int8_build_rawstr("material_1.json")));
                assert_true(slice_int8_build_rawstr(l_material_viewer_editor.material_viewer.view.slected_mesh.toLocal8Bit().data()).compare(slice_int8_build_rawstr("shape_1.obj")));

                assert_true(tk_neq(l_material_viewer_editor.material_viewer_engine_unit.material_node, tk_bd(Node)));
                assert_true(tk_neq(l_material_viewer_editor.material_viewer_engine_unit.camera_node, tk_bd(Node)));
                assert_true(tk_neq(l_material_viewer_editor.material_viewer_engine_unit.material_node_meshrenderer, tk_bd(MeshRendererComponent)));
            });
        }
        else if (l_frame_count == 3)
        {
            EngineExecutionUnit& l_unit = l_material_viewer_editor.engine_runner.engines.get(l_material_viewer_editor.material_viewer_engine_unit.engine_execution_unit);
            l_unit.sync_at_end_of_frame([&]() {
                l_material_viewer_editor.material_viewer.widgets.selected_mesh->setCurrentRow(1);
                assert_true(slice_int8_build_rawstr(l_material_viewer_editor.material_viewer.view.slected_mesh.toLocal8Bit().data()).compare(slice_int8_build_rawstr("shape_2.obj")));

                assert_true(l_material_viewer_editor.material_viewer_engine_unit.shared._data.change_requested == 1);
                assert_true(l_material_viewer_editor.material_viewer_engine_unit.shared._data.mesh_hash == HashSlice(slice_int8_build_rawstr("shape_2.obj")));
            });
        }
        else if (l_frame_count == 4)
        {
            l_material_viewer_editor.free();
            qt.close();
        }
        l_frame_count += 1;
    });
    l_main_loop.start();
    int l_exec = qt.start(l_material_viewer_editor.root());
    qt.free();
};

// TODO -> adding a test that close the material window manually before the application
// TODO -> there is an error telling that  when trying to run multiple test back to back
int main(int argc, char* argv[])
{
    material_viewer(argc, argv);
    memleak_ckeck();
    return 0;
};