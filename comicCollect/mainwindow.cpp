#include <QDebug>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QtCore>
#include <QtGui>
#include <QListWidget>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
//#include <QSqlRecord>
//#include <QSqlTableModel>
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
    fileMenu->addAction(tr("&About"), this, SLOT(about()));
    //fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"), this, SLOT(close()));

    QMenu *seriesMenu = new QMenu(tr("&Series"), this);
    menuBar()->addMenu(seriesMenu);
    seriesMenu->addAction(tr("&Add Series"), this, SLOT(addSeries()));
    seriesMenu->addAction(tr("&Edit Series"), this, SLOT(editSeries()));
    seriesMenu->addAction(tr("&Delete Series"), this, SLOT(delSeries()));

    QMenu *issueMenu = new QMenu(tr("&Issues"), this);
    menuBar()->addMenu(issueMenu);
    issueMenu->addAction(tr("&Add Issue"), this, SLOT(addIssue()));
    issueMenu->addAction(tr("&Edit Issue"), this, SLOT(editIssue()));
    issueMenu->addAction(tr("&Delete Issue"), this, SLOT(delIssue()));
    issueMenu->addSeparator();
    issueMenu->addAction(tr("&Bulk Add Issue"), this, SLOT(bulkAddIssue()));


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

    updateListWidgets();

}


void MainWindow::updateListWidgets()
{
    seriesList->clear();
    issuesList->clear();
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

    updateIssueList(seriesList->item(0));
    seriesList->item(0)->setSelected(true);
    updateIssueInfo(issuesList->item(0));
    issuesList->item(0)->setSelected(true);
}


void MainWindow::delSeries()
{
    if (lastSeries->text()==0) QMessageBox::information(this, "Error", "No issue selected.");
    else
    {
        bool deleteIssues=false;
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
            QMessageBox delBox;
            delBox.setWindowTitle("Do you...?");
            delBox.setText("Do you want to delete all issues within this series?");
            delBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            delBox.setDefaultButton(QMessageBox::No);


            // Show the dialog as modal
            if(delBox.exec() == QMessageBox::Yes) deleteIssues=true;



            QMessageBox msgBox;
            msgBox.setWindowTitle("Are you sure?");
            msgBox.setText("Do you still want to delete the series?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);


            // Show the dialog as modal
            if(msgBox.exec() == QMessageBox::Ok)
            {
                QSqlQuery query(db);
                // If the user didn't dismiss the dialog, do something
                query.prepare("DELETE FROM series WHERE Name = :name");
                query.bindValue(":name", lastSeries->text());
                if (!query.exec()) QMessageBox::information(this, "Fail", "Fail");
                if (deleteIssues)
                {
                    query.prepare("DELETE FROM issues WHERE Series = :name");
                    query.bindValue(":name", lastSeries->text());
                    if (!query.exec()) QMessageBox::information(this, "Fail", "Fail");
                }

                updateListWidgets();
            }
        }
    }
}


void MainWindow::editSeries()
{
    if (lastSeries->text()==0) QMessageBox::information(this, "Error", "No series selected.");
    else
    {
        int tempID=0;

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

            QDialog dialog(this);
            // Use a layout allowing to have a label next to each field
            QFormLayout form(&dialog);

            // Add some text above the fields
            form.addRow(new QLabel("Edit series:"));

            QString label1 = QString("Name: ");
            QString label2 = QString("Initials: ");

            QLineEdit *lineEdit1 = new QLineEdit(&dialog);
            QLineEdit *lineEdit2 = new QLineEdit(&dialog);
            lineEdit2->setMaxLength(4);

            form.addRow(label1, lineEdit1);
            form.addRow(label2, lineEdit2);

            QSqlQuery query(db);
            query.prepare("SELECT * FROM series WHERE Name = :name");
            query.bindValue(":name", lastSeries->text());

            if (query.exec())
            {
                while (query.next())
                {
                    tempID=query.value(0).toInt();
                    lineEdit1->setText(query.value(1).toString());
                    lineEdit2->setText(query.value(2).toString());
                }
            }

            // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
            QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                       Qt::Horizontal, &dialog);
            form.addRow(&buttonBox);
            QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
            QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

            // Show the dialog as modal
            if (dialog.exec() == QDialog::Accepted)
            {
                // If the user didn't dismiss the dialog, do something with the fields

                query.prepare("UPDATE series SET Name = :name,Short = :short WHERE id = :id");
                query.bindValue(":id", tempID);
                query.bindValue(":name", lineEdit1->text());
                query.bindValue(":short", lineEdit2->text());
                if (!query.exec()) QMessageBox::information(this, "Fail", "Fail");

                //qDebug() << query.lastError().text();
                updateListWidgets();
            }
        }
    }
}


