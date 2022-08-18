#include "formoptions.h"
#include "ui_formoptions.h"

#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMessageBox>
#include <QSqlError>
#include <QDebug>
#include <QStatusTipEvent>
#include <QFile>
#include <QFileDialog>
#include <QProgressDialog>


FormOptions::FormOptions(QSqlDatabase db,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormOptions)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    base=db;

    //читаем настнойки из базы
     if(base.isOpen()) {
        QSqlQuery a_query = QSqlQuery(base);
        if (!a_query.exec("SELECT organization, date_begin, date_end, rep_contract_found, regexp_c FROM options"))
                qDebug() << "Ошибка чтения настроек: " << a_query.lastError().text();
        else {
            a_query.first();
            ui->lineEdit_organization->setText(a_query.value(0).toString());
            ui->dateEdit_begin->setDate(QDate::fromString(a_query.value(1).toString(),"yyyy-MM-dd"));
            ui->dateEdit_end->setDate(QDate::fromString(a_query.value(2).toString(),"yyyy-MM-dd"));
            ui->checkBox_rep_contract_found->setChecked(a_query.value(3).toBool());
            ui->lineEdit_regex->setText(a_query.value(4).toString());
        }
    }


}

FormOptions::~FormOptions()
{
    delete ui;
}

void FormOptions::on_pushButton_close_clicked()
{

    // сохранение в базе настроек наименования и дат
     if(base.isOpen()) {
        QSqlQuery a_query = QSqlQuery(base);
        QString ss=QString("UPDATE options SET organization = '%1', date_begin = '%2', date_end = '%3', rep_contract_found = '%4', regexp_c = '%5';").arg(ui->lineEdit_organization->text()).arg(ui->dateEdit_begin->date().toString("yyyy-MM-dd")).arg(ui->dateEdit_end->date().toString("yyyy-MM-dd")).arg(ui->checkBox_rep_contract_found->isChecked()?"true":"false").arg(ui->lineEdit_regex->text());
        if (!a_query.exec(ss))
                qDebug() << "Ошибка записи настроек: " << a_query.lastError().text();
    }

    close();
}

void FormOptions::on_pushButton_clearBase_clicked()
{
    // очистка базы

    // подтверждение удаление
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Внимание!"),
                                                 tr("Уверены в удалении данных?")))  return;

    if(!base.isOpen()) {
        QMessageBox::critical(this,"Error","База не открыта!");
        return;
    }

    QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Очистка базы подождите ...")));

    QSqlQuery a_query = QSqlQuery(base);

    //очистка расшифровок
    if(ui->checkBox_bank_decryption->isChecked()) {
        // запрос на очистку расшифровок
        if (!a_query.exec("DELETE FROM bank_decryption;"))
            qDebug() << "таблица расшифровок: " << a_query.lastError().text();

        // сбрасываем счетчик расшифровок id
        if (!a_query.exec("UPDATE sqlite_sequence set seq=0 WHERE Name='bank_decryption'"))
            qDebug() << "счетчик расшифровок: " << a_query.lastError().text();
    }

    //очистка банка
    if(ui->checkBox_bank->isChecked()) {
        // запрос на очистку банка
        if (!a_query.exec("DELETE FROM bank;"))
            qDebug() << "таблица банка: " << a_query.lastError().text();

        // сбрасываем счетчик расшифровок id
        if (!a_query.exec("UPDATE sqlite_sequence set seq=0 WHERE Name='bank'"))
            qDebug() << "счетчик банка: " << a_query.lastError().text();

    }

    //очистка контрагентов // невозможно без очистки банка ввиду трудоемкости восстановления данных
    if(ui->checkBox_counterparties->isChecked()) {
        // запрос на очистку контрагентов
        if (!a_query.exec("DELETE FROM counterparties;"))
            qDebug() << "таблица контрагентов: " << a_query.lastError().text();

        // сбрасываем счетчик контрагентов id
        if (!a_query.exec("UPDATE sqlite_sequence set seq=0 WHERE Name='counterparties'"))
            qDebug() << "счетчик контрагентов: " << a_query.lastError().text();

    }

    //очистка договоров
    if(ui->checkBox_contracts->isChecked()) {
        // запрос на очистку договоров в расшифровках
        if (!a_query.exec("UPDATE bank_decryption SET contract_id = NULL"))
            qDebug() << "таблица расшифровок - сброс договоров: " << a_query.lastError().text();

        // запрос на очистку договоров
        if (!a_query.exec("DELETE FROM contracts;"))
            qDebug() << "таблица договоров: " << a_query.lastError().text();

        // сбрасываем счетчик договоров id
        if (!a_query.exec("UPDATE sqlite_sequence set seq=0 WHERE Name='contracts'"))
            qDebug() << "счетчик договоров: " << a_query.lastError().text();



    }

    //
    QMessageBox::information(this,"Info","Операция завершена.");
    QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Очистка базы завершена.")));
}

