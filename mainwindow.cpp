#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "formarticles.h"
#include "formbank.h"
#include "formcontract.h"
#include "formcounterparties.h"
#include "formimport.h"
#include "formoptions.h"


#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QDebug>
#include <QSettings>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

#include <QtPrintSupport/QPrinter>
#include <QPrintDialog>
#include <QTextStream>
#include <QTextDocument>

#include <QPdfWriter>
#include <QPainter>
#include <QDesktopServices>
#include <QCommandLineParser>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    database = QSqlDatabase::addDatabase("QSQLITE","main");

    //сбрасываем тригеры меню
    on_actionCloseBase_triggered();

    // если есть параметры в командной строке то открываем
    if (QCoreApplication::arguments().count() > 1) {
        databaseName=QCoreApplication::arguments().at(1);
        on_actionOpenBase_triggered();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::OpenBase()
{
    // читаем из настроек имя базы
//    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
//    databaseName = settings.value(SETTINGS_BASE_NAME, "").toString();

    //проверяем на наличие файл базы
    if(!QFile(databaseName).exists()){
        qDebug() << "Файла базы нет!";
    //    this->setWindowTitle(tr("Файла базы нет!"));
    }

    // открываем базу
    database.setDatabaseName(databaseName);
    if(!database.open()){
      qDebug() << "Ошибка открытия базы!";
      this->setWindowTitle("Error!");
      return false;
    }
    // титульный окна имя базы
    this->setWindowTitle("Аудит: " + databaseName);
    return true;
}

void MainWindow::on_actionBank_triggered()
{
    FormBank  *bank = new FormBank(database,this);
    ui->tabWidgetMain->insertTab(0,bank,tr("Банк"));
    ui->tabWidgetMain->setCurrentIndex(0);
}

void MainWindow::on_actionOpenBase_triggered()
{
    // отктыьб базу данных
    // выбор файла базы данных
    if (databaseName.isEmpty())
     databaseName = QFileDialog::getOpenFileName(this,tr("Open base"),"./",tr("Data base Fules (*.db)"));

     if (!databaseName.isEmpty()) {
        // закрывкем старую
            database.close();
            // сохранить в настройказ имя базы
    //        QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
            //пишем настройки
    //        settings.setValue(SETTINGS_BASE_NAME, filename);
    //        settings.sync();
            // открыть меню
            ui->actionOpenBase->setEnabled(false);
            ui->actionCloseBase->setEnabled(true);

            ui->actionSaveAs->setEnabled(true);
            // открываем новую базу
            OpenBase();
     }

}

void MainWindow::on_actionCloseBase_triggered()
{
    // закрыть все вкладки
    ui->tabWidgetMain->clear();

    //закрыть базу
    databaseName="";
    database.close();
    this->setWindowTitle("Аудит: no base");
    ui->actionCloseBase->setEnabled(false);
    ui->actionOpenBase->setEnabled(true);

    ui->actionSaveAs->setEnabled(false);

}

void MainWindow::on_actionImportPP_triggered()
{
    FormImport  *import = new FormImport(database,this);
    ui->tabWidgetMain->insertTab(0,import,tr("Импорт"));
    ui->tabWidgetMain->setCurrentIndex(0);
}

void MainWindow::on_actionOptions_triggered()
{
    FormOptions *options = new FormOptions(database,this);
    ui->tabWidgetMain->insertTab(0,options,tr("Настройки"));
    ui->tabWidgetMain->setCurrentIndex(0);
}

void MainWindow::on_actionContracts_triggered()
{
    // вызов редактора контрагентов

    FormContract *contract = new FormContract(database,this);
    ui->tabWidgetMain->insertTab(0,contract,tr("Контракты"));
    ui->tabWidgetMain->setCurrentIndex(0);


}

void MainWindow::on_actionArticles_triggered()
{
    // вызов редактора статей

    FormArticles *articles = new FormArticles(database,this);
    ui->tabWidgetMain->insertTab(0,articles,tr("Статьи"));
    ui->tabWidgetMain->setCurrentIndex(0);

}

void MainWindow::on_actionCounterparties_triggered()
{
    // вызов редактора контрагентов

    FormCounterparties *counterparties = new FormCounterparties(database,this);
    ui->tabWidgetMain->insertTab(0,counterparties,tr("Контрагенты"));
    ui->tabWidgetMain->setCurrentIndex(0);

}
