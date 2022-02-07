#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "formarticles.h"
#include "formbank.h"
#include "formcontract.h"
#include "formcounterparties.h"
#include "formimport.h"
#include "formoptions.h"
#include "formquery.h"


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
#include <QSettings>





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

    SetHistory();


    // сигнал создания запроса
    connect(this,&MainWindow::signalFromQuery,this,&MainWindow::slot_goQuery);



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

    //читаем настнойки из базы
    QSqlQuery query(database);
    if (!query.exec("SELECT organization, date_begin, date_end FROM options"))
            qDebug() << "Ошибка чтения настроек: " << query.lastError().text();
     query.first();

    // титульный окна имя базы
    this->setWindowTitle("Аудит: '" + query.value(0).toString() + "' База: "+ databaseName);

     // добавляем имя базы в историю
     QSettings settings(QSettings::IniFormat, QSettings::UserScope, ORGANIZATION_NAME, APPLICATION_NAME);
 //     settings.setValue(SETTINGS_BASE_FILE1, "");
     QString file1=settings.value(SETTINGS_BASE_FILE1, "").toString();
     QString file2=settings.value(SETTINGS_BASE_FILE2, "").toString();
     QString file3=settings.value(SETTINGS_BASE_FILE3, "").toString();

     if(file1!=databaseName) {
         // сохраняем
         settings.setValue(SETTINGS_BASE_FILE3, file2);
         settings.setValue(SETTINGS_BASE_FILE2, file1);
         settings.setValue(SETTINGS_BASE_FILE1, databaseName);
         // сбвигаем меню
         ui->actionFile01->setText(databaseName);
         ui->actionFile02->setText(file1);
         ui->actionFile03->setText(file2);
     }


     return true;
}

void MainWindow::SetHistory()
{
    // чтот динамически не вышло -  пока сделаю по простому
    // gпрочитать и добавить историю открытий баз

    // считать из настроек имя базы

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, ORGANIZATION_NAME, APPLICATION_NAME);
//     settings.setValue(SETTINGS_BASE_FILE1, "");
    QString file1=settings.value(SETTINGS_BASE_FILE1, "").toString();
    QString file2=settings.value(SETTINGS_BASE_FILE2, "").toString();
    QString file3=settings.value(SETTINGS_BASE_FILE3, "").toString();

//    if(!file1.isEmpty())
//        ui->menu->addAction(file1, this, SLOT(quit()));
//    if(!file2.isEmpty())
//        ui->menu->addAction(file2, this, SLOT(slot_OpenBase(file2)));
//    if(!file3.isEmpty())
//        ui->menu->addAction(file3, this, SLOT(slot_OpenBase(file3)));

    if(!file1.isEmpty())
        ui->actionFile01->setText(file1);
    if(!file2.isEmpty())
        ui->actionFile02->setText(file2);
    if(!file3.isEmpty())
        ui->actionFile03->setText(file3);
}

void MainWindow::on_actionBank_triggered()
{
    FormBank  *bank = new FormBank(database,this);
    ui->tabWidgetMain->insertTab(0,bank,tr("Банк"));
    ui->tabWidgetMain->setCurrentIndex(0);
}

void MainWindow::slot_OpenBase(QString fb)
{
    qDebug() << "Open history";

    databaseName = fb;
    on_actionOpenBase_triggered();
}


void MainWindow::on_actionFile01_triggered()
{
    //файл истории 01
    if(!ui->actionFile01->text().isEmpty()) {
        databaseName = ui->actionFile01->text();
        on_actionOpenBase_triggered();
    }
}

void MainWindow::on_actionFile02_triggered()
{
    //файл истории 02
    if(!ui->actionFile02->text().isEmpty()) {
        databaseName = ui->actionFile02->text();
        on_actionOpenBase_triggered();
    }

}

void MainWindow::on_actionFile03_triggered()
{
    //файл истории 03
    if(!ui->actionFile03->text().isEmpty()) {
        databaseName = ui->actionFile03->text();
        on_actionOpenBase_triggered();
    }

}