void FormOptions::on_checkBox_counterparties_stateChanged(int arg1)
{
    //принудительно включаем очистку
    if (ui->checkBox_counterparties->isChecked())
        ui->checkBox_bank->setChecked(true);
}

void FormOptions::on_checkBox_bank_stateChanged(int arg1)
{
    //принудительно включаем очистку
    if (ui->checkBox_bank->isChecked())
        ui->checkBox_bank_decryption->setChecked(true);

}

void FormOptions::on_pushButton_ExportArticles_clicked()
{
    // экспорт справочника статей
    qDebug() << "Export articles";

    // запросить новое имя
    QString  exportName = QFileDialog::getSaveFileName(this,tr("Экспортировать как"),".//exportarticles.tsv",tr("File TSV (*.tsv)"));
   // databaseName = QFileDialog::getOpenFileName(this,tr("Open base"),"./",QString("Data base Files (*%1);; All file (*)").arg(FILE_EXT));

   // если файл присутствует

    //если пустое
    if(exportName.isEmpty()){
        qDebug() << "Отменено!";
        return;
    }

    //если нет расширения добавляем
    if (exportName.indexOf(".tsv")==-1)
        exportName.append(".tsv");

    //проверяем на наличие файл
//    if(QFile(exportName).exists()){
//        qDebug() << "Такой файл уже есть!";
//        QMessageBox::information(this,"Error","Выбранный файл уже существует. Задайте другое имя!");
//        return;
//    }


    QSqlQuery query = QSqlQuery(base);

    if (!query.exec("SELECT article, code, subcode, f14, note FROM articles;")) {
          qDebug() << "Запрос статей: " << query.lastError().text();
          return;
    }


    QFile file(exportName);
      if(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
      {
          file.resize(0); // чистим старый
          QTextStream stream(&file);

          QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Идет экспорт статей подождите ...")));

            QString ss = "";
          while(query.next()) {
              ss.append(query.record().value(0).toString().remove('\n'));
              ss.append('\t');
              ss.append(query.record().value(1).toString().remove('\n'));
              ss.append('\t');
              ss.append(query.record().value(2).toString().remove('\n'));
              ss.append('\t');
              ss.append(query.record().value(3).toString().remove('\n'));
              ss.append('\t');
              ss.append(query.record().value(4).toString().replace('\n','\r')); // надо прижумать как экспортировать и импортировать текст с форматированием
//              ss.append(query.record().value(4).toString());
              ss.append('\n');
//              qDebug() << query.record().value(0).toString();
//              qDebug() << query.record().value(1).toString();
//              qDebug() << query.record().value(2).toString();
//              qDebug() << "------------------------------";
          }

//          qDebug() << ss;
          stream << ss;

          QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Экспорт статей окончен.")));

          file.close();
          qDebug() << "Writing finished";
      }
      else {
            QMessageBox::critical(this,"ERROR","Ошибка записи в файл!");
            return;
      }


}

void FormOptions::on_pushButton_ImportArticles_clicked()
{
    // импорт справочника статей

    // подтверждение импорта
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Импорт справочника статей!"),
                                                 tr("Рекомендован импорт в пустую базу!\nПроизойдет добавление в справочник статей!\nУверены в операции импорта?")))  return;

    // возможно проверить на отсутствие расшифровок и почистить старые статьи и после добавить новые
    // или вообще дать импортировать статьи только при создании новой базы, что сомнительно


    if(!base.isOpen()){
      qDebug() << "База не открыта!";
        QMessageBox::critical(this,"Error","База не открыта!");
        return;
    }

    //
    QString fileName;
    QString sep = "\t";
       fileName = QFileDialog::getOpenFileName(this,QString::fromUtf8("Открыть файл"), QDir::currentPath(),"Файлы TSV (*.tsv *.csv);;Все файлы (*)");
       QFile file(fileName);
       if(file.open (QIODevice::ReadOnly)){

           QTextStream ts (&file);

           // Cтрока в которую будем формировать запросы
           QString req = "INSERT INTO articles (article, code, subcode, f14, note) VALUES ";
           // Цикл до конца файла
           while(!ts.atEnd()){
               // Обрезаем строку до разделителя
               QStringList line = ts.readLine().split(sep);
               if (line.count()<5) {
                   qDebug() << tr("число колонок меньше трех! Импорт невозможен! ") << line;
               }
               //qDebug() << "import: " << line;
               // предпологается жеткая структура
               req.append("(\""+line.at(0)+"\",");
               req.append("\""+line.at(1)+"\",");
               req.append("\""+line.at(2)+"\",");
               req.append("\""+line.at(3)+"\",");
               req.append("\""+line.at(4)+"\"),");
            }
           file.close ();


          req.chop(1);
          req.append(";");

          qDebug()<<req;

           // добавляем в базу
           QSqlQuery query(base);
           if(!query.exec(req))
           {
               qDebug() << "ERROR Insert " << query.lastError().text();
               return;
           }

       }

}
void FormOptions::on_pushButton_regexp_def_clicked()
{
//    ui->lineEdit_regex->setText("(кон|конт|контр|контракт|дог|догов|договор)(.|)\\s{0,}(N|)\\s{0,}(\\S{1,})\\s{0,}от\\s{0,}((0[1-9]|[12][0-9]|3[01])[-\\.](0[1-9]|1[012])[-\\.]((19|20)(\\d{2})|\\d{2}))\\D");
 //   ui->lineEdit_regex->setText("(кон|конт|контр|контракт|дог|догов|договор)(.|)\s{0,}(N|)\s{0,}(\S{1,})(\s{0,}от\s{0,}|)\s{0,}((0[1-9]|[12][0-9]|3[01])[-\.](0[1-9]|1[012])[-\.]((19|20)(\d{2})|\d{2}))\D");
// дата (?<=\D|^)(?<day>[0-3][1-9])(?<sep>[^\w\s])(?<month>1[0-2]|0[1-9])\k<sep>(?<year>\d{2}|\d{4})(?=\D|$)
     ui -> lineEdit_regex -> setText ( "(?<name>К-т|контракту|контракт|контр|конт|кон|договору|договор|догов|дог)([.]|)\\s{0,}([N№]|)\\s{0,}(?<number>\\S{1,})\\s{0,}(от|)\\s{0,}(?<=\\D|^)(?<day>[0-2][0-9]|[3][0-1])(?<sep>[^\\w\\s])(?<month>1[0-2]|0[1-9])\\k<sep>(?<year>\\d{4}|\\d{2})(?=\\D|$)" );

}


