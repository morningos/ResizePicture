#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MainWindow w;
    w.showMaximized();
    return app.exec();
}
