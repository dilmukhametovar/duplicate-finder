#include "ui/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("DuplicateFinder");

    MainWindow w;
    w.resize(750, 600);
    w.show();
    return a.exec();
}
