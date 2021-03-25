
#include "AssetCompiler/asset_compiler_editor.hpp"

int main(int argc, char** argv)
{
    QApplication a(argc, argv);
    QMainWindow* l_main_window;

    AssetCompilerEditor l_asset_compiler_editor{};
    l_asset_compiler_editor.allocate(slice_int8_build_rawstr(ASSET_FOLDER_ROOT));

    int l_exec = qt_app_start(a, l_asset_compiler_editor.root(), &l_main_window);
    l_asset_compiler_editor.free();

    memleak_ckeck();

    return l_exec;
};