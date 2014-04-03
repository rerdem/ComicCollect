#include <QString>
#include <QDir>
#include <QtCore>
#include <QtGui>
#include <QMessageBox>
#include <QSqlQuery>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //set QString to UTF-8 encoding for special characters
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    //create collections folder, if it doesn't exist
    QDir path;
    QDir().mkdir("collections");
    path = QDir::currentPath() + QDir::separator() +"collections";
    //read all filenames from collections into dblist
    dblist = path.entryList( QDir::Files, QDir::Name );
    collectionpath=QDir::currentPath() + QDir::separator() + "collections" + QDir::separator();

    initialize();
    createInterface();

}

MainWindow::~MainWindow()
{
    
}


void MainWindow::createInterface()
{
    centralWidget = new QWidget(this);
    this->setCentralWidget( centralWidget );

    //this->setWindowState(Qt::WindowMaximized);

    //create file menu
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(fileMenu);
    fileMenu->addAction(tr("&About"), this, SLOT(close()));
    fileMenu->addSeparator();
    //fileMenu->addAction(tr("&New Game"), this, SLOT(reset()));
    //fileMenu->addAction(tr("&Save"), this, SLOT(save()));
    //fileMenu->addAction(tr("&Load"), this, SLOT(load()));
    //fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"), this, SLOT(close()));

}

void MainWindow::initialize()
{
    if (dblist.isEmpty())
    {
        if(!setupDb())
        {
            QMessageBox::critical(this, tr("DB-Error!"),
                                  tr("Database could not be created!"),
                                  QMessageBox::Ok);
        }
        else
        {
            //refresh dblist
            QDir path = QDir::currentPath() + QDir::separator() +"collections";
            dblist = path.entryList( QDir::Files, QDir::Name );
        }
    }
}






bool MainWindow::setupDb()
{
    Q_ASSERT(collectionpath.length()>0);

    QString dbname = QDir::currentPath() + QDir::separator() + "collections" + QDir::separator() + "myCollection.db";

    //nutze SQLITE
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbname);

    if(db.open())
    {
        //suche DB, wenn keine da ist, erstelle neue
        bool db_gefunden = false;
        foreach (QString table, db.tables())
        {
            if(table == "issues")
            {
                db_gefunden = true;
                break;
            }
        }
        if(!db_gefunden)
        {
            QSqlQuery query(db);
            query.exec("CREATE TABLE series (id INTEGER PRIMARY KEY, Name VARCHAR(512), Short VARCHAR(8))");
            query.exec("CREATE TABLE issues (id INTEGER PRIMARY KEY, Name VARCHAR(512), Number INTEGER, Year INTEGER, Series INTEGER, Box VARCHAR(16), NumberAdd VARCHAR(16), Tags VARCHAR(512))");
        }
    }
    else return false;
    return true;
}