void MainWindow::addSeries()
{
    QDialog dialog(this);
    // Use a layout allowing to have a label next to each field
    QFormLayout form(&dialog);

    // Add some text above the fields
    form.addRow(new QLabel("Add a new series:"));

    QString label1 = QString("Name: ");
    QString label2 = QString("Initials: ");

    QLineEdit *lineEdit1 = new QLineEdit(&dialog);
    QLineEdit *lineEdit2 = new QLineEdit(&dialog);
    lineEdit2->setMaxLength(4);

    form.addRow(label1, lineEdit1);
    form.addRow(label2, lineEdit2);


    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted) {
        // If the user didn't dismiss the dialog, do something with the fields
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
            QSqlQuery query(db);
            //fill issues with all issues

            query.prepare("INSERT INTO series (Name,Short) VALUES (:name,:short)");
            query.bindValue(":name", lineEdit1->text());
            query.bindValue(":short", lineEdit2->text());
            if (!query.exec()) QMessageBox::information(this, "Fail", "Fail");
        }
        updateListWidgets();
    }
}


void MainWindow::delIssue()
{
    if (lastIssue->text()==0) QMessageBox::information(this, "Error", "No issue selected.");
    else
    {
        QString itemText = lastIssue->text();
        QStringList splitOne = itemText.split(" #");
        QStringList splitTwo = splitOne.at(1).split(".");

        int tempID=0;

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

            QMessageBox msgBox;
            msgBox.setWindowTitle("Are you sure?");
            msgBox.setText("Are you sure you wish to delete this item?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);



            QSqlQuery query(db);
            //fill issues with all issues
            if (splitTwo.length() > 1)
            {
                //qDebug() << splitTwo.at(1);
                query.prepare("SELECT * FROM issues WHERE Name = :name AND Number = :number AND NumberAdd = :number_add");
                query.bindValue(":name", splitOne.at(0));
                query.bindValue(":number", splitTwo.at(0));
                query.bindValue(":number_add", splitTwo.at(1));
            }
            else
            {
                query.prepare("SELECT * FROM issues WHERE Name = :name AND Number = :number");
                query.bindValue(":name", splitOne.at(0));
                query.bindValue(":number", splitTwo.at(0));
            }
            if (query.exec())
                while (query.next()) tempID=query.value(0).toInt();



            // Show the dialog as modal
            if(msgBox.exec() == QMessageBox::Ok)
            {
                // If the user didn't dismiss the dialog, do something
                query.prepare("DELETE FROM issues WHERE id=:id");
                query.bindValue(":id", tempID);
                if (!query.exec()) QMessageBox::information(this, "Fail", "Fail");

                updateListWidgets();
            }
        }
    }
}


