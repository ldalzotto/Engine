#pragma once

#include "QTCommon/qt_common.hpp"
#include "AssetCompiler/asset_compiler.hpp"

struct AssetCompilationPassStatic
{
    String root_path;
    Vector<String> assets_to_compile;

    inline static AssetCompilationPassStatic build(const String& p_root_path, const Vector<String>& p_asset_to_compile)
    {
        return AssetCompilationPassStatic{p_root_path, p_asset_to_compile};
    };

    inline static AssetCompilationPassStatic allocate_default()
    {
        return AssetCompilationPassStatic::build(String::allocate(0), Vector<String>::allocate(0));
    };

    inline void free()
    {
        this->root_path.free();
        for (loop(i, 0, this->assets_to_compile.Size))
        {
            this->assets_to_compile.get(i).free();
        }
        this->assets_to_compile.free();
    };
};

struct AssetCompilationPass
{
    String database_path;
    String root_path;
    Vector<String> assets_to_compile;

    inline static AssetCompilationPass build(const String& p_database_path, const String& p_root_path, const Vector<String>& p_asset_to_compile)
    {
        return AssetCompilationPass{p_database_path, p_root_path, p_asset_to_compile};
    };

    inline static AssetCompilationPass allocate_default()
    {
        return AssetCompilationPass::build(String::allocate(0), String::allocate(0), Vector<String>::allocate(0));
    };

    inline static AssetCompilationPass allocate_from_static(const String& p_database_path, const AssetCompilationPassStatic& p_static)
    {
        Vector<String> l_assets_to_compile = Vector<String>::allocate_elements(p_static.assets_to_compile.to_slice());
        for (loop(i, 0, l_assets_to_compile.Size))
        {
            l_assets_to_compile.get(i) = String::allocate_elements(p_static.assets_to_compile.get(i).to_slice());
        }
        return AssetCompilationPass::build(p_database_path, String::allocate_elements(p_static.root_path.to_slice()), l_assets_to_compile);
    };

    inline void free()
    {
        this->database_path.free();
        this->root_path.free();
        for (loop(i, 0, this->assets_to_compile.Size))
        {
            this->assets_to_compile.get(i).free();
        }
        this->assets_to_compile.free();
    };
};

struct AssetSingleCompilationToken
{
    Token(AssetCompilationPass) pass_token;
    uimax index;
};

struct AssetCompilationThread
{
    thread_t thread;
    int8* thread_input_args;
    Thread::MainInput thread_input;

    ShaderCompiler shader_compiler;
    int8 ask_exit;
    int8 is_running;

    struct s_input_asset_compilation_pass_event
    {
        AssetCompilationPass pass;
        Token(AssetCompilationPass) pass_token;
    };

    struct s_input_events
    {
        Vector<s_input_asset_compilation_pass_event> compilation_passes;
    };

    MutexNative<s_input_events> input_events;

    struct s_asset_compilation_result
    {
        AssetSingleCompilationToken token;
        int8 result;
    };

    struct s_output_result
    {
        Vector<s_asset_compilation_result> compilation_results;
    };

    MutexNative<s_output_result> output_result;

    inline static AssetCompilationThread allocate()
    {
        AssetCompilationThread l_thread = AssetCompilationThread{};
        l_thread.input_events = MutexNative<s_input_events>::allocate();
        l_thread.output_result = MutexNative<s_output_result>::allocate();
        l_thread.shader_compiler = ShaderCompiler::allocate();
        return l_thread;
    };

    inline void free()
    {
        if (this->is_running)
        {
            this->stop();
        }

#if __DEBUG
        assert_true(this->input_events._data.compilation_passes.empty());
        assert_true(this->output_result._data.compilation_results.empty());
#endif

        this->input_events.free();
        this->input_events._data.compilation_passes.free();
        this->output_result.free();
        this->output_result._data.compilation_results.free();
        this->shader_compiler.free();
    };

    inline void start()
    {
        this->thread_input_args = (int8*)this;
        this->thread_input = Thread::MainInput{AssetCompilationThread::main, Slice<int8*>::build_begin_end(&this->thread_input_args, 0, 1)};
        this->thread = Thread::spawn_thread(this->thread_input);
        this->is_running = 1;
    };

    inline void pause(){

    };

    inline void unpause(){

    };

    inline void stop()
    {
        this->ask_exit = 1;
        Thread::wait_for_end_and_terminate(this->thread, -1);
        this->is_running = 0;
    };

