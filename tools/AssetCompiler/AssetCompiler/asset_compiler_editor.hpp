#pragma once

#include "QTCommon/qt_common.hpp"
#include "AssetCompiler/asset_compiler.hpp"

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

    inline void stop_and_free()
    {
        this->stop_and_wait();
        this->free();
    };

    inline void free()
    {
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
        // TODO -> implement this
    };

    inline void unpause(){
        // TODO -> implement this
    };

    inline void stop_and_wait()
    {
        this->ask_exit = 1;
        Thread::wait_for_end_and_terminate(this->thread, -1);
        this->is_running = 0;
    };

    inline int8 has_compilation_events()
    {
        return !this->input_events._data.compilation_passes.empty();
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
};

template <class ElementType> struct QListWidgetItemSelection
{
    QListWidget* root;
    Vector<ElementType> datas;
    Vector<QListWidgetItem*> item_widgets;

    Vector<Token(ElementType)> selected_items;

    struct Callbacks
    {
        void* closure;
        void (*on_selection_changed)(QListWidgetItemSelection<ElementType>* thiz, void* p_closure);
    } callbacks;

    inline void allocate(QWidget* p_parent, const Callbacks& p_callbacks)
    {
        this->root = new QListWidget(p_parent);
        this->datas = Vector<ElementType>::allocate(0);
        this->selected_items = Vector<Token(ElementType)>::allocate(0);
        this->item_widgets = Vector<QListWidgetItem*>::allocate(0);

        this->callbacks = p_callbacks;

        QObject::connect(this->root, &QListWidget::itemSelectionChanged, [&]() {
            QList<QListWidgetItem*> l_selected_items = this->root->selectedItems();
            this->selected_items.clear();
            QList<QListWidgetItem*>::iterator l_selected_items_it;
            for (l_selected_items_it = l_selected_items.begin(); l_selected_items_it != l_selected_items.end(); ++l_selected_items_it)
            {
                QListWidgetItem* l_item = *(l_selected_items_it.operator QListWidgetItem**());
                this->selected_items.push_back_element(tk_b(ElementType, this->root->row(l_item)));
            }
            this->on_selection_changed();
        });
    };

    inline void free()
    {
        this->datas.free();
        this->selected_items.free();
        this->item_widgets.free();
    };

    inline void on_selection_changed()
    {
        if (this->callbacks.on_selection_changed)
        {
            this->callbacks.on_selection_changed(this, this->callbacks.closure);
        }
    };

    inline void push_back_element(const ElementType& p_data, QListWidgetItem* p_widget)
    {
        this->root->addItem(p_widget);
        this->item_widgets.push_back_element(p_widget);
        this->datas.push_back_element(p_data);
    };

    inline ElementType& get_selected_item(const uimax p_index)
    {
        return this->datas.get(tk_v(this->selected_items.get(p_index)));
    };

    inline void erase_element_at_always(const uimax p_index)
    {
        this->datas.erase_element_at_always(p_index);
        // this->root->removeItemWidget
        this->root->removeItemWidget(this->item_widgets.get(p_index));
        delete this->item_widgets.get(p_index);
        this->item_widgets.erase_element_at_always(p_index);
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
        this->root->setParent(NULL);
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
            this->items.push_back_element(Item{l_item});
        }
    };

    inline void set_asset_compilation_result(const uimax p_asset_index, const int8 p_result)
    {
        Item& l_item = this->items.get(p_asset_index);
        l_item.set_compilation_result(p_result);
    };
};

struct AssetCompilationPassViewer
{
    QWidget* root;

    struct Widgets
    {
        QHBoxLayout* main_layout;
        QListWidgetItemSelection<Token(AssetCompilationPassWidget)> compilation_pass_list;
        QLayoutWidget<QVBoxLayout, QWidget> display_widget;
    } widgets;

    Vector<Token(AssetCompilationPassWidget)> displayed_compilation_passes;

    Vector<AssetCompilationPassWidget>* asset_compilation_pass_widget_heap;

