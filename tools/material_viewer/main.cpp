
#include "./material_viewer_window.hpp"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    MaterialViewerEditor l_material_viewer_editor{};
    l_material_viewer_editor.allocate();

    QMainWindow* l_main_window = new QMainWindow(NULL);

    l_main_window->setCentralWidget(l_material_viewer_editor.root());
    l_main_window->show();

    int l_exec = a.exec();

    l_material_viewer_editor.free();

    // TODO -> move this to test
    memleak_ckeck();

    return l_exec;
};