void MainWindow::editIssue()
{
    if (lastIssue->text()==0) QMessageBox::information(this, "Error", "No issue selected.");
    else
    {
        QString itemText = lastIssue->text();
        QStringList splitOne = itemText.split(" #");
        QStringList splitTwo = splitOne.at(1).split(".");

        int tempID=0;

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

            QDialog dialog(this);
            // Use a layout allowing to have a label next to each field
            QFormLayout form(&dialog);

            // Add some text above the fields
            form.addRow(new QLabel("Edit issue:"));

            QString label1 = QString("Name: ");
            QString label2 = QString("Number: ");
            QString label3 = QString("Year: ");
            QString label4 = QString("Series: ");
            QString label5 = QString("Publisher: ");
            QString label6 = QString("Box: ");
            QString label7 = QString("Number-Add-On: ");
            QString label8 = QString("Tags: ");

            QLineEdit *lineEdit1 = new QLineEdit(&dialog);
            QSpinBox *lineEdit2 = new QSpinBox(&dialog);
            lineEdit2->setMinimum(-999999999);
            lineEdit2->setMaximum(999999999);
            QSpinBox *lineEdit3 = new QSpinBox(&dialog);
            lineEdit3->setMinimum(0);
            lineEdit3->setMaximum(9999);
            QLineEdit *lineEdit4 = new QLineEdit(&dialog);
            QLineEdit *lineEdit5 = new QLineEdit(&dialog);
            QSpinBox *lineEdit6 = new QSpinBox(&dialog);
            lineEdit6->setMinimum(-999999999);
            lineEdit6->setMaximum(999999999);
            QLineEdit *lineEdit7 = new QLineEdit(&dialog);
            QLineEdit *lineEdit8 = new QLineEdit(&dialog);

            QSqlQuery query(db);
            //fill issues with all issues
            if (splitTwo.length() > 1)
            {
                //qDebug() << splitTwo.at(1);
                query.prepare("SELECT * FROM issues WHERE Name = :name AND Number = :number AND NumberAdd = :number_add");
                query.bindValue(":name", splitOne.at(0));
                query.bindValue(":number", splitTwo.at(0));
                query.bindValue(":number_add", splitTwo.at(1));
            }
            else
            {
                query.prepare("SELECT * FROM issues WHERE Name = :name AND Number = :number");
                query.bindValue(":name", splitOne.at(0));
                query.bindValue(":number", splitTwo.at(0));
            }
            if (query.exec())
            {
                while (query.next())
                {
                    tempID=query.value(0).toInt();
                    lineEdit1->setText(query.value(1).toString());
                    lineEdit2->setValue(query.value(2).toInt());
                    lineEdit5->setText(query.value(5).toString());
                    lineEdit3->setValue(query.value(3).toInt());
                    lineEdit4->setText(query.value(4).toString());
                    lineEdit6->setValue(query.value(6).toInt());
                    lineEdit7->setText(query.value(7).toString());
                    lineEdit8->setText(query.value(8).toString());
                }
            }


            form.addRow(label1, lineEdit1);
            form.addRow(label2, lineEdit2);
            form.addRow(label3, lineEdit3);
            form.addRow(label4, lineEdit4);
            form.addRow(label5, lineEdit5);
            form.addRow(label6, lineEdit6);
            form.addRow(label7, lineEdit7);
            form.addRow(label8, lineEdit8);

            // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
            QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                       Qt::Horizontal, &dialog);
            form.addRow(&buttonBox);
            QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
            QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

            // Show the dialog as modal
            if (dialog.exec() == QDialog::Accepted)
            {
                // If the user didn't dismiss the dialog, do something with the fields
                QString tempString=QString::number(lineEdit6->value());
                query.prepare("SELECT Short FROM series WHERE Name=:name");
                query.bindValue(":name", lineEdit4->text());
                if (query.exec())
                    while (query.next()) tempString += query.value(0).toString();

                tempString += QString::number(lineEdit2->value());
                tempString += lineEdit7->text();

                query.prepare("UPDATE issues SET Name=:name,Number=:number,Year=:year,Series=:series,Publisher=:publisher,Box=:box,NumberAdd=:numberadd,Tags=:tags,Serial=:serial WHERE id=:id");
                query.bindValue(":id", tempID);
                query.bindValue(":name", lineEdit1->text());
                query.bindValue(":number", QString::number(lineEdit2->value()));
                query.bindValue(":year", QString::number(lineEdit3->value()));
                query.bindValue(":series", lineEdit4->text());
                query.bindValue(":publisher", lineEdit5->text());
                query.bindValue(":box", QString::number(lineEdit6->value()));
                query.bindValue(":numberadd", lineEdit7->text());
                query.bindValue(":tags", lineEdit8->text());
                query.bindValue(":serial", tempString);
                if (!query.exec()) QMessageBox::information(this, "Fail", "Fail");

                updateListWidgets();
            }
        }
    }
}


