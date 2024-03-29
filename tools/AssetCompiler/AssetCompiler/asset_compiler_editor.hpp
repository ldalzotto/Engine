#pragma once

#include "QTCommon/qt_include.hpp"
#include "AssetCompiler/asset_compiler.hpp"
#include "QTCommon/qt_utility.hpp"

/*
    A Token that identify a single asset compilation inside a AssetCompilationPass
 */
struct AssetSingleCompilationToken
{
    Token<AssetCompilationPass> pass_token;
    // The single asset compilation is represended by it's index
    uimax index;
};

struct AssetCompilationThread
{
    thread_native thread;

    struct Exec
    {
        AssetCompilationThread* thiz;
        inline int8 operator()() const
        {
            return AssetCompilationThread::main(thiz);
        };
    } exec;

    ShaderCompiler shader_compiler;

    struct RunningState_V2
    {
        enum class State
        {
            UNDEFINED = 0,
            RUNNING = 1,
            STOPPED = 2
        };

        inline static RunningState_V2 build()
        {
            RunningState_V2 l_s;
            l_s.state = State::UNDEFINED;
            return l_s;
        };

        inline int8 is_running()
        {
            return this->state == State::RUNNING;
        };

        inline void set_state(const State p_state)
        {
#if __DEBUG
            assert_true(this->state != p_state);
#endif
            this->state = p_state;
        };

      private:
        State state;

    } running_state_V2;

    struct s_input_asset_compilation_pass_event
    {
        AssetCompilationPass pass;
        Token<AssetCompilationPass> pass_token;
    };

    struct s_input_events
    {
        volatile uimax compilation_passes_size;
        Vector<s_input_asset_compilation_pass_event> compilation_passes;
    };

    Mutex<s_input_events> input_events;

    struct s_asset_compilation_result
    {
        AssetSingleCompilationToken token;
        int8 result;
    };

    struct s_output_result
    {
        Vector<s_asset_compilation_result> compilation_results;
    };

    Mutex<s_output_result> output_result;

    inline static AssetCompilationThread allocate()
    {
        AssetCompilationThread l_thread = AssetCompilationThread{};
        l_thread.input_events = Mutex<s_input_events>::allocate();
        l_thread.output_result = Mutex<s_output_result>::allocate();
        l_thread.shader_compiler = ShaderCompiler::allocate();
        l_thread.running_state_V2 = RunningState_V2::build();
        return l_thread;
    };

    inline void start()
    {
        this->running_state_V2.set_state(RunningState_V2::State::RUNNING);
        this->_start();
    };

    inline void start_if_not_running()
    {
        if (!this->running_state_V2.is_running())
        {
            this->start();
        }
    };

    inline void stop()
    {
        this->running_state_V2.set_state(RunningState_V2::State::STOPPED);
        this->_stop_and_wait();
    };

    inline void stop_if_running()
    {
        if (this->running_state_V2.is_running())
        {
            this->stop();
        }
    };

    inline void free()
    {
#if __DEBUG
        assert_true(!this->running_state_V2.is_running());
        assert_true(this->input_events._data.compilation_passes.empty());
        assert_true(this->output_result._data.compilation_results.empty());
#endif

        this->input_events.free();
        this->input_events._data.compilation_passes.free();
        this->output_result.free();
        this->output_result._data.compilation_results.free();
        this->shader_compiler.free();
    };

    inline int8 has_compilation_events()
    {
        return this->input_events._data.compilation_passes_size != 0;
    };

    inline void sync_wait_for_processing_compilation_events()
    {
        while (this->has_compilation_events())
        {
        }
    };

    inline void push_asset_compilation_pass(const Token<AssetCompilationPass> p_asset_compilation_pass_token, const AssetCompilationPass& p_asset_compilation_pass)
    {
        this->input_events.acquire([&](s_input_events& p_input_events) {
            if (this->running_state_V2.is_running() && !this->has_compilation_events())
            {
                this->stop();
            }

            p_input_events.compilation_passes.push_back_element(s_input_asset_compilation_pass_event{p_asset_compilation_pass, p_asset_compilation_pass_token});
            p_input_events.compilation_passes_size = p_input_events.compilation_passes.Size;
            this->start_if_not_running();
        });
    };

    inline static int8 main(AssetCompilationThread* thiz)
    {
        return thiz->_main();
    };

