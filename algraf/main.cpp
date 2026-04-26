#include <QApplication>
#include <QLocale>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    MainWindow window;
    window.show();

    return app.exec();
}
