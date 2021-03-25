
#include "MaterialViewer/material_viewer_editor.hpp"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    QMainWindow* l_main_window;

    MaterialViewerEditor l_material_viewer_editor{};
    l_material_viewer_editor.allocate();

    int l_exec = qt_app_start(a, l_material_viewer_editor.root(), &l_main_window);
    l_material_viewer_editor.free();
    return l_exec;
};