  private:
    inline int8 _main()
    {
        while (this->has_compilation_events())
        {
            this->input_events.acquire([&](s_input_events& p_input_events) {
                for (loop(i, 0, p_input_events.compilation_passes.Size))
                {
                    this->compile_asset_compilation_pass(p_input_events.compilation_passes.get(i));
                }
                p_input_events.compilation_passes.clear();
                p_input_events.compilation_passes_size = p_input_events.compilation_passes.Size;
            });
        }
        return 0;
    };

    inline void compile_asset_compilation_pass(const s_input_asset_compilation_pass_event& p_asset_compilation_pass)
    {
        AssetCompilerPassComposition::ExecutionState l_execution = AssetCompilerPassComposition::ExecutionState::allocate(this->shader_compiler, p_asset_compilation_pass.pass);
        for (loop(i, 0, l_execution.get_size()))
        {
            int8 l_result = l_execution.compile_single(i);
            this->output_result.acquire([&](s_output_result& p_output_result) {
                p_output_result.compilation_results.push_back_element(s_asset_compilation_result{AssetSingleCompilationToken{p_asset_compilation_pass.pass_token, i}, l_result});
            });
        }
        l_execution.free();
    };

    inline void _start()
    {
        this->exec = Exec{this};
        this->thread = Thread::spawn_thread(this->exec);
    };

    inline void _stop_and_wait()
    {
        Thread::wait_for_end_and_terminate(this->thread, -1);
    };
};

/*
    Widget that display all assets that are going to be compiled in a single pass.
*/
struct AssetCompilationPassWidget
{
    QWidget* root;

    struct Widgets
    {
        QVBoxLayout* main_layout;
        QLabel* database_label;
        QListWidget* assets_to_compile;
    } widgets;

    struct Item
    {
        QListWidgetItem* widget;

        enum class CompilationState
        {
            UNDEFINED = 0,
            NONE = 1,
            SUCCESS = 2,
            FAILURE = 3
        } compilation_state;

