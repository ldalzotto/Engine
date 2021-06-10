#define __lib 1

#define QT_NO_KEYWORDS

#include "QtUiTools/quiloader.h"
#include "QtWidgets/qapplication.h"
#include "QtWidgets/qmainwindow.h"
#include "QtWidgets/qpushbutton.h"
#include "QtWidgets/qfiledialog.h"
#include "QtWidgets/qlineedit.h"
#include "QtWidgets/qboxlayout.h"
#include "QtWidgets/qlistwidget.h"
#include "QtWidgets/qlabel.h"
#include "QtCore/qtimer.h"

#include "./qt_interface.hpp"

inline QString slice_to_qstring(const Slice<int8>& p_slice)
{
    return QString::fromLocal8Bit(p_slice.Begin, p_slice.Size);
};

_GUIApplication* _GUIApplication::allocate(int argc, char* argv[])
{
    _GUIApplication* l_memory = (_GUIApplication*)new QApplication(argc, argv);
#if __MEMLEAK
    push_ptr_to_tracked((int8*)l_memory);
#endif
    return l_memory;
};

void _GUIApplication::free(_GUIApplication* thiz)
{
    heap_free((int8*)thiz);
};

int8 _GUIApplication::exec()
{
    return ((QApplication*)this)->exec();
};

void _GUIApplication::register_exit(GUIEventHandler& p_handler)
{
    QObject::connect((QApplication*)this, &QCoreApplication::aboutToQuit, [p_handler]() {
        GUIEvent l_event;
        l_event.type = GUIEvent::Type::JUST_EXIT;
        l_event.element_type = GUIEvent::SourceElementType::APPLICATION;
        p_handler.exec(l_event, 0);
    });
};

_GUIWidget* _GUIWidget::load_design_sheet(const Slice<int8>& p_path)
{
    QUiLoader l_loader;
    QFile l_file(slice_to_qstring(p_path));
    l_file.open(QFile::ReadOnly);
    return (_GUIWidget*)l_loader.load(&l_file);
};

int8* _GUIWidget::find_child(const Slice<int8>& p_name)
{
    return (int8*)(((QWidget*)this)->findChild<QWidget*>(slice_to_qstring(p_name)));
};

void _GUIWidget::set_parent(_GUIWidget* p_parent)
{
    ((QWidget*)this)->setParent((QWidget*)p_parent);
};

void _GUIListWidget::add_item(const Slice<int8>& p_name, const Token<int8> p_external_token)
{
    QListWidgetItem* l_item = new QListWidgetItem(slice_to_qstring(p_name));
    l_item->setData(Qt::UserRole, QVariant(token_value(p_external_token)));
    ((QListWidget*)this)->addItem(l_item);
};

void _GUIListWidget::clear()
{
    ((QListWidget*)this)->clear();
};

uimax _GUIListWidget::get_size()
{
    return ((QListWidget*)this)->count();
};

Token<int8> _GUIListWidget::get_value(const uimax p_index)
{
    return *(Token<int8>*)(((QListWidget*)this)->item(p_index)->data(Qt::UserRole).data());
};

_GUIMainWindow* _GUIMainWindow::allocate()
{
    return (_GUIMainWindow*)new QMainWindow(0);
};

void _GUIMainWindow::show()
{
    ((QMainWindow*)this)->show();
};

void _GUIMainWindow::close()
{
    ((QMainWindow*)this)->close();
};

void _GUILabel::set_text(const Slice<int8>& p_text)
{
    ((QLabel*)this)->setText(slice_to_qstring(p_text));
};

_GUIFileDialog* _GUIFileDialog::allocate(_GUIWidget* p_parent)
{
    return (_GUIFileDialog*)new QFileDialog((QWidget*)p_parent);
};

void _GUIFileDialog::open()
{
    ((QFileDialog*)this)->open();
};
void _GUIFileDialog::close()
{
    ((QFileDialog*)this)->close();
};

template <class _Widget> struct iWidget
{
    _Widget& widget;

    template <class _Value> inline void set_property(const Slice<int8>& p_name, const _Value& p_value)
    {
        ((QWidget*)&this->widget)->setProperty(slice_to_qstring(p_name).toLatin1().data(), QVariant(p_value));
    };

    template <class _Value> inline _Value* get_property(const Slice<int8>& p_name)
    {
        return (_Value*)((QPushButton*)&this->widget)->property(slice_to_qstring(p_name).toLatin1().data()).data();
    };

    template <class _Value> inline _Value* get_property(const Slice<int8>& p_name) const
    {
        return ((iWidget<_Widget>*)&this->widget)->get_property<_Value>(p_name);
    }
};

template <class _Func> inline void _connect_button_released(_GUIPushButton* p_widget, const _Func& p_callback)
{
    QObject::connect((QPushButton*)p_widget, &QPushButton::released, p_callback);
};

