#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // zhr add here
    QTranslator qtTranslator;
    if (qtTranslator.load(QApplication::applicationDirPath() + "/zh_CN.qm"))
    {
        a.installTranslator(&qtTranslator);
    }

    MainWindow w;
    w.show();
    return a.exec();
}
