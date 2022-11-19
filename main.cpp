#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QLocale>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Qt sets the locale in the QApplication constructor, but libmpv requires
    // the LC_NUMERIC category to be set to "C", so change it back.
    setlocale(LC_NUMERIC, "C");
    //QLocale br(QLocale::Portuguese, QLocale::Brazil);
    //QLocale::setDefault(br);

    MainWindow w;
    w.show();
    return a.exec();
}