    inline void push_asset_compilation_pass(const Token(AssetCompilationPass) p_asset_compilation_pass_token, const AssetCompilationPass& p_asset_compilation_pass)
    {
        this->input_events.acquire([&](s_input_events& p_input_events) {
            p_input_events.compilation_passes.push_back_element(s_input_asset_compilation_pass_event{p_asset_compilation_pass, p_asset_compilation_pass_token});
        });
    };

    inline static int8 main(const Slice<int8*>& p_args)
    {
        AssetCompilationThread* thiz = (AssetCompilationThread*)p_args.get(0);
        return thiz->_main();
    };

  private:
    inline int8 _main()
    {
        while (!this->ask_exit)
        {
            if (this->input_events._data.compilation_passes.Size > 0)
            {
                this->input_events.acquire([&](s_input_events& p_input_events) {
                    for (loop(i, 0, p_input_events.compilation_passes.Size))
                    {
                        this->compile_asset_compilation_pass(p_input_events.compilation_passes.get(i));
                    }
                    p_input_events.compilation_passes.clear();
                });
            }
        }
        return 0;
    };

    inline void compile_asset_compilation_pass(const s_input_asset_compilation_pass_event& p_asset_compilation_pass)
    {
        DatabaseConnection l_database_connection = DatabaseConnection::allocate(p_asset_compilation_pass.pass.database_path.to_slice());

        AssetDatabase::initialize_database(l_database_connection);
        AssetMetadataDatabase::initialize_database(l_database_connection);

        AssetDatabase l_asset_database = AssetDatabase::allocate(l_database_connection);
        AssetMetadataDatabase l_asset_metadata_database = AssetMetadataDatabase::allocate(l_database_connection);

        for (loop(i, 0, p_asset_compilation_pass.pass.assets_to_compile.Size))
        {
            int8 l_result =
                AssetCompiler_compile_and_push_to_database_single_file(this->shader_compiler, l_database_connection, l_asset_database, l_asset_metadata_database,
                                                                       p_asset_compilation_pass.pass.root_path.to_slice(), p_asset_compilation_pass.pass.assets_to_compile.get(i).to_slice());
            this->output_result.acquire([&](s_output_result& p_output_result) {
                p_output_result.compilation_results.push_back_element(s_asset_compilation_result{AssetSingleCompilationToken{p_asset_compilation_pass.pass_token, i}, l_result});
            });
        }

        l_asset_database.free(l_database_connection);
        l_asset_metadata_database.free(l_database_connection);
        l_database_connection.free();
    };
};

struct AssetCompilerHeap
{
    Pool<AssetCompilationPass> asset_compilation_passes;
    Vector<AssetCompilationPassStatic> static_compilation_passes;

    inline static AssetCompilerHeap allocate(const Slice<AssetCompilationPassStatic>& p_static_compilation_passes)
    {
        return AssetCompilerHeap{Pool<AssetCompilationPass>::allocate(0), Vector<AssetCompilationPassStatic>::allocate_elements(p_static_compilation_passes)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(!this->asset_compilation_passes.has_allocated_elements());
#endif
        this->asset_compilation_passes.free();
        for (loop(i, 0, this->static_compilation_passes.Size))
        {
            this->static_compilation_passes.get(i).free();
        }
        this->static_compilation_passes.free();
    };

    inline void free_asset_compiler_pass(const Token(AssetCompilationPass) p_asset_compiler_pass_token)
    {
        this->asset_compilation_passes.get(p_asset_compiler_pass_token).free();
        this->asset_compilation_passes.release_element(p_asset_compiler_pass_token);
    };
};

struct AssetCompilationPassWidget
{
    QWidget* root;

    struct Widgets
    {
        QVBoxLayout* main_layout;
        QLabel* database_label;
        QListWidget* assets_to_compile;
    } widgets;

    Token<AssetCompilationPass> asset_compilation_pass_token;

    struct Item
    {
        QListWidgetItem* widget;

        inline void set_compilation_result(int8 p_result)
        {
            if (p_result)
            {
                this->widget->setBackground(QBrush(QColor(171, 255, 171)));
            }
            else
            {
                this->widget->setBackground(QBrush(QColor(255, 171, 171)));
            }
        };
    };

    Vector<Item> items;