        inline void set_compilation_result(int8 p_result)
        {
            if (p_result)
            {
                this->compilation_state = CompilationState::SUCCESS;
                this->widget->setBackground(QBrush(QColor(171, 255, 171)));
            }
            else
            {
                this->compilation_state = CompilationState::FAILURE;
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

    inline void detach()
    {
        this->root->setParent(NULL);
    };

    inline void free()
    {
        this->detach();
        delete this->root;
        this->items.free();
    };

    inline int8 empty()
    {
        return this->items.empty();
    };

    inline void feed_with_compilation_pass(const AssetCompilationPass& p_asset_compilation_pass)
    {
        if (!this->empty())
        {
            this->widgets.assets_to_compile->clear();
            this->items.clear();
        }

        this->widgets.database_label->setText(p_asset_compilation_pass.database_path.to_slice().Begin);

        for (loop(i, 0, p_asset_compilation_pass.assets_to_compile.Size))
        {
            QListWidgetItem* l_item = new QListWidgetItem();
            l_item->setText(p_asset_compilation_pass.assets_to_compile.get(i).to_slice().Begin);
            this->widgets.assets_to_compile->addItem(l_item);
            this->items.push_back_element(Item{l_item, Item::CompilationState::NONE});
        }
    };

    inline void set_asset_compilation_result(const uimax p_asset_index, const int8 p_result)
    {
        Item& l_item = this->items.get(p_asset_index);
        l_item.set_compilation_result(p_result);
    };
};

struct AssetCompilerWidgetHeap
{
    Pool<AssetCompilationPassWidget> asset_compilation_passes;
    PoolIndexed<Token<AssetCompilationPass>> asset_compilation_passes_compilationtoken;

    inline static AssetCompilerWidgetHeap allocate()
    {
        return AssetCompilerWidgetHeap{Pool<AssetCompilationPassWidget>::allocate(0), PoolIndexed<Token<AssetCompilationPass>>::allocate_default()};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(!this->asset_compilation_passes.has_allocated_elements());
        assert_true(!this->asset_compilation_passes_compilationtoken.has_allocated_elements());
#endif
        this->asset_compilation_passes.free();
        this->asset_compilation_passes_compilationtoken.free();
    };

    inline Token<AssetCompilationPassWidget> allocate_widget(AssetCompilationPassWidget& p_widget, const Token<AssetCompilationPass> p_pass)
    {
        Token<AssetCompilationPassWidget> l_token = this->asset_compilation_passes.alloc_element(p_widget);
        this->asset_compilation_passes_compilationtoken.alloc_element(p_pass);
        return l_token;
    };

    inline AssetCompilationPassWidget& get_widget(const Token<AssetCompilationPassWidget> p_widget_token)
    {
        return this->asset_compilation_passes.get(p_widget_token);
    };

    inline Token<AssetCompilationPass> get_asset_compilation_pass_from_widget(const Token<AssetCompilationPassWidget> p_asset_compilation_pass_widget)
    {
        return this->asset_compilation_passes_compilationtoken.get(token_build_from<Token<AssetCompilationPass>>(p_asset_compilation_pass_widget));
    };

    inline AssetCompilationPassWidget& get_widget_from_asset_compilation_pass(const Token<AssetCompilationPass> p_asset_compilation_pass_token)
    {
        return this->asset_compilation_passes.get(token_build_from<AssetCompilationPassWidget>(p_asset_compilation_pass_token));
    };

    inline void free_widget(const Token<AssetCompilationPassWidget> p_widget_token)
    {
        this->asset_compilation_passes_compilationtoken.release_element(token_build_from<Token<AssetCompilationPass>>(p_widget_token));
        this->asset_compilation_passes.get(p_widget_token).free();
        this->asset_compilation_passes.release_element(p_widget_token);
    };
};

/*
    Display asset compilation pass selection and a list of AssetCompilationPassWidget list that matches to the selected one.
*/
struct AssetCompilationPassViewer
{
    QWidget* root;

    struct Widgets
    {
        QHBoxLayout* main_layout;
        QListWidgetItemSelection<Token<AssetCompilationPassWidget>> compilation_pass_list;
        QLayoutWidget<QVBoxLayout, QWidget> display_widget;
    } widgets;

    struct Callbacks
    {
        void* closure;
        void (*on_asset_compiler_pass_selection_change)(void*);
    } callbacks;

    Vector<Token<AssetCompilationPassWidget>> displayed_asset_compilation_pass_widgets;

    inline void allocate(Callbacks& p_callbacks)
    {
        this->root = new QWidget();
        this->widgets.main_layout = new QHBoxLayout(this->root);
        this->root->setLayout(this->widgets.main_layout);

        this->callbacks = p_callbacks;
        this->displayed_asset_compilation_pass_widgets = Vector<Token<AssetCompilationPassWidget>>::allocate(0);

        QListWidgetItemSelection<Token<AssetCompilationPassWidget>>::Callbacks l_compilation_passes_vector_callbacks;
        l_compilation_passes_vector_callbacks.closure = this;
        l_compilation_passes_vector_callbacks.on_selection_changed = [](auto, void* p_closure) {
            AssetCompilationPassViewer* thiz = (AssetCompilationPassViewer*)p_closure;
            thiz->callbacks.on_asset_compiler_pass_selection_change(thiz->callbacks.closure);
        };
        this->widgets.compilation_pass_list.allocate(this->root, l_compilation_passes_vector_callbacks);
        this->widgets.compilation_pass_list.root->setSelectionMode(QAbstractItemView::MultiSelection);
        this->widgets.main_layout->addWidget(this->widgets.compilation_pass_list.root);

        this->widgets.display_widget = QLayoutWidget<QVBoxLayout, QWidget>::allocate();
        this->widgets.main_layout->addWidget(this->widgets.display_widget.widget);
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->displayed_asset_compilation_pass_widgets.empty());
#endif
        this->displayed_asset_compilation_pass_widgets.free();
        this->widgets.compilation_pass_list.free();
    };

    inline void add_asset_compilation_to_selection(const Token<AssetCompilationPassWidget> p_compilation_pass_widget_token, const Slice<int8>& p_item_name)
    {
        QListWidgetItem* l_item = new QListWidgetItem();
        l_item->setText(p_item_name.Begin);
        this->widgets.compilation_pass_list.push_back_element(p_compilation_pass_widget_token, l_item);
    };

    inline void remove_asset_compilation_selection(const Token<AssetCompilationPassWidget> p_compilation_pass_widget_token)
    {
        for (loop_reverse(i, 0, this->widgets.compilation_pass_list.datas.Size))
        {
            if (token_equals(this->widgets.compilation_pass_list.datas.get(i), p_compilation_pass_widget_token))
            {
                this->widgets.compilation_pass_list.erase_element_at_always(i);
            }
        }
    };

    inline void remove_asset_compilation_widget(const Token<AssetCompilationPassWidget> p_compilation_pass_widget_token)
    {
        for (loop_reverse(i, 0, this->displayed_asset_compilation_pass_widgets.Size))
        {
            if (token_equals(this->displayed_asset_compilation_pass_widgets.get(i), p_compilation_pass_widget_token))
            {
                this->displayed_asset_compilation_pass_widgets.erase_element_at_always(i);
            }
        }
    };

    inline void remove_asset_compilation_widget_and_selection(const Token<AssetCompilationPassWidget> p_compilation_pass_widget_token)
    {
        this->remove_asset_compilation_widget(p_compilation_pass_widget_token);
        this->remove_asset_compilation_selection(p_compilation_pass_widget_token);
    };

    inline void remove_and_detach_all_asset_compilation_widgets(AssetCompilerWidgetHeap& p_asset_compiler_widget_heap)
    {
        for (loop_reverse(i, 0, this->displayed_asset_compilation_pass_widgets.Size))
        {
            Token<AssetCompilationPassWidget> l_widget = this->displayed_asset_compilation_pass_widgets.get(i);
            p_asset_compiler_widget_heap.get_widget(l_widget).detach();
            this->displayed_asset_compilation_pass_widgets.erase_element_at_always(i);
        }
        this->displayed_asset_compilation_pass_widgets.clear();
    };

    inline void push_selected_asset_compilations_to_view_widget(AssetCompilerWidgetHeap& p_asset_compiler_widget_heap)
    {
        for (loop(i, 0, this->widgets.compilation_pass_list.selected_items.Size))
        {
            AssetCompilationPassWidget& l_compilation_pass_widget = this->get_asset_compilationpass_widget_from_selecteditem_index(p_asset_compiler_widget_heap, i);
            this->widgets.display_widget.layout->addWidget(l_compilation_pass_widget.root);
            this->displayed_asset_compilation_pass_widgets.push_back_element(this->widgets.compilation_pass_list.get_selected_item(i));
        }
    };

    inline void match_displayed_asset_compilation_with_selected(AssetCompilerWidgetHeap& p_asset_compiler_widget_heap)
    {
        this->remove_and_detach_all_asset_compilation_widgets(p_asset_compiler_widget_heap);
        this->push_selected_asset_compilations_to_view_widget(p_asset_compiler_widget_heap);
    };

    inline AssetCompilationPassWidget& get_asset_compilationpass_widget_from_selecteditem_index(AssetCompilerWidgetHeap& p_asset_compiler_widget_heap, const uimax p_index)
    {
        return p_asset_compiler_widget_heap.get_widget(this->widgets.compilation_pass_list.get_selected_item(p_index));
    };
    inline AssetCompilationPassWidget& get_asset_compilationpass_widget_from_heap_index(AssetCompilerWidgetHeap& p_asset_compiler_widget_heap, const uimax p_index)
    {
        return p_asset_compiler_widget_heap.get_widget(this->widgets.compilation_pass_list.datas.get(p_index));
    };
};

struct AssetCompilerWindow
{
    QWidget* root;