void MainWindow::addIssue()
{
    QDialog dialog(this);
    // Use a layout allowing to have a label next to each field
    QFormLayout form(&dialog);

    // Add some text above the fields
    form.addRow(new QLabel("Add a new issue:"));

    QString label1 = QString("Name: ");
    QString label2 = QString("Number: ");
    QString label3 = QString("Year: ");
    QString label4 = QString("Series: ");
    QString label5 = QString("Publisher: ");
    QString label6 = QString("Box: ");
    QString label7 = QString("Number-Add-On: ");
    QString label8 = QString("Tags: ");

    QLineEdit *lineEdit1 = new QLineEdit(&dialog);
    QSpinBox *lineEdit2 = new QSpinBox(&dialog);
    lineEdit2->setMinimum(-999999999);
    lineEdit2->setMaximum(999999999);
    QSpinBox *lineEdit3 = new QSpinBox(&dialog);
    lineEdit3->setMinimum(0);
    lineEdit3->setMaximum(9999);
    QLineEdit *lineEdit4 = new QLineEdit(&dialog);
    QLineEdit *lineEdit5 = new QLineEdit(&dialog);
    QSpinBox *lineEdit6 = new QSpinBox(&dialog);
    lineEdit6->setMinimum(-999999999);
    lineEdit6->setMaximum(999999999);
    QLineEdit *lineEdit7 = new QLineEdit(&dialog);
    QLineEdit *lineEdit8 = new QLineEdit(&dialog);

    form.addRow(label1, lineEdit1);
    form.addRow(label2, lineEdit2);
    form.addRow(label3, lineEdit3);
    form.addRow(label4, lineEdit4);
    form.addRow(label5, lineEdit5);
    form.addRow(label6, lineEdit6);
    form.addRow(label7, lineEdit7);
    form.addRow(label8, lineEdit8);



    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted) {
        // If the user didn't dismiss the dialog, do something with the fields
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
            QSqlQuery query(db);
            //fill issues with all issues

            QString tempString=QString::number(lineEdit6->value());

            query.prepare("SELECT Short FROM series WHERE Name=:name");
            query.bindValue(":name", lineEdit4->text());
            if (query.exec())
                while (query.next()) tempString += query.value(0).toString();

            tempString += QString::number(lineEdit2->value());
            tempString += lineEdit7->text();

            query.prepare("INSERT INTO issues (Name,Number,Year,Series,Publisher,Box,NumberAdd,Tags,Serial) VALUES (:name,:number,:year,:series,:publisher,:box,:numberadd,:tags,:serial)");
            query.bindValue(":name", lineEdit1->text());
            query.bindValue(":number", QString::number(lineEdit2->value()));
            query.bindValue(":year", QString::number(lineEdit3->value()));
            query.bindValue(":series", lineEdit4->text());
            query.bindValue(":publisher", lineEdit5->text());
            query.bindValue(":box", QString::number(lineEdit6->value()));
            query.bindValue(":numberadd", lineEdit7->text());
            query.bindValue(":tags", lineEdit8->text());
            query.bindValue(":serial", tempString);
            if (!query.exec()) QMessageBox::information(this, "Fail", "Fail");

            /**
            qDebug() << query.lastError().text();
            qDebug() << lineEdit1->text();
            qDebug() << lineEdit2->value();
            qDebug() << lineEdit3->value();
            qDebug() << lineEdit4->text();
            qDebug() << lineEdit5->text();
            qDebug() << lineEdit6->value();
            qDebug() << lineEdit7->text();
            qDebug() << lineEdit8->text();
            qDebug() << tempString;
            **/
        }
        updateListWidgets();
    }
}