void MainWindow::on_actionOpenBase_triggered()
{
    // отктыьб базу данных
    // выбор файла базы данных
    if (databaseName.isEmpty())
     databaseName = QFileDialog::getOpenFileName(this,tr("Open base"),"./",QString("Data base Files (*%1);; All file (*)").arg(FILE_EXT));

    // если файл присутствует
    //проверяем на наличие файл базы
    if(!QFile(databaseName).exists()){
        qDebug() << "Файла базы нет!";
        QMessageBox::information(this,"Error","Выбранная база не существует!");
        return;
    }

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

            ui->actionBank->setEnabled(true);
            ui->actionArticles->setEnabled(true);
            ui->actionCounterparties->setEnabled(true);
            ui->actionContracts->setEnabled(true);
            ui->actionOptions->setEnabled(true);
            ui->actionImportPP->setEnabled(true);
            ui->actionRep_bank->setEnabled(true);
            ui->actionRepContracts->setEnabled(true);
            ui->actionRepContracsShort->setEnabled(true);
            ui->actionRepContractsIsNote->setEnabled(true);
            ui->actionRepContractsFor->setEnabled(true);

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

    ui->actionBank->setEnabled(false);
    ui->actionArticles->setEnabled(false);
    ui->actionCounterparties->setEnabled(false);
    ui->actionContracts->setEnabled(false);
    ui->actionOptions->setEnabled(false);
    ui->actionImportPP->setEnabled(false);
    ui->actionRep_bank->setEnabled(false);
    ui->actionRepContracts->setEnabled(false);
    ui->actionRepContracsShort->setEnabled(false);
    ui->actionRepContractsIsNote->setEnabled(false);
    ui->actionRepContractsFor->setEnabled(false);

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

void MainWindow::on_actionSaveAs_triggered()
{
    // дублирование базы
    if (!databaseName.isEmpty()) {
        // запросить новое имя
        QString  newBase = QFileDialog::getSaveFileName(this,tr("Сохранить как"),databaseName,QString("Data base Fules (*%1);; All (*)").arg(FILE_EXT));
        // если дано новое имя
        if (databaseName != newBase) {
            database.close();
            qDebug() << "copy " << databaseName << " to " << newBase;
            //если нет расширения добавляем
            if (newBase.indexOf(FILE_EXT)==-1)
                newBase.append(FILE_EXT);

            // возможно надо проверить не открыта ли база кем то еще
            if (!QFile::copy(databaseName,newBase)) {
                    QMessageBox::critical(this,"ERROR","База не скопирована!");
                    return;
            }
            databaseName = newBase;
        }
        on_actionOpenBase_triggered();
    }
}

void MainWindow::on_actionQuery_triggered()
{
    // вызов окна запроса
    // тест
    emit  signalFromQuery("SELECT DATE('now')");

}

void MainWindow::slot_goQuery(QString sq)
{
    // вызов окна запроса
    // придумать как вызвать из других окон и передать параметры!
    FormQuery *query = new FormQuery(database, sq, this);
    ui->tabWidgetMain->insertTab(0,query,tr("Запрос"));
    ui->tabWidgetMain->setCurrentIndex(0);

}


void MainWindow::on_actionExit_triggered()
{
        QApplication::closeAllWindows();
}

void MainWindow::on_actionAbout_triggered()
{
    //об авторах
    // дата компиляции
    //const QString BUILDV =  QStringLiteral(__DATE__ " " __TIME__);

    QMessageBox::information(this,"Info",QString("Программа ведения финансового аудита\n в разрезе статей затрат.\n\n %1 %2 \nggorinich@gmail.com").arg(VER_PROG).arg(BUILDV));
}


void MainWindow::on_actionNewBase_triggered()
{
    // выбор файла базы данных
    QString newBase =  QFileDialog::getSaveFileName(this,tr("Create new base"),"./",QString("Data base Fules (*%1);;All (*)").arg(FILE_EXT));

     if (!newBase.isEmpty()) {
        // создаем

         //если нет расширения добавляем
         if (newBase.indexOf(FILE_EXT)==-1)
             newBase.append(FILE_EXT);

         //проверяем на наличие файл базы
         if(QFile(newBase).exists()){
             qDebug() << "Файла базы есть!";
             QMessageBox::information(this,"Error","Выбранная база уже существует. Выберете другое имя!");
             return;
         }

     // открываем базу
         QSqlDatabase dbm = QSqlDatabase::addDatabase("QSQLITE","new");
         dbm.setDatabaseName(newBase);
         if(!dbm.open()){
           qDebug() << "Ошибка открытия базы!";
           QMessageBox::critical(this,"Error",dbm.lastError().text());
           return;
         }

         QSqlQuery a_query = QSqlQuery(dbm);

         // запрос на создание таблицы ПП
         QString str = "CREATE TABLE bank ("
             "id                    INTEGER         PRIMARY KEY AUTOINCREMENT"
                                                   " UNIQUE,"
             "payment_number        VARCHAR,"
             "payment_date          DATE            DEFAULT [2000-01-01],"
             "counterparty_id       INTEGER         REFERENCES counterparties (id),"
             "decryption_of_payment TEXT,"
             "amount_of_payment     DECIMAL (20, 2) DEFAULT (0),"
             "this_receipt          BOOLEAN         DEFAULT (0),"
             "article               VARCHAR,"
             "note                  VARCHAR"
         ");";
         if (!a_query.exec(str))
             qDebug() << "таблица ПП: " << a_query.lastError().text();


         // запрос на создание таблицы Расшифровок
         str = "CREATE TABLE bank_decryption ("
             "id                   INTEGER         PRIMARY KEY AUTOINCREMENT"
                                                  " UNIQUE,"
             "bank_id              INTEGER         REFERENCES bank (id),"
             "sum                  DECIMAL (20, 2),"
             "article_id           INTEGER         REFERENCES articles (id),"
             "contract_id          INTEGER         REFERENCES contracts (id),"
             "expense_confirmation BOOLEAN         DEFAULT (0) "
         ");";
         if (!a_query.exec(str))
             qDebug() << "таблица Расшифровок: " << a_query.lastError().text();

         // запрос на создание таблицы Статей
         str = "CREATE TABLE articles ("
             "id      INTEGER PRIMARY KEY AUTOINCREMENT"
                             " UNIQUE,"
             "article VARCHAR,"
             "code    VARCHAR,"
             "subcode VARCHAR,"
             "f14     VARCHAR,"
             "note    TEXT"
         ");";
         if (!a_query.exec(str))
             qDebug() << "таблица Статей: " << a_query.lastError().text();

         // запрос на создание таблицы Контрагентов
         str = "CREATE TABLE counterparties ("
             "id           INTEGER PRIMARY KEY AUTOINCREMENT"
                                  " UNIQUE,"
             "counterparty TEXT,"
             "note         VARCHAR"
         ");";
         if (!a_query.exec(str))
             qDebug() << "таблица Контрагентов: " << a_query.lastError().text();

         // запрос на создание таблицы Контрактов
         str = "CREATE TABLE contracts ("
             "id              INTEGER         PRIMARY KEY AUTOINCREMENT"
                                             " UNIQUE,"
             "contract_number VARCHAR,"
             "contract_date   DATE            DEFAULT [2000-01-01],"
             "due_date        DATE            DEFAULT [2000-01-01],"
             "counterparty_id INTEGER         REFERENCES counterparties (id),"
             "price           DECIMAL (20, 2) DEFAULT (0),"
             "state_contract  BOOLEAN         DEFAULT (0),"
             "completed       BOOLEAN         DEFAULT (0),"
             "found           BOOLEAN         DEFAULT (0),"
             "for_audit       BOOLEAN         DEFAULT (0), "
             "note            VARCHAR, "
             "for_check       BOOLEAN         DEFAULT (0)"
         ");";
         if (!a_query.exec(str))
             qDebug() << "таблица Контрактов: " << a_query.lastError().text();

         // запрос на создание таблицы Настроек
         str = "CREATE TABLE options ("
             "organization       VARCHAR,"
             "date_begin         DATE,"
             "date_end           DATE,"
             "rep_contract_found BOOLEAN DEFAULT (0),"
             "regexp_c           VARCHAR"
         ");";
         if (!a_query.exec(str))
             qDebug() << "таблица Настроек: " << a_query.lastError().text();
         // вставить строку насстноек
         str = "INSERT INTO options (organization, regexp_c) values ('','(кон|конт|контр|контракт|дог|догов|договор)(.|)\\s{0,}(N|)\\s{0,}(\\S{1,})\\s{0,}от\\s{0,}((0[1-9]|[12][0-9]|3[01])[-\\.](0[1-9]|1[012])[-\\.]((19|20)(\\d{2})|\\d{2}))\\D');";
         if (!a_query.exec(str))
             qDebug() << "таблица настроек строка: " << a_query.lastError().text();


         //
         QMessageBox::information(this,"Info","Операция завершена.");
         dbm.close();

         // откроем созданную базу
        databaseName=newBase;
        on_actionOpenBase_triggered();
     }

}



void MainWindow::on_actionRep_bank_triggered()
{
    // главный отчет по банку
    // локаль для сумм

    // база открыта?
    if(!database.isOpen()){
          qDebug() << "База не открыта!";
          QMessageBox::critical(this,"Error","База не открыта!");
          return;
    }

    QSqlQuery a_query = QSqlQuery(database);

    //читаем настнойки из базы
    QString organization = "";
    QString dateBegin = "";
    QString dateEnd = "";

    if (!a_query.exec("SELECT organization, date_begin, date_end FROM options"))
            qDebug() << "Ошибка чтения настроек: " << a_query.lastError().text();
    else {
        a_query.first();
        organization = a_query.value(0).toString();
        dateBegin = a_query.value(1).toString();
        dateEnd = a_query.value(2).toString();
    }


    // фильтр дат
    QString flt="";
    if (!dateBegin.isEmpty() && !dateEnd.isEmpty())
        flt=QString("WHERE bank.payment_date >= '%1' AND bank.payment_date <= '%2'").arg(dateBegin).arg(dateEnd);
    // запрос
    QString query = QString("SELECT strftime('%Y',bank.payment_date), bank.this_receipt, articles.article, ROUND(SUM(sum),2), COUNT(bank.payment_number) FROM bank_decryption inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id %1 GROUP BY strftime('%Y',bank.payment_date), bank.this_receipt, articles.article;").arg(flt);
    if (!a_query.exec(query)) {
         qDebug() << "Ошибка запроса отчета: " << a_query.lastError().text();
         return;
    }
    qDebug() << flt;
    qDebug() << query;

    // формирование и печать

    QString stringStream;
    QTextStream out(&stringStream);


    // формирование отчета
    out << "<html>\n" << "<head>\n" << "meta Content=\"Text/html;charsrt=Windows-1251\">\n" <<
           QString("<title>%1</title>\n").arg("Report") <<

           "<style>"
           "table {"
            "width: 100%; /* Ширина таблицы */"
//            "border-collapse: collapse; /* Убираем двойные линии */"
//            "border-bottom: 1px none #333; /* Линия снизу таблицы */"
//            "border-bottom: none; /* Линия снизу таблицы */"
//            "border-top: none; /* Линия сверху таблицы */"
           "}"
           "td { "
            "text-align: left; /* Выравнивание по лево */"
            "border-bottom: none;"
           "}"
           "td, th {"
            "padding: 1px; /* Поля в ячейках */"
           "}"
            "th {"
            "border-top: 1px dashed;"
            "}"
         "  </style>"

           "</head>\n"
           "<body bgcolor = #ffffff link=#5000A0>\n";


    // маркеры смены группы
    QString val01= ""; // год
    QString val02= ""; // поступление

    // счетчик для количества
    double countA=0;

    // титульный
    out << QString("<h2>ОТЧЕТ по статьям: %1 </h2>").arg(organization);
    out << QString("<h3>за период: с %1 по %2 </h3>").arg(dateBegin).arg(dateEnd);

    // данные
    out <<  "<table>\n";
    while (a_query.next()) {
        // печать категорий
        // год
        if (val01!=a_query.value(0).toString()) {
            // если поменялось, то пропечатываем
            out << "<tr>";
            out <<  QString("<th colspan=\"4\" align=left>%1 </th>").arg(a_query.value(0).toString());
            // запоминаем новое
            val01= a_query.value(0).toString();
            out << "</tr>\n";
        }
        // поступление
        if (val02!=a_query.value(1).toString()) {
            // вывод итога прошлой группы
            if(countA!=0) {
                out << "<tr>";
                out <<  QString("<td></td>");
                out <<  QString("<td></td>");
                out <<  QString("<td align=right><i>ИТОГО по статье: </i></td>");
                out <<  QString("<td align=right><i>%1</i></td>").arg(QString("%L1").arg(countA,-0,'f',2));
                countA=0;
             }

            // смена группы
            // запоминаем новое
            val02= a_query.value(1).toString();
            out << "</tr>\n";

            // если поменялось, то пропечатываем
            out << "<tr>";
            out <<  QString("<td></td>");
            out <<  QString("<th colspan=\"3\" align=left>%1 </th>").arg(a_query.value(1).toBool()?"Поступление":"Расход");
//            out <<  QString("<th colspan=\"3\" align=left>%1 </th>").arg((!a_query.value(1).toString().isEmpty())? a_query.value(1).toString():QString("&nbsp;"));
            // запоминаем новое
//            val02= a_query.value(1).toString();
            out << "</tr>\n";
        }


        out << "<tr>";
        // печать основных данных
        out <<  QString("<td></td>");
        out <<  QString("<td></td>");
        out <<  QString("<td  width=\"70%\">%1 </td>").arg((!a_query.value(2).toString().isEmpty())? a_query.value(2).toString():QString("&nbsp;"));
        out <<  QString("<td align=right>%1 </td>").arg((!a_query.value(3).toString().isEmpty())? QString("%L1").arg(a_query.value(3).toDouble(), -0, 'f', 2):QString("&nbsp;"));
//        out <<  QString("<td align=right>%1 </td>").arg((!a_query.value(3).toString().isEmpty())? QString::number(a_query.value(3).toDouble(),'f',2):QString("&nbsp;"));
        out <<  QString("<td align=right>%1 </td>").arg((!a_query.value(4).toString().isEmpty())? a_query.value(4).toString():QString("&nbsp;"));

        //добавляем текущее значение
        countA=countA+a_query.value(3).toDouble();

        out << "</tr>\n";
    }

        // пропечатываем последний итог
        out << "<tr>";
        out <<  QString("<td></td>");
        out <<  QString("<td></td>");
        out <<  QString("<td align=right><i>ИТОГО по статье: </i></td>");
        out <<  QString("<td align=right><i>%1</i></td>").arg(QString("%L1").arg(countA,-0,'f',2));

    out << "</table>\n";



    out << "</body>\n" << "</html>\n";

//    qDebug() << out.readAll();
    // печать
    QTextDocument document;
    document.setHtml(stringStream);

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
//    printer.setPageSize( setPaperSize(QPrinter::A4);
    printer.setOutputFileName("rep_bank.pdf");
    printer.setPageMargins(QMarginsF(15, 15, 15, 15));

    document.print(&printer);

    // откровем созданный отчет
    QDesktopServices::openUrl(QUrl(QUrl::fromLocalFile("rep_bank.pdf")));

}


void MainWindow::on_actionRepContracts_triggered()
{
    // главный отчет по контрактам полный

    // база открыта?
    if(!database.isOpen()){
          qDebug() << "База не открыта!";
          QMessageBox::critical(this,"Error","База не открыта!");
          return;
    }

    QSqlQuery a_query = QSqlQuery(database);

    //читаем настнойки из базы
    QString organization = "";
    QString dateBegin = "";
    QString dateEnd = "";
    bool rep_contract_found = false;


    if (!a_query.exec("SELECT organization, date_begin, date_end, rep_contract_found FROM options"))
            qDebug() << "Ошибка чтения настроек: " << a_query.lastError().text();
    else {
        a_query.first();
        organization = a_query.value(0).toString();
        dateBegin = a_query.value(1).toString();
        dateEnd = a_query.value(2).toString();
        rep_contract_found=a_query.value(3).toBool();
    }


    // фильтр дат
    QString flt="";
    if ((!dateBegin.isEmpty() && !dateEnd.isEmpty()) || rep_contract_found) {
        flt.append("WHERE ");

        if (!dateBegin.isEmpty() && !dateEnd.isEmpty())
            flt.append(QString("bank.payment_date >= '%1' AND bank.payment_date <= '%2'").arg(dateBegin).arg(dateEnd));
        if (rep_contract_found)
            flt.append(" AND contracts.found=true ");
    }


    // запрос
//    QString query = QString("SELECT  contracts.state_contract , articles.article, contracts.contract_number, contracts.contract_date, counterparties.counterparty, ROUND(SUM(sum),2), COUNT(DISTINCT contracts.contract_number)  FROM bank_decryption inner join contracts on bank_decryption.contract_id=contracts.id inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id inner join counterparties on bank_decryption.contract_id=counterparties.id %1 GROUP BY  contracts.state_contract , articles.article, contracts.contract_number, contracts.contract_date;").arg(flt);
    QString query = QString("SELECT  contracts.state_contract , articles.article, contracts.contract_number, contracts.contract_date, counterparties.counterparty, ROUND(SUM(sum),2), COUNT(DISTINCT contracts.contract_number)  FROM bank_decryption inner join contracts on bank_decryption.contract_id=contracts.id inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id inner join counterparties on contracts.counterparty_id =counterparties.id %1 GROUP BY  contracts.state_contract , articles.article, contracts.contract_number, contracts.contract_date;").arg(flt);
//    counterparties on contracts.counterparty_id =counterparties.id
    if (!a_query.exec(query)) {
         qDebug() << "Ошибка запроса отчета: " << a_query.lastError().text();
         return;
    }
//    qDebug() << flt;
//    qDebug() << query;

    // формирование и печать

    QString stringStream;
    QTextStream out(&stringStream);


    // формирование отчета
    out << "<html>\n" << "<head>\n" << "meta Content=\"Text/html;charsrt=Windows-1251\">\n" <<
           QString("<title>%1</title>\n").arg("Report") <<

           "<style>"
           "table {"
            "width: 100%; /* Ширина таблицы */"
//            "border-collapse: collapse; /* Убираем двойные линии */"
//            "border-bottom: 1px none #333; /* Линия снизу таблицы */"
//            "border-bottom: none; /* Линия снизу таблицы */"
//            "border-top: none; /* Линия сверху таблицы */"
           "}"
           "td { "
            "text-align: left; /* Выравнивание по лево */"
            "border-bottom: none;"
           "}"
           "td, th {"
            "padding: 1px; /* Поля в ячейках */"
           "}"
            "th {"
            "border-top: 1px dashed;"
            "}"
         "  </style>"

           "</head>\n"
           "<body bgcolor = #ffffff link=#5000A0>\n";


    // маркеры смены группы
    QString val01= ""; // Госконтракт
    QString val02= ""; // статья

    // счетчик для количества
    double sumA=0;
    int countA=0;

    // титульный
    out << QString("<h2>ОТЧЕТ по контрактам: %1 </h2>").arg(organization);
    out << QString("- за период: с %1 по %2 ").arg(dateBegin).arg(dateEnd);
    if (rep_contract_found)
        out << QString(", включены только найденные контракты.").arg(dateBegin).arg(dateEnd);


    // данные
    out <<  "<table>\n";
    while (a_query.next()) {
        // печать категорий
        // госконтракт
        if (val01!=a_query.value(0).toString()) {
            // если поменялось, то пропечатываем
            out << "<tr>";
            out <<  QString("<th colspan=\"6\" align=left>%1 </th>").arg(a_query.value(0).toBool()?"Госконтракт":"Договор");
            // запоминаем новое
            val01= a_query.value(0).toString();
            out << "</tr>\n";
        }
        // статья
        if (val02!=a_query.value(1).toString()) {
            // вывод итога прошлой группы
            if(countA!=0) {
                out << "<tr>";
                out <<  QString("<td></td>");
                out <<  QString("<td></td>");
                out <<  QString("<td></td>");
                out <<  QString("<td></td>");
                out <<  QString("<td align=right><b>ИТОГО по статье: </b></td>");
                out <<  QString("<td align=right><b>%1</b></td>").arg(QString("%L1").arg(sumA,-0,'f',2));
                out <<  QString("<td align=right><b>%1</b></td>").arg(countA);
                countA=0;
                sumA=0;

             }

            // смена группы
            // запоминаем новое
            val02= a_query.value(1).toString();
            out << "</tr>\n";

            // если поменялось, то пропечатываем
            out << "<tr>";
            out <<  QString("<td></td>");
            out <<  QString("<th colspan=\"6\" align=left>%1 </th>").arg(a_query.value(1).toString());
//            out <<  QString("<th colspan=\"3\" align=left>%1 </th>").arg((!a_query.value(1).toString().isEmpty())? a_query.value(1).toString():QString("&nbsp;"));
            // запоминаем новое
//            val02= a_query.value(1).toString();
            out << "</tr>\n";
        }


        out << "<tr>";
        // печать основных данных
        out <<  QString("<td></td>");
        out <<  QString("<td></td>");
        out <<  QString("<td>%1 </td>").arg((!a_query.value(2).toString().isEmpty())? a_query.value(2).toString():QString("&nbsp;"));
        out <<  QString("<td>%1 </td>").arg((!a_query.value(3).toString().isEmpty())? a_query.value(3).toString():QString("&nbsp;"));
//        out <<  QString("<td>%1 </td>").arg((!a_query.value(4).toString().isEmpty())? a_query.value(4).toString():QString("&nbsp;"));
        out <<  QString("<td  width=\"50%\"><small> %1 </small></td>").arg((!a_query.value(4).toString().isEmpty())? a_query.value(4).toString():QString("&nbsp;"));
        out <<  QString("<td align=right>%1 </td>").arg((!a_query.value(5).toString().isEmpty())? QString("%L1").arg(a_query.value(5).toDouble(), -0, 'f', 2):QString("&nbsp;"));
        out <<  QString("<td align=right>%1 </td>").arg((!a_query.value(6).toString().isEmpty())? a_query.value(6).toString():QString("&nbsp;"));

        //добавляем текущее значение
        countA=countA+a_query.value(6).toInt();
        sumA=sumA+a_query.value(5).toDouble();

        out << "</tr>\n";
    }

        // пропечатываем последний итог
        out << "<tr>";
        out <<  QString("<td></td>");
        out <<  QString("<td></td>");
        out <<  QString("<td></td>");
        out <<  QString("<td></td>");
        out <<  QString("<td align=right><b>ИТОГО по статье: </b></td>");
        out <<  QString("<td align=right><b>%1</b></td>").arg(QString("%L1").arg(sumA,-0,'f',2));
        out <<  QString("<td align=right><b>%1</b></td>").arg(countA);

    out << "</table>\n";



    out << "</body>\n" << "</html>\n";

//    qDebug() << out.readAll();
    // печать
    QTextDocument document;
    document.setHtml(stringStream);

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
//    printer.setPaperSize(QPrinter::A4);
    printer.setOutputFileName("rep_Contracts.pdf");
    printer.setPageMargins(QMarginsF(15, 15, 15, 15));

    document.print(&printer);

    // откровем созданный отчет
    QDesktopServices::openUrl(QUrl(QUrl::fromLocalFile("rep_Contracts.pdf")));
}



void MainWindow::on_actionRepContracsShort_triggered()
{
    // главный отчет по контрактам короткий по субкодам

    // база открыта?
    if(!database.isOpen()){
          qDebug() << "База не открыта!";
          QMessageBox::critical(this,"Error","База не открыта!");
          return;
    }

    QSqlQuery a_query = QSqlQuery(database);

    //читаем настнойки из базы
    QString organization = "";
    QString dateBegin = "";
    QString dateEnd = "";
    bool rep_contract_found = false;


    if (!a_query.exec("SELECT organization, date_begin, date_end, rep_contract_found FROM options"))
            qDebug() << "Ошибка чтения настроек: " << a_query.lastError().text();
    else {
        a_query.first();
        organization = a_query.value(0).toString();
        dateBegin = a_query.value(1).toString();
        dateEnd = a_query.value(2).toString();
        rep_contract_found=a_query.value(3).toBool();
    }


    // фильтр дат
    QString flt="";
    if ((!dateBegin.isEmpty() && !dateEnd.isEmpty()) || rep_contract_found) {
        flt.append("WHERE ");

        if (!dateBegin.isEmpty() && !dateEnd.isEmpty())
            flt.append(QString("bank.payment_date >= '%1' AND bank.payment_date <= '%2'").arg(dateBegin).arg(dateEnd));
        if (rep_contract_found)
            flt.append(" AND contracts.found=true ");
    }

    qDebug() << "фильтр: " << flt;

    // запрос
    QString query = QString("SELECT contracts.state_contract , articles.subcode, ROUND(SUM(sum),2), COUNT(DISTINCT contracts.contract_number) FROM bank_decryption inner join contracts on bank_decryption.contract_id=contracts.id inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id inner join counterparties on contracts.counterparty_id=counterparties.id %1 GROUP BY  contracts.state_contract , articles.subcode;").arg(flt);
    if (!a_query.exec(query)) {
         qDebug() << "Ошибка запроса отчета: " << a_query.lastError().text();
         return;
    }
//    qDebug() << flt;
//    qDebug() << query;

    // формирование и печать

    QString stringStream;
    QTextStream out(&stringStream);


    // формирование отчета
    out << "<html>\n" << "<head>\n" << "meta Content=\"Text/html;charsrt=Windows-1251\">\n" <<
           QString("<title>%1</title>\n").arg("Report") <<

           "<style>"
           "table {"
            "width: 100%; /* Ширина таблицы */"
//            "border-collapse: collapse; /* Убираем двойные линии */"
//            "border-bottom: 1px none #333; /* Линия снизу таблицы */"
//            "border-bottom: none; /* Линия снизу таблицы */"
//            "border-top: none; /* Линия сверху таблицы */"
           "}"
           "td { "
            "text-align: left; /* Выравнивание по лево */"
            "border-bottom: none;"
           "}"
           "td, th {"
            "padding: 1px; /* Поля в ячейках */"
           "}"
            "th {"
            "border-top: 1px dashed;"
            "}"
         "  </style>"

           "</head>\n"
           "<body bgcolor = #ffffff link=#5000A0>\n";


    // маркеры смены группы
    QString val01= ""; // Госконтракт

    // счетчик для количества
    double sumA=0;
    int countA=0;

    // титульный
    out << QString("<h2>ОТЧЕТ по контрактам короткий по субкодам для акта: %1 </h2>").arg(organization);
    out << QString("- за период: с %1 по %2 ").arg(dateBegin).arg(dateEnd);
    if (rep_contract_found)
        out << QString(", включены только найденные контракты.").arg(dateBegin).arg(dateEnd);


    // данные
    out <<  "<table>\n";
    while (a_query.next()) {
        // печать категорий
        // госконтракт
        // поступление
        if (val01!=a_query.value(0).toString()) {
            // вывод итога прошлой группы
            if(countA!=0) {
                out << "<tr>";
                out <<  QString("<td></td>");
                out <<  QString("<td align=right><b>ИТОГО по категории: </b></td>");
                out <<  QString("<td align=right><b>%1</b></td>").arg(QString("%L1").arg(sumA,-0,'f',2));
                out <<  QString("<td align=right><b>%1</b></td>").arg(countA);
                countA=0;
                sumA=0;
             }

            // смена группы
            // запоминаем новое
            val01= a_query.value(0).toString();
            out << "</tr>\n";

            // если поменялось, то пропечатываем
            out << "<tr>";
            out <<  QString("<th colspan=\"4\" align=left>%1 </th>").arg(a_query.value(0).toBool()?"Госконтракт":"Договор");
            out << "</tr>\n";
        }


        out << "<tr>";
        // печать основных данных
        out <<  QString("<td></td>");
        out <<  QString("<td  width=\"70%\"><small> %1 </small></td>").arg((!a_query.value(1).toString().isEmpty())? a_query.value(1).toString():QString("&nbsp;"));
        out <<  QString("<td align=right>%1 </td>").arg((!a_query.value(2).toString().isEmpty())? QString("%L1").arg(a_query.value(2).toDouble(), -0, 'f', 2):QString("&nbsp;"));
        out <<  QString("<td align=right>%1 </td>").arg((!a_query.value(3).toString().isEmpty())? a_query.value(3).toString():QString("&nbsp;"));

        //добавляем текущее значение
        countA=countA+a_query.value(3).toInt();
        sumA=sumA+a_query.value(2).toDouble();

        out << "</tr>\n";
    }

        // пропечатываем последний итог
        out << "<tr>";
        out <<  QString("<td></td>");
        out <<  QString("<td align=right><b>ИТОГО по категории: </b></td>");
        out <<  QString("<td align=right><b>%1</b></td>").arg(QString("%L1").arg(sumA,-0,'f',2));
        out <<  QString("<td align=right><b>%1</b></td>").arg(countA);

    out << "</table>\n";



    out << "</body>\n" << "</html>\n";

//    qDebug() << out.readAll();
    // печать
    QTextDocument document;
    document.setHtml(stringStream);

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
//    printer.setPaperSize(QPrinter::A4);
    printer.setOutputFileName("rep_Contracts_s.pdf");
    printer.setPageMargins(QMarginsF(15, 15, 15, 15));

    document.print(&printer);

    // откровем созданный отчет
    QDesktopServices::openUrl(QUrl(QUrl::fromLocalFile("rep_Contracts_s.pdf")));
}


void MainWindow::on_actionRepContractsIsNote_triggered()
{
    // отчет по контрактам с примечанием

    // база открыта?
    if(!database.isOpen()){
          qDebug() << "База не открыта!";
          QMessageBox::critical(this,"Error","База не открыта!");
          return;
    }

    QSqlQuery a_query = QSqlQuery(database);

    //читаем настнойки из базы
    QString organization = "";

    if (!a_query.exec("SELECT organization FROM options"))
            qDebug() << "Ошибка чтения настроек: " << a_query.lastError().text();
    else {
        a_query.first();
        organization = a_query.value(0).toString();
    }


    // запрос
    QString query = QString("SELECT counterparties.counterparty, contracts.contract_number, contracts.contract_date, contracts.state_contract, SUM(sum), COUNT(DISTINCT bank.id), articles.article, contracts.note, contracts.found  FROM bank_decryption inner join contracts on bank_decryption.contract_id=contracts.id inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id inner join counterparties on contracts.counterparty_id=counterparties.id WHERE NOT contracts.note='' GROUP BY counterparties.counterparty, contracts.contract_number, contracts.contract_date, articles.article");
    if (!a_query.exec(query)) {
         qDebug() << "Ошибка запроса отчета: " << a_query.lastError().text();
         return;
    }
//    qDebug() << flt;
//    qDebug() << query;

    // формирование и печать

    QString stringStream;
    QTextStream out(&stringStream);


    // формирование отчета
    out << "<html>\n" << "<head>\n" << "meta Content=\"Text/html;charsrt=Windows-1251\">\n" <<
           QString("<title>%1</title>\n").arg("Report") <<

           "<style>"
           "table {"
            "width: 100%; /* Ширина таблицы */"
//            "border-collapse: collapse; /* Убираем двойные линии */"
//            "border-bottom: 1px none #333; /* Линия снизу таблицы */"
//            "border-bottom: none; /* Линия снизу таблицы */"
//            "border-top: none; /* Линия сверху таблицы */"
           "}"
           "td { "
            "text-align: left; /* Выравнивание по лево */"
            "border-bottom: none;"
           "}"
           "td, th {"
            "padding: 1px; /* Поля в ячейках */"
           "}"
            "th {"
            "border-top: 1px dashed;"
            "}"
         "  </style>"

           "</head>\n"
           "<body bgcolor = #ffffff link=#5000A0>\n";


    // маркеры смены группы
    QString val01= ""; // Контрагент

    // счетчик для количества
    double sumA=0;
    int countA=0;

    // титульный
    out << QString("<h2>Список контрактов с примечанием: %1 </h2>").arg(organization);

    // данные
    out <<  "<table>\n";
    while (a_query.next()) {
        // печать категорий
        // контрагент
        if (val01!=a_query.value(0).toString()) {
            // вывод итога прошлой группы
//            if(countA!=0) {
//                out << "<tr>";
//                out <<  QString("<td></td>");
//                out <<  QString("<td></td>");
//                out <<  QString("<td></td>");
//                out <<  QString("<td align=right><b>ИТОГО: </b></td>");
//                out <<  QString("<td align=right><b>%1</b></td>").arg(QString("%L1").arg(sumA,-0,'f',2));
//                out <<  QString("<td align=right><b>%1</b></td>").arg(countA);
//                countA=0;
//                sumA=0;
//             }

            // смена группы
            // запоминаем новое
            val01= a_query.value(0).toString();
            out << "</tr>\n";

            // если поменялось, то пропечатываем
            out << "<tr>";
            out <<  QString("<th colspan=\"7\" align=left>%1 </th>").arg(a_query.value(0).toString());
            out << "</tr>\n";
        }


        out << "<tr>";
        // печать основных данных
        out <<  QString("<td></td>");
//        out <<  QString("<td  width=\"90%\"><small> %1 </small></td>").arg((!a_query.value(1).toString().isEmpty())? a_query.value(1).toString():QString("&nbsp;"));
        out <<  QString("<td> %1 </td>").arg((!a_query.value(1).toString().isEmpty())? a_query.value(1).toString():QString("&nbsp;"));
        out <<  QString("<td> %1 </td>").arg((!a_query.value(2).toString().isEmpty())? a_query.value(2).toString():QString("&nbsp;"));
        out <<  QString("<td> %1 </td>").arg(a_query.value(3).toBool()?"Гoc":"Дor");
        out <<  QString("<td align=right>%1 </td>").arg((!a_query.value(4).toString().isEmpty())? QString("%L1").arg(a_query.value(4).toDouble(), -0, 'f', 2):QString("&nbsp;"));
        out <<  QString("<td align=right>%1 </td>").arg((!a_query.value(5).toString().isEmpty())? a_query.value(5).toString():QString("&nbsp;"));
        out <<  QString("<td><small> %1 </small></td>").arg((!a_query.value(6).toString().isEmpty())? a_query.value(6).toString().left(25):QString("&nbsp;")); // статья
        out <<  QString("<td><small> %1 </small></td>").arg((!a_query.value(8).toString().isEmpty())? a_query.value(8).toString():QString("&nbsp;")); // найден
        out <<  QString("<td><small> %1 </small></td>").arg((!a_query.value(7).toString().isEmpty())? a_query.value(7).toString():QString("&nbsp;")); // примечание

        //добавляем текущее значение
        countA=countA+a_query.value(5).toInt();
        sumA=sumA+a_query.value(4).toDouble();

        out << "</tr>\n";
    }

        // пропечатываем последний итог
//        out << "<tr>";
//        out <<  QString("<td></td>");
//        out <<  QString("<td></td>");
//        out <<  QString("<td></td>");
//        out <<  QString("<td align=right><b>ИТОГО: </b></td>");
//        out <<  QString("<td align=right><b>%1</b></td>").arg(QString("%L1").arg(sumA,-0,'f',2));
//        out <<  QString("<td align=right><b>%1</b></td>").arg(countA);

    out << "</table>\n";



    out << "</body>\n" << "</html>\n";

//    qDebug() << out.readAll();
    // печать
    QTextDocument document;
    document.setHtml(stringStream);

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
//    printer.setPaperSize(QPrinter::A4);
//    printer.setOrientation(QPrinter::Landscape);
    printer.setOutputFileName("rep_Contracts_n.pdf");
    printer.setPageMargins(QMarginsF(10, 10, 10, 10));

    document.print(&printer);

    // откровем созданный отчет
    QDesktopServices::openUrl(QUrl(QUrl::fromLocalFile("rep_Contracts_n.pdf")));

}


void MainWindow::on_actionRepContractsFor_triggered()
{
    // главный отчет по контрактам короткий по субкодам без деления на договора контрагты

    // база открыта?
    if(!database.isOpen()){
          qDebug() << "База не открыта!";
          QMessageBox::critical(this,"Error","База не открыта!");
          return;
    }

    QSqlQuery a_query = QSqlQuery(database);

    //читаем настнойки из базы
    QString organization = "";
    QString dateBegin = "";
    QString dateEnd = "";
    bool rep_contract_found = false;


    if (!a_query.exec("SELECT organization, date_begin, date_end, rep_contract_found FROM options"))
            qDebug() << "Ошибка чтения настроек: " << a_query.lastError().text();
    else {
        a_query.first();
        organization = a_query.value(0).toString();
        dateBegin = a_query.value(1).toString();
        dateEnd = a_query.value(2).toString();
        rep_contract_found=a_query.value(3).toBool();
    }


    // фильтр дат
    QString flt="";
    if ((!dateBegin.isEmpty() && !dateEnd.isEmpty()) || rep_contract_found) {
        flt.append("WHERE ");

        if (!dateBegin.isEmpty() && !dateEnd.isEmpty())
            flt.append(QString("bank.payment_date >= '%1' AND bank.payment_date <= '%2'").arg(dateBegin).arg(dateEnd));
        if (rep_contract_found)
            flt.append(" AND contracts.found=true ");
    }

    qDebug() << "фильтр: " << flt;

    // запрос
    QString query = QString("SELECT articles.subcode, ROUND(SUM(sum),2), COUNT(DISTINCT contracts.contract_number) FROM bank_decryption inner join contracts on bank_decryption.contract_id=contracts.id inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id inner join counterparties on contracts.counterparty_id=counterparties.id %1 GROUP BY articles.subcode;").arg(flt);
    if (!a_query.exec(query)) {
         qDebug() << "Ошибка запроса отчета: " << a_query.lastError().text();
         return;
    }
//    qDebug() << flt;
//    qDebug() << query;

    // формирование и печать

    QString stringStream;
    QTextStream out(&stringStream);


    // формирование отчета
    out << "<html>\n" << "<head>\n" << "meta Content=\"Text/html;charsrt=Windows-1251\">\n" <<
           QString("<title>%1</title>\n").arg("Report") <<

           "<style>"
           "table {"
            "width: 100%; /* Ширина таблицы */"
//            "border-collapse: collapse; /* Убираем двойные линии */"
//            "border-bottom: 1px none #333; /* Линия снизу таблицы */"
//            "border-bottom: none; /* Линия снизу таблицы */"
//            "border-top: none; /* Линия сверху таблицы */"
           "}"
           "td { "
            "text-align: left; /* Выравнивание по лево */"
            "border-bottom: none;"
           "}"
           "td, th {"
            "padding: 1px; /* Поля в ячейках */"
           "}"
            "th {"
            "border-top: 1px dashed;"
            "}"
         "  </style>"

           "</head>\n"
           "<body bgcolor = #ffffff link=#5000A0>\n";


    // маркеры смены группы
    //QString val01= ""; // Госконтракт

    // счетчик для количества
    double sumA=0;
    int countA=0;

    // титульный
    out << QString("<h2>ОТЧЕТ по контрактам по субкодам для акта без дазбивки: %1 </h2>").arg(organization);
    out << QString("- за период: с %1 по %2 ").arg(dateBegin).arg(dateEnd);
    if (rep_contract_found)
        out << QString(", включены только найденные контракты.").arg(dateBegin).arg(dateEnd);


    // данные
    out <<  "<table>\n";
    while (a_query.next()) {
        // печать категорий
        // госконтракт


        out << "<tr>";
        // печать основных данных
        out <<  QString("<td></td>");
        out <<  QString("<td  width=\"70%\"><small> %1 </small></td>").arg((!a_query.value(0).toString().isEmpty())? a_query.value(0).toString():QString("&nbsp;"));
        out <<  QString("<td align=right>%1 </td>").arg((!a_query.value(1).toString().isEmpty())? QString("%L1").arg(a_query.value(1).toDouble(), -0, 'f', 2):QString("&nbsp;"));
        out <<  QString("<td align=right>%1 </td>").arg((!a_query.value(2).toString().isEmpty())? a_query.value(2).toString():QString("&nbsp;"));

        //добавляем текущее значение
        countA=countA+a_query.value(2).toInt();
        sumA=sumA+a_query.value(1).toDouble();

        out << "</tr>\n";
    }

        // пропечатываем последний итог
        out << "<tr>";
        out <<  QString("<td></td>");
        out <<  QString("<td align=right><b>ИТОГО по категории: </b></td>");
        out <<  QString("<td align=right><b>%1</b></td>").arg(QString("%L1").arg(sumA,-0,'f',2));
        out <<  QString("<td align=right><b>%1</b></td>").arg(countA);

    out << "</table>\n";



    out << "</body>\n" << "</html>\n";

//    qDebug() << out.readAll();
    // печать
    QTextDocument document;
    document.setHtml(stringStream);

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
//    printer.setPaperSize(QPrinter::A4);
    printer.setOutputFileName("rep_Contracts_s2.pdf");
    printer.setPageMargins(QMarginsF(15, 15, 15, 15));

    document.print(&printer);

    // откровем созданный отчет
    QDesktopServices::openUrl(QUrl(QUrl::fromLocalFile("rep_Contracts_s2.pdf")));

}