    struct Widgets
    {
        QVBoxLayout* main_layout;
        QLayoutWidget<QHBoxLayout, QWidget> top_bar;
        QPushButton* feed_tree_button;
        QFileDialog* feed_tree_file_dialog;
        QPushButton* go_button;
        AssetCompilationPassViewer asset_compilation_viewer;
    } widgets;

    struct Callbacks
    {
        void* closure;
        void (*load_asset_tree)(AssetCompilerWindow* p_window, Vector<Token<AssetCompilationPass>>* in_out_asset_compilation_pass, const Slice<int8>& p_file_configuration, void* p_closure);
        void (*on_go_button_pushed)(AssetCompilerWindow* p_window, void* p_closure);
    } callbacks;

    Vector<Token<AssetCompilationPass>> callbacks_asset_compilation_pass_buffer;

    AssetCompilerWidgetHeap widget_heap;

    inline void allocate(AssetCompilerPassHeap& p_asset_compiler_heap, const Callbacks& p_callbacks)
    {
        this->callbacks_asset_compilation_pass_buffer = Vector<Token<AssetCompilationPass>>::allocate(0);
        this->allocate_widgets();
        this->setup_widget_layout();
        this->setup_widget_events(p_callbacks, p_asset_compiler_heap);
    };

    inline void free()
    {
        this->widgets.asset_compilation_viewer.free();
        this->widget_heap.free();
        this->callbacks_asset_compilation_pass_buffer.free();
    };

