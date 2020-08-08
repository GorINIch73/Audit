#include "formoptions.h"
#include "ui_formoptions.h"

#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>
#include <QDebug>


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
        if (!a_query.exec("SELECT organization, date_begin, date_end, rep_contract_found FROM options"))
                qDebug() << "Ошибка чтения настроек: " << a_query.lastError().text();
        else {
            a_query.first();
            ui->lineEdit_organization->setText(a_query.value(0).toString());
            ui->dateEdit_begin->setDate(QDate::fromString(a_query.value(1).toString(),"yyyy-MM-dd"));
            ui->dateEdit_end->setDate(QDate::fromString(a_query.value(2).toString(),"yyyy-MM-dd"));
            ui->checkBox_rep_contract_found->setChecked(a_query.value(3).toBool());
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
        QString ss=QString("UPDATE options SET organization = '%1', date_begin = '%2', date_end = '%3', rep_contract_found = '%4';").arg(ui->lineEdit_organization->text()).arg(ui->dateEdit_begin->date().toString("yyyy-MM-dd")).arg(ui->dateEdit_end->date().toString("yyyy-MM-dd")).arg(ui->checkBox_rep_contract_found->isChecked()?"true":"false");
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
        // запрос на очистку договоров
        if (!a_query.exec("DELETE FROM contracts;"))
            qDebug() << "таблица договоров: " << a_query.lastError().text();

        // сбрасываем счетчик договоров id
        if (!a_query.exec("UPDATE sqlite_sequence set seq=0 WHERE Name='contracts'"))
            qDebug() << "счетчик договоров: " << a_query.lastError().text();

    }



    //
    QMessageBox::information(this,"Info","Операция завершена.");

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