    inline static AssetCompilationPassWidget allocate_empty()
    {
        AssetCompilationPassWidget l_widget{};
        l_widget.root = new QWidget();
        l_widget.widgets.main_layout = new QVBoxLayout(l_widget.root);
        l_widget.widgets.database_label = new QLabel();
        l_widget.widgets.assets_to_compile = new QListWidget();
        l_widget.items = Vector<Item>::allocate(0);

        QLayoutBuilder l_layout_bulder;
        l_layout_bulder.bind_layout(l_widget.widgets.main_layout);
        l_layout_bulder.add_widget(l_widget.widgets.database_label);
        l_layout_bulder.add_widget(l_widget.widgets.assets_to_compile);

        return l_widget;
    };

    inline void free()
    {
        this->items.free();
    };

    inline int8 empty()
    {
        return this->items.empty();
    };

    inline void feed_with_compilation_pass(const Token<AssetCompilationPass> p_asset_compilation_pass_token, const AssetCompilationPass& p_asset_compilation_pass)
    {
        if (!this->empty())
        {
            this->widgets.assets_to_compile->clear();
            this->items.clear();
        }

        this->asset_compilation_pass_token = p_asset_compilation_pass_token;
        this->widgets.database_label->setText(p_asset_compilation_pass.database_path.to_slice().Begin);

        for (loop(i, 0, p_asset_compilation_pass.assets_to_compile.Size))
        {
            QListWidgetItem* l_item = new QListWidgetItem();
            l_item->setText(p_asset_compilation_pass.assets_to_compile.get(i).to_slice().Begin);
            this->widgets.assets_to_compile->addItem(l_item);
            this->items.push_back_element(Item{l_item});
        }
    };

    inline void set_asset_compilation_result(const uimax p_asset_index, const int8 p_result)
    {
        Item& l_item = this->items.get(p_asset_index);
        l_item.set_compilation_result(p_result);
    };
};

struct AssetCompilerWindow
{
    QWidget* root;

    struct Widgets
    {
        QVBoxLayout* main_layout;
        QPushButton* feed_tree_button;
        QPushButton* go_button;
        Vector<AssetCompilationPassWidget> asset_compilation_passes;
    } widgets;

    struct Callbacks
    {
        void* closure;
        void (*load_asset_tree)(AssetCompilerWindow* p_window, Vector<Token(AssetCompilationPass)>* in_out_asset_compilation_pass, void* p_closure);
        void (*on_go_button_pushed)(AssetCompilerWindow* p_window, void* p_closure);
    } callbacks;

    Vector<Token(AssetCompilationPass)> callbacks_asset_compilation_pass_buffer;

    struct View
    {
        Vector<Token(AssetCompilationPass)> compilation_passes;
    } view;

    inline void allocate(AssetCompilerHeap& p_asset_compiler_heap, const Callbacks& p_callbacks)
    {
        this->callbacks_asset_compilation_pass_buffer = Vector<Token(AssetCompilationPass)>::allocate(0);
        this->view.compilation_passes = Vector<Token(AssetCompilationPass)>::allocate(0);
        this->allocate_widgets();
        this->setup_widget_layout();
        this->setup_widget_events(p_callbacks, p_asset_compiler_heap);
    };

    inline void free(AssetCompilerHeap& p_asset_compiler_heap)
    {
        for (loop(i, 0, this->view.compilation_passes.Size))
        {
            this->widgets.asset_compilation_passes.get(i).free();
            p_asset_compiler_heap.free_asset_compiler_pass(this->view.compilation_passes.get(i));
        }

        this->widgets.asset_compilation_passes.free();
        this->view.compilation_passes.free();
        this->callbacks_asset_compilation_pass_buffer.free();
    };

    inline void set_compilation_asset_result(const AssetSingleCompilationToken p_asset_compilation_token, const int8 p_result)
    {
        for (loop(i, 0, this->view.compilation_passes.Size))
        {
            if (tk_eq(this->view.compilation_passes.get(i), p_asset_compilation_token.pass_token))
            {
                this->widgets.asset_compilation_passes.get(i).set_asset_compilation_result(p_asset_compilation_token.index, p_result);
                break;
            }
        }
    };

  private:
    inline void allocate_widgets()
    {
        this->root = new QWidget();
        this->widgets.main_layout = new QVBoxLayout(this->root);

        this->widgets.feed_tree_button = new QPushButton();
        this->widgets.feed_tree_button->setText("FEED");

        this->widgets.go_button = new QPushButton();
        this->widgets.go_button->setText("GO");

        this->widgets.asset_compilation_passes = Vector<AssetCompilationPassWidget>::allocate(0);
    };

    inline void setup_widget_layout()
    {
        QLayoutBuilder l_layout_builder;
        l_layout_builder.bind_layout(this->widgets.main_layout);
        l_layout_builder.add_widget(this->widgets.feed_tree_button);
        l_layout_builder.add_widget(this->widgets.go_button);
    };

