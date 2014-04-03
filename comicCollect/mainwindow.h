#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QString>
#include <QSqlDatabase>

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    public:
        MainWindow(QWidget *parent = 0);
        ~MainWindow();

    private:
        QString collectionpath;
        QStringList dblist;
        QSqlDatabase db;
        QWidget *centralWidget;

        void createInterface();
        void initialize();
        bool setupDb();


    private slots:
};

#endif // MAINWINDOW_H