    inline void allocate(Vector<AssetCompilationPassWidget>& p_asset_compilation_pass_widget_heap)
    {
        this->root = new QWidget();
        this->widgets.main_layout = new QHBoxLayout(this->root);
        this->root->setLayout(this->widgets.main_layout);

        this->asset_compilation_pass_widget_heap = &p_asset_compilation_pass_widget_heap;
        this->displayed_compilation_passes = Vector<Token(AssetCompilationPassWidget)>::allocate(0);

        QListWidgetItemSelection<Token(AssetCompilationPassWidget)>::Callbacks l_compilation_passes_vector_callbacks;
        l_compilation_passes_vector_callbacks.closure = this;
        l_compilation_passes_vector_callbacks.on_selection_changed = [](QListWidgetItemSelection<Token(AssetCompilationPassWidget)>* p_widget, void* p_closure) {
            AssetCompilationPassViewer* thiz = (AssetCompilationPassViewer*)p_closure;
            thiz->match_displayed_asset_compilation_with_selected();
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
        assert_true(this->displayed_compilation_passes.empty());
#endif
        this->displayed_compilation_passes.free();
        this->widgets.compilation_pass_list.free();
    };

    inline void add_item(const Token(AssetCompilationPassWidget) p_compilation_pass_widget_token, AssetCompilationPassWidget& p_compilation_pass_widget, const Slice<int8>& p_item_name)
    {
        QListWidgetItem* l_item = new QListWidgetItem();
        l_item->setText(p_item_name.Begin);
        this->widgets.compilation_pass_list.push_back_element(p_compilation_pass_widget_token, l_item);
    };

    inline void match_displayed_asset_compilation_with_selected()
    {

        for (loop(i, 0, this->displayed_compilation_passes.Size))
        {
            this->asset_compilation_pass_widget_heap->get(tk_v(this->displayed_compilation_passes.get(i))).root->setParent(NULL);
        }
        this->displayed_compilation_passes.clear();

        for (loop(i, 0, this->widgets.compilation_pass_list.selected_items.Size))
        {
            AssetCompilationPassWidget& l_compilation_pass_widget = this->asset_compilation_pass_widget_heap->get(tk_v(this->widgets.compilation_pass_list.get_selected_item(i)));
            this->widgets.display_widget.layout->addWidget(l_compilation_pass_widget.root);
            this->displayed_compilation_passes.push_back_element(this->widgets.compilation_pass_list.get_selected_item(i));
        }
    };

    inline void remove_AssetCompilationPassWidget_references(const Token(AssetCompilationPassWidget) p_compilation_pass_widget_token)
    {

        for (loop_reverse(i, 0, this->displayed_compilation_passes.Size))
        {
            if (tk_eq(this->displayed_compilation_passes.get(i), p_compilation_pass_widget_token))
            {
                this->displayed_compilation_passes.erase_element_at_always(i);
            }
        }

        for (loop_reverse(i, 0, this->widgets.compilation_pass_list.datas.Size))
        {
            if (tk_eq(this->widgets.compilation_pass_list.datas.get(i), p_compilation_pass_widget_token))
            {
                this->widgets.compilation_pass_list.erase_element_at_always(i);
            }
        }
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
        AssetCompilationPassViewer asset_compilation_viewer;
    } widgets;

    struct Callbacks
    {
        void* closure;
        void (*load_asset_tree)(AssetCompilerWindow* p_window, Vector<Token(AssetCompilationPass)>* in_out_asset_compilation_pass, void* p_closure);
        void (*on_go_button_pushed)(AssetCompilerWindow* p_window, void* p_closure);
    } callbacks;

    Vector<Token(AssetCompilationPass)> callbacks_asset_compilation_pass_buffer;

    struct WidgetHeap
    {
        Vector<AssetCompilationPassWidget> asset_compilation_passes;
    } widget_heap;

    struct View
    {
        // TODO -> move this to the heap as the index is the same as the AssetCompilationPassWidget
        Vector<Token(AssetCompilationPass)> compilation_passes;
    } view;

    inline void allocate(AssetCompilerPassHeap& p_asset_compiler_heap, const Callbacks& p_callbacks)
    {
        this->callbacks_asset_compilation_pass_buffer = Vector<Token(AssetCompilationPass)>::allocate(0);
        this->view.compilation_passes = Vector<Token(AssetCompilationPass)>::allocate(0);
        this->allocate_widgets();
        this->setup_widget_layout();
        this->setup_widget_events(p_callbacks, p_asset_compiler_heap);
    };

    inline void free(AssetCompilerPassHeap& p_asset_compiler_heap)
    {
        for (loop_reverse(i, 0, this->view.compilation_passes.Size))
        {
            p_asset_compiler_heap.free_asset_compiler_pass(this->view.compilation_passes.get(i));
            this->remove_asset_compilation(i);
        }

        this->widget_heap.asset_compilation_passes.free();
        this->widgets.asset_compilation_viewer.free();
        this->view.compilation_passes.free();
        this->callbacks_asset_compilation_pass_buffer.free();
    };

    inline void set_compilation_asset_result(const AssetSingleCompilationToken p_asset_compilation_token, const int8 p_result)
    {
        for (loop(i, 0, this->view.compilation_passes.Size))
        {
            if (tk_eq(this->view.compilation_passes.get(i), p_asset_compilation_token.pass_token))
            {
                this->widget_heap.asset_compilation_passes.get(i).set_asset_compilation_result(p_asset_compilation_token.index, p_result);
                break;
            }
        }
    };

    inline Token(AssetCompilationPass) remove_asset_compilation(const uimax p_index)
    {
        Token(AssetCompilationPass) l_return = this->view.compilation_passes.get(p_index);
        this->view.compilation_passes.erase_element_at_always(p_index);

        this->widgets.asset_compilation_viewer.remove_AssetCompilationPassWidget_references(tk_b(AssetCompilationPassWidget, p_index));
        this->widget_heap.asset_compilation_passes.get(p_index).free();
        this->widget_heap.asset_compilation_passes.erase_element_at_always(p_index);
        return l_return;
    };

    inline Span<Token<AssetCompilationPass>> remove_all_asset_compilation_pass_widgets()
    {
        Span<Token<AssetCompilationPass>> l_removed_asset_compilation_passes = Span<Token<AssetCompilationPass>>::allocate(this->widget_heap.asset_compilation_passes.Size);
        for (loop_reverse(i, 0, this->widget_heap.asset_compilation_passes.Size))
        {
            l_removed_asset_compilation_passes.get(i) = this->remove_asset_compilation(i);
        }
        this->widget_heap.asset_compilation_passes.clear();
        return l_removed_asset_compilation_passes;
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

        this->widget_heap.asset_compilation_passes = Vector<AssetCompilationPassWidget>::allocate(0);

        this->widgets.asset_compilation_viewer.allocate(this->widget_heap.asset_compilation_passes);
        this->widgets.asset_compilation_viewer.root->setParent(this->root);
    };

    inline void setup_widget_layout()
    {
        QLayoutBuilder l_layout_builder;
        l_layout_builder.bind_layout(this->widgets.main_layout);
        l_layout_builder.add_widget(this->widgets.feed_tree_button);
        l_layout_builder.add_widget(this->widgets.go_button);
        l_layout_builder.add_widget(this->widgets.asset_compilation_viewer.root);
    };

    inline void setup_widget_events(const Callbacks& p_callbacks, AssetCompilerPassHeap& p_asset_compiler_heap)
    {
        this->callbacks = p_callbacks;
        QObject::connect(this->widgets.feed_tree_button, &QPushButton::released, [&]() {
            this->callbacks.load_asset_tree(this, &this->callbacks_asset_compilation_pass_buffer, this->callbacks.closure);
            for (loop(i, 0, this->callbacks_asset_compilation_pass_buffer.Size))
            {
                Token(AssetCompilationPass) l_pass = this->callbacks_asset_compilation_pass_buffer.get(i);
                AssetCompilationPassWidget l_compilation_pass_widget = AssetCompilationPassWidget::allocate_empty();
                // this->widgets.main_layout->addWidget(l_compilation_pass_widget.root);
                AssetCompilationPass& l_asset_compilation_pass = p_asset_compiler_heap.asset_compilation_passes.get(l_pass);
                l_compilation_pass_widget.feed_with_compilation_pass(l_asset_compilation_pass);
                this->widget_heap.asset_compilation_passes.push_back_element(l_compilation_pass_widget);
                this->view.compilation_passes.push_back_element(l_pass);
                this->widgets.asset_compilation_viewer.add_item(tk_b(AssetCompilationPassWidget, this->widget_heap.asset_compilation_passes.Size - 1), l_compilation_pass_widget,
                                                                l_asset_compilation_pass.root_path.to_slice());
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

    inline static void push_selected_assetcompilation_pass_to_thread(AssetCompilerWindow& p_window, AssetCompilerPassHeap& p_heap, AssetCompilationThread& p_thread)
    {
        for (loop(i, 0, p_window.widgets.asset_compilation_viewer.displayed_compilation_passes.Size))
        {
            Token(AssetCompilationPass) l_asset_compilation_pass = p_window.view.compilation_passes.get(tk_v(p_window.widgets.asset_compilation_viewer.displayed_compilation_passes.get(i)));
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
    Slice<int8> asset_folder_root;

    inline QWidget* root()
    {
        return this->window.root;
    };

    inline void allocate(const Slice<int8>& p_asset_folder_root)
    {
        this->heap = AssetCompilerPassHeap::allocate();
        this->asset_folder_root = p_asset_folder_root;
        AssetCompilerWindow::Callbacks l_asset_compiler_window_cb{};
        l_asset_compiler_window_cb.closure = this;
        l_asset_compiler_window_cb.load_asset_tree = [](AssetCompilerWindow* p_window, Vector<Token(AssetCompilationPass)>* in_out_asset_compilation_pass, void* p_closure) {
            AssetCompilerEditor* thiz = (AssetCompilerEditor*)p_closure;
            while (thiz->compilation_thread.has_compilation_events())
            {
            };
            Span<Token<AssetCompilationPass>> l_removed_compilation_pass = thiz->window.remove_all_asset_compilation_pass_widgets();
            for (loop(i, 0, l_removed_compilation_pass.Capacity))
            {
                thiz->heap.free_asset_compiler_pass(l_removed_compilation_pass.get(i));
            }
            l_removed_compilation_pass.free();
            // TODO -> remove absolute
            AssetCompilerPassComposition::allocate_passes_from_json_configuration(thiz->heap, slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/_asset/asset/compile_conf.json"),
                                                                                  thiz->asset_folder_root, in_out_asset_compilation_pass);
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
        this->compilation_thread.stop_and_wait();
        AssetCompilerComposition::try_consume_output_thread_events(this->compilation_thread, this->window);
        this->compilation_thread.free();
        this->window.free(this->heap);
        this->heap.free();
    };
};