#include "formimport.h"
#include "ui_formimport.h"

#include <QtWidgets>
#include <QFileDialog>
#include <QFile>
#include <QtDebug>
#include <QMessageBox>
#include <QSqlError>


FormImport::FormImport(QSqlDatabase db,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormImport)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    base=db;
    importName="";

}

FormImport::~FormImport()
{
    delete ui;
}


void FormImport::on_pushButton_getFile_clicked()
{
    // импорт данных из CSV с разделителям табуляцией

    // сброс состояний
    ui->tableWidget->clear();
    ui->plainTextEdit_rep->clear();
    ui->checkBox_receipt->setChecked(false);
    ui->checkBox_recovery->setChecked(false);
    ui->lineEdit_note->setText("");

    // откты файл импорта
    // выбор файла
    importName = QFileDialog::getOpenFileName(this,QString("Открыть файл"),QDir::currentPath(),tr("Типы файлов (*.tsv;*.csv;*.txt;);;Все файлы (*.*)"));
    //importName=QString("d:/Qt/Project/base_a/import.txt"); // временно пропускаем выбор
    ui->lineEdit_file->setText(importName);
    qDebug() << "Импорт из: " << importName;
    // открыть
    if(!importName.isEmpty()) {
        //читаем первую строку с именами полей и заполняем справочник
        QString sep = "\t";
       QFile file(importName);
       if(file.open (QIODevice::ReadOnly)){
           QTextStream ts (&file);
           // Обрезаем строку до разделителя
           QStringList line = ts.readLine().split(sep);
//           qDebug() << "import: " << line;



           ui->tableWidget->setColumnCount(line.count());
           ui->tableWidget->insertRow(0);


           //добавляем управление
           for (int i = 0; i < line.size(); ++i) {
              // добавляем шапку из названий полей импорта
               ui->tableWidget->setHorizontalHeaderItem(i,new QTableWidgetItem(line.at(i)));
               // устанавливаем значения для импорта
               // настраиваем список полей для совмещения
               // настраиваем комбобокс
               QComboBox *combo = new QComboBox(this);
               combo->addItem("");
               combo->addItem("payment_number");
               combo->addItem("payment_date");
               combo->addItem("counterparty_id");
               combo->addItem("decryption_of_payment");
               combo->addItem("amount_of_payment");
               combo->addItem("article");
               combo->addItem("note");
               combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
               ui->tableWidget->setCellWidget(0, i, combo);

//           ui->tableWidget->setItem(0,i,new QTableWidgetItem(line.at(i)));
            }

           ui->tableWidget->insertRow(1); //вставляем пустышку
           //добавляем пару строк примера
           for(int c=2; c<4; ++c) {
               // читаем строку с данными
               line = ts.readLine().split(sep);
               ui->tableWidget->insertRow(c);
               for (int i = 0; i < line.size(); ++i)
                   ui->tableWidget->setItem(c, i, new QTableWidgetItem(line.at(i)));
           }

       }

        file.close ();

    }

}

void FormImport::on_pushButton_close_clicked()
{
    close();
}