    inline void free_compiler_passes(AssetCompilerPassHeap& p_asset_compiler_heap)
    {
        this->widget_heap.asset_compilation_passes_compilationtoken.foreach_reverse(
            [&](const Token<Token<AssetCompilationPass>> p_token, const Token<AssetCompilationPass> p_asset_compilation_pass_token) {
                p_asset_compiler_heap.free_asset_compiler_pass(p_asset_compilation_pass_token);
                this->free_asset_compilation_widget(token_build_from<AssetCompilationPassWidget>(p_token));
            });
    };

    inline void free_with_asset_compiler_passes(AssetCompilerPassHeap& p_asset_compiler_heap)
    {
        this->free_compiler_passes(p_asset_compiler_heap);
        this->free();
    };

    inline void set_compilation_asset_result(const AssetSingleCompilationToken p_asset_compilation_token, const int8 p_compilation_return)
    {
        // this->widget_heap.compilation_passes.get(p_asset_compilation_token.pass_token)
        this->widget_heap.asset_compilation_passes_compilationtoken.foreach_breakable(
            [&](const Token<Token<AssetCompilationPass>> p_token, const Token<AssetCompilationPass> p_asset_compilation_pass_token) {
                if (token_equals(p_token, p_asset_compilation_token.pass_token))
                {
                    this->widget_heap.get_widget_from_asset_compilation_pass(p_asset_compilation_pass_token).set_asset_compilation_result(p_asset_compilation_token.index, p_compilation_return);
                    return 1;
                }
                return 0;
            });
    };

    inline Token<AssetCompilationPass> free_asset_compilation_widget(const Token<AssetCompilationPassWidget> p_widget_token)
    {
        Token<AssetCompilationPass> l_widget = this->widget_heap.get_asset_compilation_pass_from_widget(p_widget_token);
        this->widgets.asset_compilation_viewer.remove_asset_compilation_widget_and_selection(p_widget_token);
        this->widget_heap.free_widget(p_widget_token);
        return l_widget;
    };

    inline void on_compilation_pass_selection_changed()
    {
        this->widgets.asset_compilation_viewer.match_displayed_asset_compilation_with_selected(this->widget_heap);
    };

  private:
    inline void allocate_widgets()
    {
        this->root = new QWidget();
        this->widgets.main_layout = new QVBoxLayout(this->root);

        this->widgets.top_bar = QLayoutWidget<QHBoxLayout, QWidget>::allocate();

        this->widgets.feed_tree_button = new QPushButton();
        this->widgets.feed_tree_button->setText("FEED");

        this->widgets.go_button = new QPushButton();
        this->widgets.go_button->setText("GO");

        this->widget_heap = AssetCompilerWidgetHeap::allocate();

        AssetCompilationPassViewer::Callbacks l_asset_compilation_pass_viewer_callbacks{};
        l_asset_compilation_pass_viewer_callbacks.closure = this;
        l_asset_compilation_pass_viewer_callbacks.on_asset_compiler_pass_selection_change = [](void* p_closure) {
            AssetCompilerWindow* thiz = (AssetCompilerWindow*)p_closure;
            thiz->on_compilation_pass_selection_changed();
        };
        this->widgets.asset_compilation_viewer.allocate(l_asset_compilation_pass_viewer_callbacks);
        this->widgets.asset_compilation_viewer.root->setParent(this->root);
    };

    inline void setup_widget_layout()
    {
        this->widgets.top_bar.layout->addWidget(this->widgets.feed_tree_button);
        this->widgets.top_bar.layout->addWidget(this->widgets.go_button);

        this->widgets.main_layout->addWidget(this->widgets.top_bar.widget);
        this->widgets.main_layout->addWidget(this->widgets.asset_compilation_viewer.root);
    };

