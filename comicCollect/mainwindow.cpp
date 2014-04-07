#include <QString>
#include <QStringList>
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

    //this->setFixedSize(800, 600);

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
    seriesList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(seriesList, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(updateIssueList(QListWidgetItem *)));

    issuesList = new QListWidget;
    issuesList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(issuesList, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(updateIssueInfo(QListWidgetItem *)));

    iNameLabel = new QLabel(this);
    iNumberLabel = new QLabel(this);
    iPubLabel = new QLabel(this);
    iYearLabel = new QLabel(this);
    iSeriesLabel = new QLabel(this);
    iBoxLabel = new QLabel(this);
    iNumberAddLabel = new QLabel(this);
    iTagsLabel = new QLabel(this);
    iSerialLabel = new QLabel(this);

    iNameLabel->setText("Name: ");
    iNumberLabel->setText("#");
    iPubLabel->setText("Publisher: ");
    iYearLabel->setText("Year: ");
    iSeriesLabel->setText("Series: ");
    iBoxLabel->setText("Box: ");
    iNumberAddLabel->setText("");;
    iTagsLabel->setText("Tags: ");
    iSerialLabel->setText("Serial: ");

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



    //set dbpath
    QString dbpath;
    QDir path = QDir::currentPath() + QDir::separator() +"collections";
    dbpath = path.absolutePath() + QDir::separator() + dbfilename;
    Q_ASSERT(dbpath.length()>0);

    //use SQLITE
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbpath);

    if(db.open())
    {
        //fill series list with all series
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
            QListWidgetItem *newItem = new QListWidgetItem;
            newItem->setText("###All###");
            int row = seriesList->row(seriesList->currentItem());
            seriesList->insertItem(row, newItem);
        }
        //fill issues with all issues
        if (query.exec("SELECT Name, Number, NumberAdd FROM issues"))
        {
            while (query.next())
            {
                QListWidgetItem *newItem = new QListWidgetItem;
                QString itemText=query.value(0).toString() + " #" + query.value(1).toString();
                if (query.value(2).toString() != "") itemText += "." + query.value(2).toString();
                newItem->setText(itemText);
                int row = issuesList->row(issuesList->currentItem());
                issuesList->insertItem(row, newItem);
            }
        }
    }
    seriesList->sortItems(Qt::AscendingOrder);
    issuesList->sortItems(Qt::AscendingOrder);
}


void MainWindow::updateIssueList(QListWidgetItem *item)
{
    issuesList->clear();
    //set dbpath
    QString dbpath;
    QDir path = QDir::currentPath() + QDir::separator() +"collections";
    dbpath = path.absolutePath() + QDir::separator() + dbfilename;
    Q_ASSERT(dbpath.length()>0);

    //use SQLITE
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbpath);

    QString itemText = item->text();

    if(db.open())
    {
        QSqlQuery query(db);
        //fill issues with all issues
        if (itemText=="###All###") query.prepare("SELECT Name, Number, NumberAdd FROM issues");
        else
        {
            query.prepare("SELECT Name, Number, NumberAdd FROM issues WHERE Series = :series_name");
            query.bindValue(":series_name", itemText);
        }
        if (query.exec())
        {
            while (query.next())
            {
                QListWidgetItem *newItem = new QListWidgetItem;
                QString itemText=query.value(0).toString() + " #" + query.value(1).toString();
                if (query.value(2).toString() != "") itemText += "." + query.value(2).toString();
                newItem->setText(itemText);
                int row = issuesList->row(issuesList->currentItem());
                issuesList->insertItem(row, newItem);
            }
        }
    }
    issuesList->sortItems(Qt::AscendingOrder);
}


void MainWindow::updateIssueInfo(QListWidgetItem *item)
{
    //set dbpath
    QString dbpath;
    QDir path = QDir::currentPath() + QDir::separator() +"collections";
    dbpath = path.absolutePath() + QDir::separator() + dbfilename;
    Q_ASSERT(dbpath.length()>0);

    //use SQLITE
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbpath);

    QString itemText = item->text();
    QStringList splitOne = itemText.split(" #");
    QStringList splitTwo = splitOne.at(1).split(".");

    if(db.open())
    {
        QSqlQuery query(db);
        //fill issues with all issues
        if (splitTwo.length() > 1)
        {
            qDebug() << splitTwo.at(1);
            query.prepare("SELECT * FROM issues WHERE Series = :series_name AND Number = :number AND NumberAdd = :number_add");
            query.bindValue(":series_name", splitOne.at(0));
            query.bindValue(":number", splitTwo.at(0));
            query.bindValue(":number_add", splitTwo.at(1));
        }
        else
        {
            query.prepare("SELECT * FROM issues WHERE Series = :series_name AND Number = :number");
            query.bindValue(":series_name", splitOne.at(0));
            query.bindValue(":number", splitTwo.at(0));
        }
        if (query.exec())
        {
            while (query.next())
            {
                iNameLabel->setText("Name: " + query.value(1).toString());
                iNumberLabel->setText("#" + query.value(2).toString());
                iPubLabel->setText("Publisher: " + query.value(5).toString());
                iYearLabel->setText("Year: " + query.value(3).toString());
                iSeriesLabel->setText("Series: " + query.value(4).toString());
                iBoxLabel->setText("Box: " + query.value(6).toString());
                iNumberAddLabel->setText(query.value(7).toString());;
                iTagsLabel->setText("Tags: " + query.value(8).toString());
                iSerialLabel->setText("Serial: " + query.value(9).toString());
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

