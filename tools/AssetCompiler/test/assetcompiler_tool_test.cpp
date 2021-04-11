#include "AssetCompiler/asset_compiler_editor.hpp"

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

// TODO -> write test
/*
    1/ Loading configurations from file and ensure that the requested compilation pass have been taken into account
       Then execute compilation and assert result.
       Ensure that only the selected pass are processed.

    2/ Configuration file reloading.
*/

/*
    4 asset passes, first two selected, 1 error
*/
inline void compilation_execution(qt_test& p_qt_test)
{
    AssetCompilerEditor l_asset_compiler_editor{};
    l_asset_compiler_editor.allocate();
    AssetCompilationPassViewer& l_asset_compilationpass_viewer = l_asset_compiler_editor.window.widgets.asset_compilation_viewer;

    Span<int8> l_config_0_database = Span<int8>::allocate_slice_2(slice_int8_build_rawstr(ASSET_FOLDER_ROOT), slice_int8_build_rawstr("/config_0/asset.db"));
    Span<int8> l_config_1_database = Span<int8>::allocate_slice_2(slice_int8_build_rawstr(ASSET_FOLDER_ROOT), slice_int8_build_rawstr("/config_1/asset.db"));
    {
        File l_database_0_file = File::create_or_open(l_config_0_database.slice);
        l_database_0_file.erase();
    }
    {
        File l_database_0_file = File::create_or_open(l_config_1_database.slice);
        l_database_0_file.erase();
    }


    QTimer l_main_loop = QTimer();
    l_main_loop.setInterval(0);

    uimax l_frame_count = 0;
    QObject::connect(&l_main_loop, &QTimer::timeout, [&]() {
        if (l_frame_count == 0)
        {
        }
        else if (l_frame_count == 1)
        {
            l_asset_compiler_editor.window.widgets.feed_tree_button->click();
        }
        else if (l_frame_count == 2)
        {
            Span<int8> l_configuration_path = Span<int8>::allocate_slice_2(slice_int8_build_rawstr(ASSET_FOLDER_ROOT), slice_int8_build_rawstr("/test_config_file_1.json"));
            QFileDialog_simulate_pick(l_asset_compiler_editor.window.widgets.feed_tree_file_dialog, l_configuration_path.slice);
            l_configuration_path.free();
            l_asset_compiler_editor.window.widgets.feed_tree_file_dialog->close();
            assert_true(l_asset_compilationpass_viewer.widgets.compilation_pass_list.get_size() == 4);
            for (loop(i, 0, 4))
            {
                if (i < 2)
                {
                    l_asset_compilationpass_viewer.widgets.compilation_pass_list.get_widget_item(i)->setSelected(1);
                }
            }
            assert_true(l_asset_compilationpass_viewer.widgets.compilation_pass_list.selected_items.Size == 2);

            {
                AssetCompilationPassWidget& l_allocationpass_widget_0 =
                    l_asset_compilationpass_viewer.get_asset_compilationpass_widget_from_selecteditem_index(l_asset_compiler_editor.window.widget_heap, 0);
                assert_true(l_allocationpass_widget_0.items.Size == 2);
            }
            {
                AssetCompilationPassWidget& l_allocationpass_widget_1 =
                    l_asset_compilationpass_viewer.get_asset_compilationpass_widget_from_selecteditem_index(l_asset_compiler_editor.window.widget_heap, 1);
                assert_true(l_allocationpass_widget_1.items.Size == 4);
            }

            l_asset_compiler_editor.window.widgets.go_button->click();
        }
        else if (l_frame_count == 3)
        {
            l_asset_compiler_editor.compilation_thread.sync_wait_for_processing_compilation_events();
        }
        else if (l_frame_count == 4)
        {
            l_asset_compiler_editor.compilation_thread.output_result.acquire([&](AssetCompilationThread::s_output_result& p_trhead_output_result) {
                {
                    AssetCompilationPassWidget& l_allocationpass_widget_0 =
                        l_asset_compilationpass_viewer.get_asset_compilationpass_widget_from_selecteditem_index(l_asset_compiler_editor.window.widget_heap, 0);
                    assert_true(l_allocationpass_widget_0.items.get(0).compilation_state == AssetCompilationPassWidget::Item::CompilationState::SUCCESS);
                    assert_true(l_allocationpass_widget_0.items.get(1).compilation_state == AssetCompilationPassWidget::Item::CompilationState::SUCCESS);
                }
                {
                    AssetCompilationPassWidget& l_allocationpass_widget_1 =
                        l_asset_compilationpass_viewer.get_asset_compilationpass_widget_from_selecteditem_index(l_asset_compiler_editor.window.widget_heap, 1);
                    assert_true(l_allocationpass_widget_1.items.get(0).compilation_state == AssetCompilationPassWidget::Item::CompilationState::SUCCESS);
                    assert_true(l_allocationpass_widget_1.items.get(1).compilation_state == AssetCompilationPassWidget::Item::CompilationState::SUCCESS);
                    assert_true(l_allocationpass_widget_1.items.get(2).compilation_state == AssetCompilationPassWidget::Item::CompilationState::FAILURE);
                    assert_true(l_allocationpass_widget_1.items.get(3).compilation_state == AssetCompilationPassWidget::Item::CompilationState::SUCCESS);
                }

                // Span<int8>::allocate_slice_2(slice_int8_build_rawstr(ASSET_FOLDER_ROOT), slice_int8_build_rawstr("/config_0/asset.db"));
            });
        }
        // config file reloading
        else if (l_frame_count == 5)
        {
            Span<int8> l_configuration_path = Span<int8>::allocate_slice_2(slice_int8_build_rawstr(ASSET_FOLDER_ROOT), slice_int8_build_rawstr("/test_config_file_1.json"));
            QFileDialog_simulate_pick(l_asset_compiler_editor.window.widgets.feed_tree_file_dialog, l_configuration_path.slice);
            l_configuration_path.free();
            l_asset_compiler_editor.window.widgets.feed_tree_file_dialog->close();
            assert_true(l_asset_compilationpass_viewer.widgets.compilation_pass_list.get_size() == 4);
            assert_true(l_asset_compilationpass_viewer.widgets.compilation_pass_list.selected_items.Size == 0);

            l_asset_compiler_editor.compilation_thread.output_result.acquire([&](AssetCompilationThread::s_output_result& p_trhead_output_result) {
                {
                    AssetCompilationPassWidget& l_allocationpass_widget_0 =
                        l_asset_compilationpass_viewer.get_asset_compilationpass_widget_from_heap_index(l_asset_compiler_editor.window.widget_heap, 0);
                    assert_true(l_allocationpass_widget_0.items.get(0).compilation_state == AssetCompilationPassWidget::Item::CompilationState::NONE);
                    assert_true(l_allocationpass_widget_0.items.get(1).compilation_state == AssetCompilationPassWidget::Item::CompilationState::NONE);
                }
                {
                    AssetCompilationPassWidget& l_allocationpass_widget_1 =
                        l_asset_compilationpass_viewer.get_asset_compilationpass_widget_from_heap_index(l_asset_compiler_editor.window.widget_heap, 1);
                    assert_true(l_allocationpass_widget_1.items.get(0).compilation_state == AssetCompilationPassWidget::Item::CompilationState::NONE);
                    assert_true(l_allocationpass_widget_1.items.get(1).compilation_state == AssetCompilationPassWidget::Item::CompilationState::NONE);
                    assert_true(l_allocationpass_widget_1.items.get(2).compilation_state == AssetCompilationPassWidget::Item::CompilationState::NONE);
                    assert_true(l_allocationpass_widget_1.items.get(3).compilation_state == AssetCompilationPassWidget::Item::CompilationState::NONE);
                }
            });
        }
        else
        {
            p_qt_test.close();
        }
        l_frame_count += 1;
    });
    l_main_loop.start();
    p_qt_test.start(l_asset_compiler_editor.root());

    File l_database_0_file = File::open(l_config_0_database.slice);
    assert_true(l_database_0_file.is_valid());
    l_database_0_file.erase();
    l_config_0_database.free();

    File l_database_1_file = File::open_silent(l_config_1_database.slice);
    assert_true(!l_database_1_file.is_valid());
    l_database_1_file.free();
    l_config_1_database.free();

    l_asset_compiler_editor.free();
};

int main(int argc, char* argv[])
{
    qt_test app = qt_test::allocate(argc, argv);

    compilation_execution(app);

    app.free();
    memleak_ckeck();
    return 0;
}