template <class _Func> inline void _connect_filedialog_fileselected(_GUIFileDialog* p_widget, const _Func& p_callback)
{
    QObject::connect((QFileDialog*)p_widget, &QFileDialog::fileSelected, p_callback);
};

Token<GUIButton> GUIButton::register_(_GUIPushButton* p_button, const uitag_t p_tag, Pool<GUIButton>& p_my_button_pool, GUIEventHandler& p_handler)
{
    Token<GUIButton> l_button_token = p_my_button_pool.alloc_element(GUIButton{p_tag, p_button});

    {
        iWidget<_GUIPushButton> l_button = iWidget<_GUIPushButton>{*p_button};
        l_button.set_property(slice_int8_build_rawstr("user"), token_value(l_button_token));
    }

    _connect_button_released(p_button, [p_button, p_handler]() {
        iWidget<_GUIPushButton> l_button = iWidget<_GUIPushButton>{*p_button};
        Token<GUIButton>* l_token = l_button.get_property<Token<GUIButton>>(slice_int8_build_rawstr("user"));
        GUIEvent l_event;
        l_event.type = GUIEvent::Type::JUST_PRESSED;
        l_event.element_type = GUIEvent::SourceElementType::BUTTON;
        l_event.token = *(Token<int8>*)l_token;

        p_handler.exec(l_event, 0);
    });
    return l_button_token;
};

Token<GUIFileDialog> GUIFileDialog::register_(_GUIFileDialog* p_file_dialog, const uitag_t p_tag, Pool<GUIFileDialog>& p_my_file_dialog_pool, GUIEventHandler& p_handler)
{
    Token<GUIFileDialog> l_button_token = p_my_file_dialog_pool.alloc_element(GUIFileDialog{p_tag, p_file_dialog});

    {
        iWidget<_GUIFileDialog> l_file_dialog = iWidget<_GUIFileDialog>{*p_file_dialog};
        l_file_dialog.set_property(slice_int8_build_rawstr("user"), token_value(l_button_token));
    }

    _connect_filedialog_fileselected(p_file_dialog, [p_file_dialog, p_handler](const QString& p_file) {
        iWidget<_GUIFileDialog> l_file_dialog = iWidget<_GUIFileDialog>{*p_file_dialog};
        Token<GUIFileDialog>* l_token = l_file_dialog.get_property<Token<GUIFileDialog>>(slice_int8_build_rawstr("user"));
        GUIEvent l_event;
        l_event.type = GUIEvent::Type::FILE_SELECTED;
        l_event.element_type = GUIEvent::SourceElementType::FILE_DIALOG;
        l_event.token = *(Token<int8>*)l_token;
        QByteArray l_database_file_path_arr = p_file.toLocal8Bit();
        Slice<int8> l_file_path = slice_int8_build_rawstr(l_database_file_path_arr.data());
        p_handler.exec(l_event, (int8*)&l_file_path);
    });
    return l_button_token;
}

template <class _Func> inline void _connect_list_selectionchanged(_GUIListWidget* p_widget, const _Func& p_callback)
{
    QObject::connect((QListWidget*)p_widget, &QListWidget::currentItemChanged, p_callback);
};

Token<GUIList> GUIList::register_(_GUIListWidget* p_button, const uitag_t p_tag, Pool<GUIList>& p_my_button_pool, GUIEventHandler& p_handler)
{
    Token<GUIList> l_token = p_my_button_pool.alloc_element(GUIList{p_tag, p_button});
    {
        iWidget<_GUIListWidget> l_file_dialog = iWidget<_GUIListWidget>{*p_button};
        l_file_dialog.set_property(slice_int8_build_rawstr("user"), token_value(l_token));
    }

    _connect_list_selectionchanged(p_button, [p_button, p_handler](QListWidgetItem* current, QListWidgetItem* previous) {
        iWidget<_GUIListWidget> l_file_dialog = iWidget<_GUIListWidget>{*p_button};
        Token<GUIFileDialog>* l_token = l_file_dialog.get_property<Token<GUIFileDialog>>(slice_int8_build_rawstr("user"));

        GUIEvent l_event;
        l_event.type = GUIEvent::Type::LIST_SELECTION_CHANGED;
        l_event.element_type = GUIEvent::SourceElementType::LIST;
        l_event.token = *(Token<int8>*)l_token;

        GUIListSelectionChangeInput l_input;
        if (previous)
        {
            l_input.previous_element = *(Token<int8>*)previous->data(Qt::UserRole).data();
        }
        else
        {
            l_input.previous_element = token_build_default<int8>();
        }

        l_input.current_element = *(Token<int8>*)current->data(Qt::UserRole).data();

        p_handler.exec(l_event, (int8*)&l_input);
    });

    return l_token;
};