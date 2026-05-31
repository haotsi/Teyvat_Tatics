#include <QApplication>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Teyvat Tactics");
    app.setApplicationVersion("1.0");

    MainWindow window;
    window.show();

    return app.exec();
}