void MainWindow::bulkAddIssue()
{
    QDialog dialog(this);
    // Use a layout allowing to have a label next to each field
    QFormLayout form(&dialog);

    // Add some text above the fields
    form.addRow(new QLabel("Add a new issue:"));

    QString label1 = QString("Name: ");
    QString label2 = QString("Start-Number: ");
    QString label22 = QString("End-Number: ");
    QString label3 = QString("Year: ");
    QString label4 = QString("Series: ");
    QString label5 = QString("Publisher: ");
    QString label6 = QString("Box: ");
    QString label7 = QString("Number-Add-On: ");
    QString label8 = QString("Tags: ");

    QLineEdit *lineEdit1 = new QLineEdit(&dialog);
    QSpinBox *lineEdit2 = new QSpinBox(&dialog);
    lineEdit2->setMinimum(-999999999);
    lineEdit2->setMaximum(999999999);

    QSpinBox *lineEdit22 = new QSpinBox(&dialog);
    lineEdit22->setMinimum(-999999999);
    lineEdit22->setMaximum(999999999);

    QSpinBox *lineEdit3 = new QSpinBox(&dialog);
    lineEdit3->setMinimum(0);
    lineEdit3->setMaximum(9999);
    QLineEdit *lineEdit4 = new QLineEdit(&dialog);
    QLineEdit *lineEdit5 = new QLineEdit(&dialog);
    QSpinBox *lineEdit6 = new QSpinBox(&dialog);
    lineEdit6->setMinimum(-999999999);
    lineEdit6->setMaximum(999999999);
    QLineEdit *lineEdit7 = new QLineEdit(&dialog);
    QLineEdit *lineEdit8 = new QLineEdit(&dialog);

    form.addRow(label1, lineEdit1);
    form.addRow(label2, lineEdit2);
    form.addRow(label22, lineEdit22);
    form.addRow(label3, lineEdit3);
    form.addRow(label4, lineEdit4);
    form.addRow(label5, lineEdit5);
    form.addRow(label6, lineEdit6);
    form.addRow(label7, lineEdit7);
    form.addRow(label8, lineEdit8);



    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted) {
        // If the user didn't dismiss the dialog, do something with the fields
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
            QSqlQuery query(db);
            //fill issues with all issues

            QString shortForm="";
            query.prepare("SELECT Short FROM series WHERE Name=:name");
            query.bindValue(":name", lineEdit4->text());
            if (query.exec())
                while (query.next()) shortForm = query.value(0).toString();

            int numberCounter=lineEdit22->value()-lineEdit2->value()+1;

            db.transaction();
            for (int i=0; i<numberCounter; i++)
            {
                QString tempString=QString::number(lineEdit6->value());
                tempString += shortForm;
                tempString += QString::number(lineEdit2->value()+i);
                tempString += lineEdit7->text();

                query.prepare("INSERT INTO issues (Name,Number,Year,Series,Publisher,Box,NumberAdd,Tags,Serial) VALUES (:name,:number,:year,:series,:publisher,:box,:numberadd,:tags,:serial)");
                query.bindValue(":name", lineEdit1->text());
                query.bindValue(":number", QString::number(lineEdit2->value()+i));
                query.bindValue(":year", QString::number(lineEdit3->value()));
                query.bindValue(":series", lineEdit4->text());
                query.bindValue(":publisher", lineEdit5->text());
                query.bindValue(":box", QString::number(lineEdit6->value()));
                query.bindValue(":numberadd", lineEdit7->text());
                query.bindValue(":tags", lineEdit8->text());
                query.bindValue(":serial", tempString);
                query.exec();
            }
            db.commit();
        }
        updateListWidgets();
    }
}


void MainWindow::updateIssueList(QListWidgetItem *item)
{
    issuesList->clear();
    lastSeries=item;
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
    updateIssueInfo(issuesList->item(0));
    issuesList->item(0)->setSelected(true);
    //lastIssue = issuesList->item(0);
}


void MainWindow::updateIssueInfo(QListWidgetItem *item)
{
    lastIssue=item;
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
            //qDebug() << splitTwo.at(1);
            query.prepare("SELECT * FROM issues WHERE Name = :name AND Number = :number AND NumberAdd = :number_add");
            query.bindValue(":name", splitOne.at(0));
            query.bindValue(":number", splitTwo.at(0));
            query.bindValue(":number_add", splitTwo.at(1));
        }
        else
        {
            query.prepare("SELECT * FROM issues WHERE Name = :name AND Number = :number");
            query.bindValue(":name", splitOne.at(0));
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
                iNumberAddLabel->setText(query.value(7).toString());
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
    lastSeries = new QListWidgetItem;
    lastIssue = new QListWidgetItem;
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

void MainWindow::about()
{
    QMessageBox::information(this, "About", "This application was created by Rona Erdem.");
}