    inline void setup_widget_events(const Callbacks& p_callbacks, AssetCompilerPassHeap& p_asset_compiler_heap)
    {
        this->callbacks = p_callbacks;
        QObject::connect(this->widgets.feed_tree_button, &QPushButton::released, [&]() {
            this->widgets.feed_tree_file_dialog = new QFileDialog();
            this->widgets.feed_tree_file_dialog->setNameFilters({"Asset configuration files (*.json)", "Any files (*)"});
            QObject::connect(this->widgets.feed_tree_file_dialog, &QFileDialog::fileSelected, [&](const QString& p_file) {
                // if (this->on_database_file_selected(p_file))
                QByteArray l_configuration_file_path_arr = p_file.toLocal8Bit();
                Slice<int8> l_configuration_file_path = slice_int8_build_rawstr(l_configuration_file_path_arr.data());
                this->callbacks.load_asset_tree(this, &this->callbacks_asset_compilation_pass_buffer, l_configuration_file_path, this->callbacks.closure);
                for (loop(i, 0, this->callbacks_asset_compilation_pass_buffer.Size))
                {
                    Token<AssetCompilationPass> l_pass = this->callbacks_asset_compilation_pass_buffer.get(i);
                    AssetCompilationPassWidget l_compilation_pass_widget = AssetCompilationPassWidget::allocate_empty();
                    // this->widgets.main_layout->addWidget(l_compilation_pass_widget.root);
                    AssetCompilationPass& l_asset_compilation_pass = p_asset_compiler_heap.asset_compilation_passes.get(l_pass);
                    l_compilation_pass_widget.feed_with_compilation_pass(l_asset_compilation_pass);
                    Token<AssetCompilationPassWidget> l_widget_token = this->widget_heap.allocate_widget(l_compilation_pass_widget, l_pass);
                    this->widgets.asset_compilation_viewer.add_asset_compilation_to_selection(l_widget_token, l_asset_compilation_pass.root_path.to_slice());
                }
                this->callbacks_asset_compilation_pass_buffer.clear();
            });
            this->widgets.feed_tree_file_dialog->open();
        });
        QObject::connect(this->widgets.go_button, &QPushButton::released, [&]() {
            this->callbacks.on_go_button_pushed(this, this->callbacks.closure);
        });
    };
};

struct AssetCompilerComposition
{

    inline static void push_selected_assetcompilation_pass_to_thread(AssetCompilerWindow& p_window, AssetCompilerPassHeap& p_heap, AssetCompilationThread& p_thread)
    {
        for (loop(i, 0, p_window.widgets.asset_compilation_viewer.displayed_asset_compilation_pass_widgets.Size))
        {
            Token<AssetCompilationPassWidget> l_widget_token = p_window.widgets.asset_compilation_viewer.displayed_asset_compilation_pass_widgets.get(i);
            Token<AssetCompilationPass> l_asset_compilation_pass = p_window.widget_heap.get_asset_compilation_pass_from_widget(l_widget_token);
            p_thread.push_asset_compilation_pass(l_asset_compilation_pass, p_heap.asset_compilation_passes.get(l_asset_compilation_pass));
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
    AssetCompilerPassHeap heap;
    AssetCompilerWindow window;
    AssetCompilationThread compilation_thread;
    QTimer* compilation_thread_output_consumer;

    inline QWidget* root()
    {
        return this->window.root;
    };

    inline void allocate()
    {
        this->heap = AssetCompilerPassHeap::allocate();
        AssetCompilerWindow::Callbacks l_asset_compiler_window_cb{};
        l_asset_compiler_window_cb.closure = this;
        l_asset_compiler_window_cb.load_asset_tree = [](AssetCompilerWindow* p_window, Vector<Token<AssetCompilationPass>>* in_out_asset_compilation_pass,
                                                        const Slice<int8>& p_compile_configuration_file, void* p_closure) {
            AssetCompilerEditor* thiz = (AssetCompilerEditor*)p_closure;
            thiz->compilation_thread.sync_wait_for_processing_compilation_events();
            thiz->window.free_compiler_passes(thiz->heap);
            AssetCompilerPassComposition::allocate_passes_from_json_configuration_with_assetroot_as_configuration_file_path(thiz->heap, p_compile_configuration_file, in_out_asset_compilation_pass);
        };
        l_asset_compiler_window_cb.on_go_button_pushed = [](AssetCompilerWindow* p_window, void* p_closure) {
            AssetCompilerEditor* thiz = (AssetCompilerEditor*)p_closure;
            AssetCompilerComposition::push_selected_assetcompilation_pass_to_thread(thiz->window, thiz->heap, thiz->compilation_thread);
        };
        this->window.allocate(this->heap, l_asset_compiler_window_cb);

        this->compilation_thread = AssetCompilationThread::allocate();

        this->compilation_thread_output_consumer = new QTimer();
        this->compilation_thread_output_consumer->setInterval(0);
        QObject::connect(this->compilation_thread_output_consumer, &QTimer::timeout, [&]() {
            AssetCompilerComposition::try_consume_output_thread_events(this->compilation_thread, this->window);
        });
        this->compilation_thread_output_consumer->start();
    };

    inline void free()
    {
        this->compilation_thread.stop_if_running();
        AssetCompilerComposition::try_consume_output_thread_events(this->compilation_thread, this->window);
        this->compilation_thread.free();
        this->window.free_with_asset_compiler_passes(this->heap);
        this->heap.free();
    };
};