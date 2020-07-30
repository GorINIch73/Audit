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
    // откты файл импорта
    // выбор файла
    ui->tableWidget->clear();
    importName = QFileDialog::getOpenFileName(this,QString("Открыть файл"),QDir::currentPath(),tr("Типы файлов (*.csv;*.txt;);;Все файлы (*.*)"));
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
               combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
               ui->tableWidget->setCellWidget(0, i, combo);


//           ui->tableWidget->setItem(0,i,new QTableWidgetItem(line.at(i)));
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

    // подтверждение импорта
    if(!base.isOpen()){
      qDebug() << "база не открыта!";
      QMessageBox::critical(this,"Error","База не открыта!");
      return;
    }

    QSqlQuery query(base);


    QString sep = "\t";
    QString tabl = "bank";
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
           for (int i=0;i < ui->tableWidget->columnCount(); ++i) {
               box = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(0,i));
               //qDebug() << "значение ключа:" << box->currentText() << "ключ:" << box->currentIndex();
               if (!box->currentText().isEmpty()) { // если не пустое
                   req.append(box->currentText());
                   req.append(",");
               }
            }
           req.chop(1);
           req.append(") VALUES ");


            // данные

           int count=0; // счетчик

           // Цикл до конца файла
           while(!ts.atEnd()){
               // читаем стоку
               //ui->plainTextEdit_rep->appendPlainText(QString("добавляем запись: %1").arg(count));
               ui->lineEdit_count->setText(QString("%1").arg(count));
               ui->lineEdit_count->repaint();
               line = ts.readLine().split(sep);

               //удаляем мусор
               line.replaceInStrings("'","\"");
//               line.replaceInStrings("\n"," ");

//               qDebug() << line;
               req.append("(");
               QComboBox *box;
               for (int i=0;i < ui->tableWidget->columnCount(); ++i) {
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
                       req.append("'");
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
                               req.append(query.value(0).toString());
                           }
                           else {
                               // иначе добавляем
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
                                   req.append(query.value(0).toString());
                               }
                                else {
                                   req.append(""); // если вставить не выщло - крайний случай
                                   qDebug() << tr("Что то пошло не так.");
                               }
                           }

                       }
                       else {
                           //если это не контрагнеты то вставляем просто значение
                            req.append(tt);
                       }
                       req.append("',");
                   }
                }

               req.chop(1);
               req.append("),");

//               qDebug()<<req;
               count++;
           }
           req.chop(1);
           req.append(";");

            qDebug()<<req;

           if(!query.exec(req))
           {
               qDebug() << req;
               qDebug() << "ERROR Insert: " << query.lastError().text();
               ui->plainTextEdit_rep->appendPlainText(QString("Ошибка импорта. Не импортировано: %1").arg(req));
               ui->plainTextEdit_rep->repaint();

           }

           ui->plainTextEdit_rep->appendPlainText(QString("Импорт завершон! Обработано %1 записей").arg(count-1));

        }
        file.close ();
}
