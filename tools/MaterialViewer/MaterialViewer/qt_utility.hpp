#pragma once

inline int32 qt_app_start(QApplication& qt_app, QWidget* p_widget, QMainWindow** out_main_window)
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