    inline void setup_widget_events(const Callbacks& p_callbacks, AssetCompilerHeap& p_asset_compiler_heap)
    {
        this->callbacks = p_callbacks;
        QObject::connect(this->widgets.feed_tree_button, &QPushButton::released, [&]() {
            this->callbacks.load_asset_tree(this, &this->callbacks_asset_compilation_pass_buffer, this->callbacks.closure);
            for (loop(i, 0, this->callbacks_asset_compilation_pass_buffer.Size))
            {
                Token(AssetCompilationPass) l_pass = this->callbacks_asset_compilation_pass_buffer.get(i);
                AssetCompilationPassWidget l_compilation_pass_widget = AssetCompilationPassWidget::allocate_empty();
                this->widgets.main_layout->addWidget(l_compilation_pass_widget.root);
                l_compilation_pass_widget.feed_with_compilation_pass(l_pass, p_asset_compiler_heap.asset_compilation_passes.get(l_pass));
                this->widgets.asset_compilation_passes.push_back_element(l_compilation_pass_widget);
                this->view.compilation_passes.push_back_element(l_pass);
            }
            this->callbacks_asset_compilation_pass_buffer.clear();
        });
        QObject::connect(this->widgets.go_button, &QPushButton::released, [&]() {
            this->callbacks.on_go_button_pushed(this, this->callbacks.closure);
        });
    };
};

struct AssetCompilerComposition
{

    inline static void push_selected_assetcompilation_pass_to_thread(AssetCompilerWindow& p_window, AssetCompilerHeap& p_heap, AssetCompilationThread& p_thread)
    {
        for (loop(i, 0, p_window.view.compilation_passes.Size))
        {
            p_thread.push_asset_compilation_pass(p_window.view.compilation_passes.get(i), p_heap.asset_compilation_passes.get(p_window.view.compilation_passes.get(i)));
        }
    };

    inline static void try_consume_output_thread_events(AssetCompilationThread& p_thread, AssetCompilerWindow& p_window)
    {
        if (p_thread.output_result._data.compilation_results.Size > 0)
        {
            p_thread.output_result.acquire([&](AssetCompilationThread::s_output_result& p_output_result) {
                for (loop(i, 0, p_output_result.compilation_results.Size))
                {
                    auto& l_result = p_output_result.compilation_results.get(i);
                    p_window.set_compilation_asset_result(l_result.token, l_result.result);
                }

                p_output_result.compilation_results.clear();
            });
        }
    };
};

struct AssetCompilerEditor
{
    AssetCompilerHeap heap;
    AssetCompilerWindow window;
    AssetCompilationThread compilation_thread;
    QTimer* compilation_thread_output_consumer;

    inline QWidget* root()
    {
        return this->window.root;
    };

