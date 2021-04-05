#include "AssetCompiler/asset_compiler_editor.hpp"

struct qt_test
{
    QApplication app;
    QMainWindow* main_window;

    inline static qt_test allocate(int argc, char* argv[])
    {
        return qt_test{QApplication(argc, argv), NULL};
    };

    inline int start(QWidget* p_central_widget)
    {
        return qt_app_start(this->app, p_central_widget, &this->main_window);
    };

    inline void close()
    {
        this->main_window->close();
    };

    inline void free()
    {
        this->app.processEvents();
        this->app.exit(0);
    };
};

// TODO -> write test
/*
    1/ Loading configurations from file and ensure that the requested compilation pass have been taken into account
    2/

*/

int main(int argc, char* argv[])
{
    qt_test app = qt_test::allocate(argc, argv);


    app.free();
    memleak_ckeck();
    return 0;
}