void FormOptions::on_pushButton_add_contracts_clicked()
{
    // автопоиск и простановка контрактов

    // подтверждение
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Автопростановка контрактов!"),
                                                 tr("Запустить автопростановку в случае возможности найти номер договора\nв присутствующие расшифровки с пустым полем контракта?")))  return;
    // база открыта
    if(!base.isOpen()){
      qDebug() << "База не открыта!";
        QMessageBox::critical(this,"Error","База не открыта!");
        return;
    }


    //для всех записей банка с расхифровками


    QSqlQuery query_cont(base);
    QSqlQuery query_bank(base);
    QSqlQuery query(base);

    QCoreApplication::postEvent(this, new QStatusTipEvent("Подождите идет автозаполнение контрактов по данным назначения платежа ..."));
    ui->plainTextEdit_contracts_log->appendPlainText(QString("идет автозаполнение контрактов"));


    // формируем список для банка с расшифровками
    if(!query_bank.exec("SELECT bank.id, bank.counterparty_id, bank.decryption_of_payment FROM bank WHERE bank.id IN (SELECT bank_id FROM bank_decryption)")) {
       qDebug() << "ERROR SELECT bank: " << query_bank.lastError().text();
       return;
    }

    int count=0; // счетчик

    //создание регулярного выражения с заданным шаблоном
    QRegularExpression rx(ui->lineEdit_regex->text());
    rx.setPatternOptions(QRegularExpression::CaseInsensitiveOption); // не чувствительность к регистру
    //проверка на корректность
    if (!rx.isValid()){
        qDebug() << "Error! Regex is not valid.";
        return;
    }

    // прогресс выполнения
    QProgressDialog progress(tr("Простановка контрактов..."), tr("Отмена"), 0,1000,this); // передалать макс значение првильно
    progress.setWindowModality(Qt::WindowModal);

    // Cтрока в которую будем формировать запросы

    while(query_bank.next()) {
        QString sNum=""; // найденный номер контракта
        QString sDate=""; // найденная дата контракта
        QString sIDcont=""; // найденный контракт
        bool state = false;

        // определяем номер дату контракта

        // если что то найдено
        QRegularExpressionMatch match = rx.match(query_bank.value(2).toString());
        if(match.hasMatch() ) {
            // первое вхождение
            sNum = match . captured ( 4 );
            if ( match . captured ( 9 ). length ()== 2 ) {
                            sDate = QDate :: fromString ( QString ( "%1.%2.20%3" ). arg ( match . captured ( 6 )). arg ( match . captured ( 8 )). arg ( match . captured ( 9 )), "dd.MM.yyyy" ). toString ( "yyyy-MM-dd" ); //приведение даты к длинному формату
              }
              else
                            sDate = QDate :: fromString ( QString ( "%1.%2.%3" ). arg ( match . captured ( 6 )). arg ( match . captured ( 8 )). arg ( match . captured ( 9 )), "dd.MM.yyyy" ). toString ( "yyyy-MM-dd" );
            //                 sDate = QDate::fromString(match.captured(5),"dd.MM.yyyy").toString("yyyy-MM-dd");

            // сомнительное определение - часто просто пишут контракт думаю не нужно полагаться на это
            //if(match.captured(1).left(1)=="к")
            //    state = true;

            // если номер и дата не пустые!
            if(!sNum.isEmpty() && !sDate.isEmpty()) {

                // ищем контракт в базе, если его нет добавляем

                // ищем только по номеру и дате - не чувствительно к контрагенту так как быает много дубликотов контрагентов - пробуем так!
                QString ff= QString("SELECT contracts.id FROM contracts WHERE (contracts.contract_number = '%1' AND contracts.contract_date = '%2')").arg(sNum).arg(sDate);
            //    qDebug() << ff;
                if(!query_cont.exec(ff)) {
                   qDebug() << "ERROR SELECT contracts: " << query_cont.lastError().text();
                   return;
                }
                //если не найден добавляем
                if(!query_cont.first()) {
                    QString ss= QString("INSERT INTO contracts (contract_number, contract_date,counterparty_id, state_contract) VALUES ('%1', '%2','%3','%4')").arg(sNum).arg(sDate).arg(query_bank.value(1).toString()).arg(state);
                    //qDebug() << ss;
                    if(!query.exec(ss)) {
                       qDebug() << "ERROR INSERT kontract: " << query.lastError().text();
                       //return;
                    }

                    ui->plainTextEdit_contracts_log->appendPlainText(QString("Добавлен № %1 от %2").arg(sNum).arg(sDate));

                    // ищем повторно
                    ss= QString("SELECT contracts.id FROM contracts WHERE (contracts.contract_number = '%1' AND contracts.contract_date = '%2')").arg(sNum).arg(sDate);
                    if(!query.exec(ss)) {
                       qDebug() << "ERROR SELECT contracts: " << query.lastError().text();
                       return;
                    }

                    query.first();
                    sIDcont=query.value(0).toString();

                }
                else sIDcont=query_cont.value(0).toString();
                //
                // если не пустой добавляем номер контракта к записи
                if (!sIDcont.isEmpty()) {

                    QString ss=QString("UPDATE bank_decryption SET contract_id = '%1' WHERE bank_id = '%2' AND contract_id IS NULL;").arg(sIDcont).arg(query_bank.value(0).toString());
                    if(!query.exec(ss)) {
                       qDebug() << "ERROR UPDATE bank_decryption: " << query.lastError().text();
                       // return;
                    }

                }
            }


        }

        //следующая пп
        count++;
        ui->lineEdit_log->setText(QString::number(count));

        // дергаем интерфейс, что бы не зависал
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        // роверяем на прекращение операции
        progress.setValue(count);
        if (progress.wasCanceled())
            break;

    }


       QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Обработано %1 записей.").arg(count)));
     progress.setValue(1000); // передалать првильно  // добавить такое же в операцию импорта!!
     //
}

