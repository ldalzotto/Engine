#pragma once

inline int qt_app_start(QApplication& qt_app, QWidget* p_widget, QMainWindow** out_main_window)
{
    *out_main_window = new QMainWindow(NULL);
    (*out_main_window)->setCentralWidget(p_widget);
    (*out_main_window)->show();
    return qt_app.exec();
};

template <class Layout_t, class Widget_t> struct QLayoutWidget
{
    Layout_t* layout;
    Widget_t* widget;

    inline static QLayoutWidget<Layout_t, Widget_t> allocate()
    {
        QLayoutWidget<Layout_t, Widget_t> l_return;
        l_return.widget = new Widget_t();
        l_return.layout = new Layout_t(l_return.widget);
        l_return.widget->setLayout(l_return.layout);
        return l_return;
    };
};

struct QLayoutBuilder
{
    QLayout* binded_layout;

    template <class Layout_t, class Widget_t> inline void bind_layout(const QLayoutWidget<Layout_t, Widget_t>& p_layout_widget)
    {
        this->binded_layout = p_layout_widget.layout;
    };

    inline void bind_layout(QLayout* p_layout)
    {
        this->binded_layout = p_layout;
    };

    inline void add_widget(QWidget* p_widget)
    {
        this->binded_layout->addWidget(p_widget);
    };

    inline void add_widget_2(QWidget* p_widget_1, QWidget* p_widget_2)
    {
        this->add_widget(p_widget_1);
        this->add_widget(p_widget_2);
    };

    inline void add_widget_3(QWidget* p_widget_1, QWidget* p_widget_2, QWidget* p_widget_3)
    {
        this->add_widget_2(p_widget_1, p_widget_2);
        this->add_widget(p_widget_3);
    };
};


inline static void QFileDialog_simulate_pick(QFileDialog* p_file_dialog, const Slice<int8>& p_path)
{
    p_file_dialog->fileSelected(QString::fromLocal8Bit(p_path.Begin, p_path.Size));
};

/*
    Associated ElementType data to a QListWidgetItem widget.
    Indices of data and widgets are ensured to be the same identical.
*/
template <class ElementType> struct QListWidgetItemSelection
{
    QListWidget* root;
    Vector<ElementType> datas;
    Vector<QListWidgetItem*> item_widgets;

    Vector<Token<ElementType>> selected_items;

    struct Callbacks
    {
        void* closure;
        void (*on_selection_changed)(QListWidgetItemSelection<ElementType>* thiz, void* p_closure);
    } callbacks;

    inline void allocate(QWidget* p_parent, const Callbacks& p_callbacks)
    {
        this->root = new QListWidget(p_parent);
        this->datas = Vector<ElementType>::allocate(0);
        this->selected_items = Vector<Token<ElementType>>::allocate(0);
        this->item_widgets = Vector<QListWidgetItem*>::allocate(0);

        this->callbacks = p_callbacks;

        QObject::connect(this->root, &QListWidget::itemSelectionChanged, [&]() {
          QList<QListWidgetItem*> l_selected_items = this->root->selectedItems();
          this->selected_items.clear();
          QList<QListWidgetItem*>::iterator l_selected_items_it;
          for (l_selected_items_it = l_selected_items.begin(); l_selected_items_it != l_selected_items.end(); ++l_selected_items_it)
          {
              QListWidgetItem* l_item = *(l_selected_items_it.operator QListWidgetItem**());
              this->selected_items.push_back_element(token_build<ElementType>(this->root->row(l_item)));
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

    inline uimax get_size()
    {
        return this->datas.Size;
    };

    inline void push_back_element(const ElementType& p_data, QListWidgetItem* p_widget)
    {
        this->root->addItem(p_widget);
        this->item_widgets.push_back_element(p_widget);
        this->datas.push_back_element(p_data);
    };

    inline ElementType& get_selected_item(const uimax p_index)
    {
        return this->datas.get(token_value(this->selected_items.get(p_index)));
    };

    inline QListWidgetItem* get_widget_item(const uimax p_index)
    {
        return this->item_widgets.get(p_index);
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