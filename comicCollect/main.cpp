#include <QtGui/QApplication>
#include "mainwindow.h"

//ComicCollect was created by Rona Erdem 2014

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
