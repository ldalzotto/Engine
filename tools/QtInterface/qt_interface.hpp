
#define __NATIVE_WINDOW_ENABLED 0
#include "Common2/common2.hpp"

using uitag_t = uimax;

struct GUIEvent
{
    enum class Type
    {
        UNDEFINED = 0,
        JUST_PRESSED = 1,
        FILE_SELECTED = 2,
        LIST_SELECTION_CHANGED = 3,
        JUST_EXIT = 4,
    } type;

    enum class SourceElementType
    {
        UNDEFINED = 0,
        BUTTON = 1,
        FILE_DIALOG = 2,
        LIST = 3,
        APPLICATION = 4
    } element_type;

    Token<int8> token;
};

struct GUIEventHandler
{
    void (*callback)(const GUIEvent& p_event, int8* p_args, int8* p_closure);
    int8* closure;

    inline void exec(const GUIEvent& p_event, int8* p_args) const
    {
        this->callback(p_event, p_args, this->closure);
    };
};

struct _GUIApplication
{
    static _GUIApplication* allocate(int argc, char* argv[]);
    static void free(_GUIApplication* thiz);
    int8 exec();

    void register_exit(GUIEventHandler& p_handler);
};

struct _GUIWidget
{
    static _GUIWidget* load_design_sheet(const Slice<int8>& p_path);
    int8* find_child(const Slice<int8>& p_name);
    void set_parent(_GUIWidget* p_parent);
};

struct _GUIListWidget
{
    void add_item(const Slice<int8>& p_name, const Token<int8> p_external_token);
    void clear();
    uimax get_size();
    Token<int8> get_value(const uimax p_index);
};

struct _GUIMainWindow
{
    static _GUIMainWindow* allocate();
    void show();
    void close();
};

struct _GUILabel
{
    void set_text(const Slice<int8>& p_text);
};

struct _GUIPushButton
{
};

struct _GUIFileDialog
{
    static _GUIFileDialog* allocate(_GUIWidget* p_parent);
    void open();
    void close();
};



struct GUIButton
{
    uitag_t tag;
    _GUIPushButton* button;
    static Token<GUIButton> register_(_GUIPushButton* p_button, const uitag_t p_tag, Pool<GUIButton>& p_my_button_pool, GUIEventHandler& p_handler);
    template <class _InternalWidget> _InternalWidget* find_child(const Slice<int8>& p_name);
};

struct GUIFileDialog
{
    uitag_t tag;
    _GUIFileDialog* file_dialog;

    static Token<GUIFileDialog> register_(_GUIFileDialog* p_file_dialog, const uitag_t p_tag, Pool<GUIFileDialog>& p_my_file_dialog_pool, GUIEventHandler& p_handler);
};

struct GUIListSelectionChangeInput {
    Token<int8> previous_element;
    Token<int8> current_element;
};

struct GUIList
{
    uitag_t tag;
    _GUIListWidget* list;

    static Token<GUIList> register_(_GUIListWidget* p_button, const uitag_t p_tag, Pool<GUIList>& p_my_button_pool, GUIEventHandler& p_handler);

};

