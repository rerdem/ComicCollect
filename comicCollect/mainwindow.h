#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QString>
#include <QListWidget>
#include <QSqlDatabase>

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    public:
        MainWindow(QWidget *parent = 0);
        ~MainWindow();

    private:
        QString collectionpath;
        QString dbfilename;
        QStringList dblist;
        QSqlDatabase db;

        QWidget *centralWidget;

        QListWidget *seriesList;
        QListWidget *issuesList;

        QGridLayout *mainBox;
        QGridLayout *issueBox;

        QLabel *iNameLabel;
        QLabel *iNumberLabel;
        QLabel *iPubLabel;
        QLabel *iYearLabel;
        QLabel *iSeriesLabel;
        QLabel *iBoxLabel;
        QLabel *iNumberAddLabel;
        QLabel *iTagsLabel;
        QLabel *iSerialLabel;

        void createInterface();
        void initialize();
        bool setupDb();


    private slots:
        void updateIssueList(QListWidgetItem *item);
        void updateIssueInfo(QListWidgetItem *item);
};

#endif // MAINWINDOW_H