    inline void allocate()
    {
        SliceN<String, 2> l_static_assets =
            SliceN<String, 2>{String::allocate_elements(slice_int8_build_rawstr("internal/quad_blit.vert")), String::allocate_elements(slice_int8_build_rawstr("internal/quad_blit.frag"))};
        SliceN<AssetCompilationPassStatic, 1> l_static_pass = SliceN<AssetCompilationPassStatic, 1>{AssetCompilationPassStatic::build(
            String::allocate_elements(slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/_asset/asset/common/")), Vector<String>::allocate_elements(slice_from_slicen(&l_static_assets)))};

        this->heap = AssetCompilerHeap::allocate(slice_from_slicen(&l_static_pass));

        AssetCompilerWindow::Callbacks l_asset_compiler_window_cb{};
        l_asset_compiler_window_cb.closure = this;
        l_asset_compiler_window_cb.load_asset_tree = [](AssetCompilerWindow* p_window, Vector<Token(AssetCompilationPass)>* in_out_asset_compilation_pass, void* p_closure) {
            AssetCompilerEditor* thiz = (AssetCompilerEditor*)p_closure;
            {
                SliceN<String, 13> l_assets =
                    SliceN<String, 13>{String::allocate_elements(slice_int8_build_rawstr("grad_1.png")),      String::allocate_elements(slice_int8_build_rawstr("grad_2.png")),
                                       String::allocate_elements(slice_int8_build_rawstr("grad_3.png")),      String::allocate_elements(slice_int8_build_rawstr("grad_4.png")),
                                       String::allocate_elements(slice_int8_build_rawstr("material_1.json")), String::allocate_elements(slice_int8_build_rawstr("material_2.json")),
                                       String::allocate_elements(slice_int8_build_rawstr("material_3.json")), String::allocate_elements(slice_int8_build_rawstr("material_4.json")),
                                       String::allocate_elements(slice_int8_build_rawstr("cone.obj")),        String::allocate_elements(slice_int8_build_rawstr("cube.obj")),
                                       String::allocate_elements(slice_int8_build_rawstr("cylinder.obj")),    String::allocate_elements(slice_int8_build_rawstr("icosphere.obj")),
                                       String::allocate_elements(slice_int8_build_rawstr("sphere.obj"))};
                in_out_asset_compilation_pass->push_back_element(thiz->heap.asset_compilation_passes.alloc_element(
                    AssetCompilationPass::build(String::allocate_elements(slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/_asset/asset/MaterialViewerTest/asset.db")),
                                                String::allocate_elements(slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/_asset/asset/MaterialViewerTest/")),
                                                Vector<String>::allocate_elements(slice_from_slicen(&l_assets)))));

                for (loop(i, 0, thiz->heap.static_compilation_passes.Size))
                {
                    in_out_asset_compilation_pass->push_back_element(thiz->heap.asset_compilation_passes.alloc_element(AssetCompilationPass::allocate_from_static(
                        String::allocate_elements(slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/_asset/asset/MaterialViewerTest/asset.db")), thiz->heap.static_compilation_passes.get(i))));
                }
            }
            {
                SliceN<String, 13> l_assets =
                    SliceN<String, 13>{String::allocate_elements(slice_int8_build_rawstr("grad_1.png")),      String::allocate_elements(slice_int8_build_rawstr("grad_2.png")),
                                       String::allocate_elements(slice_int8_build_rawstr("grad_3.png")),      String::allocate_elements(slice_int8_build_rawstr("grad_4.png")),
                                       String::allocate_elements(slice_int8_build_rawstr("material_1.json")), String::allocate_elements(slice_int8_build_rawstr("material_2.json")),
                                       String::allocate_elements(slice_int8_build_rawstr("material_3.json")), String::allocate_elements(slice_int8_build_rawstr("material_4.json")),
                                       String::allocate_elements(slice_int8_build_rawstr("cone.obj")),        String::allocate_elements(slice_int8_build_rawstr("cube.obj")),
                                       String::allocate_elements(slice_int8_build_rawstr("cylinder.obj")),    String::allocate_elements(slice_int8_build_rawstr("icosphere.obj")),
                                       String::allocate_elements(slice_int8_build_rawstr("sphere.obj"))};
                in_out_asset_compilation_pass->push_back_element(thiz->heap.asset_compilation_passes.alloc_element(
                    AssetCompilationPass::build(String::allocate_elements(slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/_asset/asset/MaterialViewerTest/asset_2.db")),
                                                String::allocate_elements(slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/_asset/asset/MaterialViewerTest/")),
                                                Vector<String>::allocate_elements(slice_from_slicen(&l_assets)))));

                for (loop(i, 0, thiz->heap.static_compilation_passes.Size))
                {
                    in_out_asset_compilation_pass->push_back_element(thiz->heap.asset_compilation_passes.alloc_element(
                        AssetCompilationPass::allocate_from_static(String::allocate_elements(slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/_asset/asset/MaterialViewerTest/asset_2.db")),
                                                                   thiz->heap.static_compilation_passes.get(i))));
                }
            }
        };
        l_asset_compiler_window_cb.on_go_button_pushed = [](AssetCompilerWindow* p_window, void* p_closure) {
            AssetCompilerEditor* thiz = (AssetCompilerEditor*)p_closure;
            AssetCompilerComposition::push_selected_assetcompilation_pass_to_thread(thiz->window, thiz->heap, thiz->compilation_thread);
        };
        this->window.allocate(this->heap, l_asset_compiler_window_cb);

        this->compilation_thread = AssetCompilationThread::allocate();
        this->compilation_thread.start();

        this->compilation_thread_output_consumer = new QTimer();
        this->compilation_thread_output_consumer->setInterval(0);
        QObject::connect(this->compilation_thread_output_consumer, &QTimer::timeout, [&]() {
            AssetCompilerComposition::try_consume_output_thread_events(this->compilation_thread, this->window);
        });
        this->compilation_thread_output_consumer->start();
    };

    inline void free()
    {
        this->window.free(this->heap);
        this->compilation_thread.free();
        this->heap.free();
    };
};