void FormImport::on_pushButton_ImportZ_clicked()
{
    //импорт кучей
    // надо реализовать составное примечание  из поля и ручного - хз как

    // подтверждение импорта
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Внимание!"),
                                                 tr("Уверены в импорте данных?")))  return;


    if(!base.isOpen()){
      qDebug() << "база не открыта!";
      QMessageBox::critical(this,"Error","База не открыта!");
      return;
    }

    QSqlQuery query(base);


    QString sep = "\t";
    QString tabl = "bank";

    bool manual_note=true;

       QFile file(importName);
       if(file.open (QIODevice::ReadOnly)){

           QTextStream ts (&file);
           QStringList line = ts.readLine().split(sep); //читаем первую строку с заголовками и выбрасываем
//           qDebug() << "import: " << line;


           ui->plainTextEdit_rep->appendPlainText(QString("Подождите идет импорт из : %1").arg(importName));



           // Cтрока в которую будем формировать запросы
           QString req = "INSERT INTO ";
           req.append(tabl);
           req.append(" (");
           // список полей ставки
           // для всех значений из таблицы настроек
           QComboBox *box;
           //заголовок
           int count_t=0;
           for (int i=0;i < ui->tableWidget->columnCount(); ++i) {
               box = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(0,i));
               //qDebug() << "значение ключа:" << box->currentText() << "ключ:" << box->currentIndex();
               if (!box->currentText().isEmpty()) { // если не пустое
                   req.append(box->currentText());
                   req.append(",");
                   count_t++;
                   //проверка на поле примечание
                   if (box->currentText()=="note") manual_note=false;
               }
            }

           //если не сопоставлено ни одно поле
           if (count_t==0) {
               qDebug() << "не сопоставлено ни одно поле!";
               QMessageBox::critical(this,"Error","Не сопоставлено ни одно поле!");
               ui->plainTextEdit_rep->appendPlainText(QString("Ошибка! Не сопоставлено ни одно поле!"));
               file.close();
               return;
           }
           req.chop(1);

           // если не пустое поле с примечанием и оно не добавлено в таблице добавляем его принудительно
           if (!ui->lineEdit_note->text().isEmpty() && manual_note)
                req.append(",note");
           // если установлен признак поступления добавляем его принудительно
           if (ui->checkBox_receipt->isChecked())
                req.append(",this_receipt");


           req.append(") VALUES ");
            // данные

           int count=0; // счетчик всего
           int countAdd=0; // счетчик добавлено

           // Цикл до конца файла
           while(!ts.atEnd()){
               // для каждой строки
               // читаем стоку
               //ui->plainTextEdit_rep->appendPlainText(QString("добавляем запись: %1").arg(count));
               ui->lineEdit_count->setText(QString("%1").arg(count));
               ui->lineEdit_count->repaint();
               line = ts.readLine().split(sep);

               //удаляем мусор
               line.replaceInStrings("'","\"");
//               line.replaceInStrings("\n"," ");

//               qDebug() << line;

               bool zero=false; //тригер на нулевую сумму платежа
               // формируем строку отдельно, для возможности дропнуть!
               QString val_line = "";
               // начало формирования строки
               val_line.append("(");
               QComboBox *box;
               // для каждого значения (колонки)
               for (int i=0;i < ui->tableWidget->columnCount(); ++i) {
                   // считана строка с меньшим значением элементов
                   if (line.count()< i) {
                       ui->plainTextEdit_rep->appendPlainText(QString("Некорректное чтение строки. Очистите файл от лишних возвратов строки."));
                       ui->plainTextEdit_rep->appendPlainText(line.at(0));
                       qDebug() << line;
                       file.close ();
                       return;
                   }
                   //qDebug() << i;
                   box = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(0,i));
                   if (!box->currentText().isEmpty()) { // если не пустое
                       val_line.append("'");
                       QString tt=line.at(i);
                       // если это контрагент надо проверить на его присуьствие в справочнике и найти id или добавить нового контрагента
                       if (box->currentText() == "counterparty_id") {
                           //проверяем на наличие в базе
                           QString ss = QString("SELECT id, counterparty FROM counterparties WHERE counterparty = '%1'").arg(tt);
                           if(!query.exec(ss))
                                qDebug() << "ERROR serch: " << query.lastError().text();
                            // если уже есть
                           if(query.next()) {
//                               qDebug() << tr("Уже есть подставляем: ") << query.value(0).toString();
                               val_line.append(query.value(0).toString());
                           }
                           else {
                               // иначе добавляем новоко контрагента
                               qDebug() << QString("добавляем контрагента: %1").arg(tt);
                               ui->plainTextEdit_rep->appendPlainText(QString("добавляем контрагента: %1").arg(tt));
                               QString ss = QString("INSERT INTO counterparties (counterparty) VALUES ('%1')").arg(tt);
                               if(!query.exec(ss))
                                  qDebug() << "ERROR add counterparties: " << query.lastError().text();

                               //получаем новый id
                               ss = QString("SELECT id, counterparty FROM counterparties WHERE counterparty = '%1'").arg(tt);
                               if(!query.exec(ss))
                                    qDebug() << "ERROR new serch: " << query.lastError().text();
                               if(query.next()) {
//                                   qDebug() << tr("Добавлено и найдено - подставляем: ") << query.value(0).toString();
                                   val_line.append(query.value(0).toString());
                               }
                                else {
                                   val_line.append(""); // если вставить не выщло - крайний случай
                                   qDebug() << tr("Что то пошло не так - контрагент не добавлен и не найден.");
                               }
                           }

                       }
                       else {
                           // если поле это мумма платежа
                           if (box->currentText() == "amount_of_payment") {
                                   tt.replace(" ",""); //удаляем лишние пробелы
                                   tt.replace(",","."); //меняем запятые на точки
                                   // если ноль устанавливаем признак сброса
                                   if (tt.toDouble() ==0)
                                       zero=true;
//                                       qDebug() << "пустая пропущено> " << tt << " " << tt.toDouble();
                                   //если это восстановление
                                   if (ui->checkBox_recovery->isChecked())
                                       tt="-"+tt; //добавить знак минус    Возможно надо преодразовать в число умножить на -1


                           }
                           // если поле это дата преобразовать в варимый формат
                           if (box->currentText() == "payment_date") {
                                   QDate date = QDate::fromString(tt,"dd.MM.yyyy");
                                   tt=date.toString("yyyy-MM-dd");
                                   //qDebug << date;
                           }
                           // если поле это примечание выбранное в таблице и не пустое примечание в поле лайнэдит то совмещаем строки
                           if (box->currentText() == "note" && !ui->lineEdit_note->text().isEmpty()) {
                                   tt= QString("%1 : %2").arg(ui->lineEdit_note->text()).arg(tt);
                           }



                            //если это не контрагнеты то вставляем просто значение
                            val_line.append(tt);
                       }
                       val_line.append("',");
                   }
                }
               // собрали строку
               val_line.chop(1);

               // если не пустое поле с примечанием и добавлено вручную добавляем его
               if (!ui->lineEdit_note->text().isEmpty() && manual_note) {
                    val_line.append(",'");
                    val_line.append(ui->lineEdit_note->text());
                    val_line.append("'");
               }
               // если установлен признак поступления добавляем его
               if (ui->checkBox_receipt->isChecked()) {
                    val_line.append(",'true'");
               }


               val_line.append("),");
               //если сумма пп не ноль то добавляем строку к запросу
               if (!zero) {
                   req.append(val_line);
                   countAdd++; // счетчик добавленныз строк
               }

//               qDebug()<<req;
               count++; // счетчик обраьотаных записей
               // дергаем интерфейс, что бы не зависал
               QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
               // следующая строка
           }

           // файл кончился - запрос сформирован
           req.chop(1);
           req.append(";");

//            qDebug()<<req;

           if(!query.exec(req))
           {
               qDebug() << req;
               qDebug() << "ERROR Insert: " << query.lastError().text();
               ui->plainTextEdit_rep->appendPlainText(QString("Ошибка импорта. Не импортировано: %1").arg(req));
               ui->plainTextEdit_rep->repaint();

           }

           ui->plainTextEdit_rep->appendPlainText(QString("Импорт завершён! Обработано %1 записей, добавлено %2 записей").arg(count).arg(countAdd));

        }
        else {
           qDebug() << "файл не открыт!";
           QMessageBox::critical(this,"Error","Файл не выбран или недоступен!");
           return;
       }

        file.close ();
}

void FormImport::on_checkBox_recovery_stateChanged(int arg1)
{
    if(ui->checkBox_recovery->isChecked())
        ui->lineEdit_note->setText("ВОССТАНОВЛЕНИЕ");
    else
        ui->lineEdit_note->setText("");

}
