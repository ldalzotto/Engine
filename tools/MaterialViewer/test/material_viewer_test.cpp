
#include "MaterialViewer/material_viewer_window.hpp"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QMainWindow* l_main_window;
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
            assert_true(tk_eq(l_material_viewer_editor.engine_thread.material_node, tk_bd(Node)));
            assert_true(tk_eq(l_material_viewer_editor.engine_thread.camera_node, tk_bd(Node)));
            assert_true(tk_eq(l_material_viewer_editor.engine_thread.material_node_meshrenderer, tk_bd(MeshRendererComponent)));

            QFileDialog* fd = l_material_viewer_editor.material_viewer.widgets.db_file_dialog;
            Slice<int8> l_file_path = slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/_asset/asset/MaterialViewer/asset.db");
            fd->fileSelected(QString::fromLocal8Bit(l_file_path.Begin, l_file_path.Size));
            assert_true(slice_int8_build_rawstr(l_material_viewer_editor.material_viewer.view.database_file.toLocal8Bit().data()).compare(l_file_path));
            fd->close();

            l_material_viewer_editor.engine_thread.wait_for_engine_spawned();

            assert_true(l_material_viewer_editor.material_viewer.widgets.selected_mesh->count() == 3);
            assert_true(l_material_viewer_editor.material_viewer.widgets.selected_material->count() == 3);

            l_material_viewer_editor.material_viewer.widgets.selected_mesh->setCurrentRow(0);
            l_material_viewer_editor.material_viewer.widgets.selected_material->setCurrentRow(0);

            l_material_viewer_editor.engine_thread.wait_for_atleast_a_single_frame();
            l_material_viewer_editor.engine_thread.wait_for_atleast_a_single_frame();

            // TODO -> adding better synchronisation option for EngineThread
            l_material_viewer_editor.engine_thread.stop_at_end_of_frame = 1;
            while (!l_material_viewer_editor.engine_thread.is_stopped)
            {
            }

            assert_true(slice_int8_build_rawstr(l_material_viewer_editor.material_viewer.view.selected_material.toLocal8Bit().data()).compare(slice_int8_build_rawstr("material_1.json")));
            assert_true(slice_int8_build_rawstr(l_material_viewer_editor.material_viewer.view.slected_mesh.toLocal8Bit().data()).compare(slice_int8_build_rawstr("shape_1.obj")));

            assert_true(tk_neq(l_material_viewer_editor.engine_thread.material_node, tk_bd(Node)));
            assert_true(tk_neq(l_material_viewer_editor.engine_thread.camera_node, tk_bd(Node)));
            assert_true(tk_neq(l_material_viewer_editor.engine_thread.material_node_meshrenderer, tk_bd(MeshRendererComponent)));

            l_material_viewer_editor.engine_thread.stop_at_end_of_frame = 0;
        }
        else if (l_frame_count == 3)
        {
            l_material_viewer_editor.engine_thread.stop_at_end_of_frame = 1;
            while (!l_material_viewer_editor.engine_thread.is_stopped)
            {
            }

            l_material_viewer_editor.material_viewer.widgets.selected_mesh->setCurrentRow(1);
            assert_true(slice_int8_build_rawstr(l_material_viewer_editor.material_viewer.view.slected_mesh.toLocal8Bit().data()).compare(slice_int8_build_rawstr("shape_2.obj")));

            assert_true(l_material_viewer_editor.engine_thread.shared._data.change_requested == 1);
            assert_true(l_material_viewer_editor.engine_thread.shared._data.mesh_hash == HashSlice(slice_int8_build_rawstr("shape_2.obj")));

            l_material_viewer_editor.engine_thread.stop_at_end_of_frame = 0;
        }
        else if (l_frame_count == 4)
        {
            l_material_viewer_editor.free();
            l_main_window->close();
        }
        l_frame_count += 1;
    });
    l_main_loop.start();
    int l_exec = qt_app_start(a, l_material_viewer_editor.root(), &l_main_window);
    l_material_viewer_editor.free();

    // TODO -> move this to a utility function ?
    // It is very important to ensure that all events are processed before testing another q application
    a.processEvents();
    a.exit(0);

    memleak_ckeck();

    return 0;
};