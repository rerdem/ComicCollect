#include <QString>
#include <QDir>
#include <QtCore>
#include <QtGui>
#include <QListWidget>
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
    dbfilename="myCollection.db";

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

    this->setFixedSize(800, 600);

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


    seriesList = new QListWidget;
    issuesList = new QListWidget;

    iNameLabel = new QLabel(this);
    iNumberLabel = new QLabel(this);
    iPubLabel = new QLabel(this);
    iYearLabel = new QLabel(this);
    iSeriesLabel = new QLabel(this);
    iBoxLabel = new QLabel(this);
    iNumberAddLabel = new QLabel(this);
    iTagsLabel = new QLabel(this);
    iSerialLabel = new QLabel(this);

    iNameLabel->setText("Name: Test");
    iNumberLabel->setText("# 12");
    iPubLabel->setText("Publisher: Marvel");
    iYearLabel->setText("Year: 2014");
    iSeriesLabel->setText("Series: TestSeries");
    iBoxLabel->setText("Box: 1");
    iNumberAddLabel->setText("LEER");;
    iTagsLabel->setText("Tags: Action, Fantasy");
    iSerialLabel->setText("Serial: 1TS12LEER");

    issueBox = new QGridLayout();
    issueBox->addWidget(iNameLabel, 0,1,1,1);
    issueBox->addWidget(iNumberLabel, 0,2,1,1);
    issueBox->addWidget(iNumberAddLabel, 0,3,1,1);
    issueBox->addWidget(iSeriesLabel, 1,1,1,1);
    issueBox->addWidget(iYearLabel, 1,3,1,1);
    issueBox->addWidget(iPubLabel, 2,1,1,1);
    issueBox->addWidget(iBoxLabel, 2,3,1,1);
    issueBox->addWidget(iSerialLabel, 3,1,1,1);
    issueBox->addWidget(iTagsLabel, 4,1,1,3);

    //create and fill Layout
    mainBox = new QGridLayout(centralWidget);
    mainBox->addWidget(seriesList, 1, 0, 5, 1);
    mainBox->addWidget(issuesList, 1, 1, 5, 1);
    mainBox->addLayout(issueBox, 1, 2);



    //fülle wb1
    QString dbpath;
    QDir path = QDir::currentPath() + QDir::separator() +"collections";
    dbpath = path.absolutePath() + QDir::separator() + dbfilename;
    Q_ASSERT(dbpath.length()>0);

    //nutze SQLITE
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbpath);

    if(db.open())
    {
        QSqlQuery query(db);
        if (query.exec("SELECT Name FROM series"))
        {
            while (query.next())
            {
                QListWidgetItem *newItem = new QListWidgetItem;
                newItem->setText(query.value(0).toString());
                int row = seriesList->row(seriesList->currentItem());
                seriesList->insertItem(row, newItem);
            }
        }
        if (query.exec("SELECT Name, Number, NumberAdd FROM issues"))
        {
            while (query.next())
            {
                QListWidgetItem *newItem = new QListWidgetItem;
                QString itemText=query.value(0).toString() + "#" + query.value(1).toString();
                if (query.value(2).toString() != "") itemText += "." + query.value(2).toString();
                newItem->setText(itemText);
                int row = issuesList->row(issuesList->currentItem());
                issuesList->insertItem(row, newItem);
            }
        }
    }


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
    dbfilename="myCollection.db";

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
            query.exec("CREATE TABLE issues (id INTEGER PRIMARY KEY, Name VARCHAR(512), Number INTEGER, Year INTEGER, Series INTEGER, Publisher VARCHAR(512), Box VARCHAR(16), NumberAdd VARCHAR(16), Tags VARCHAR(512), Serial VARCHAR(512))");
        }
    }
    else return false;
    return true;
}

