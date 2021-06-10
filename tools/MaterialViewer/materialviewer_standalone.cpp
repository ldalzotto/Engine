
#include "MaterialViewer/material_viewer_editor_v2.hpp"

int main(int argc, char* argv[])
{
    _GUIApplication* l_gui_app = _GUIApplication::allocate(argc, argv);
    _GUIMainWindow* l_main_window = _GUIMainWindow::allocate();

    App l_app = App::allocate();
    l_app.setup();

    l_app.main_elements.root->set_parent((_GUIWidget*)l_main_window);
    // ((_GUIWidget*)l_main_window)->set_parent(l_app.main_elements.root);
    l_gui_app->register_exit(l_app.event_handler);

    l_main_window->show();
    int l_return = l_gui_app->exec();
    l_app.free();

    _GUIApplication::free(l_gui_app);
    memleak_ckeck();

    return l_return;
};