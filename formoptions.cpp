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


}

FormOptions::~FormOptions()
{
    delete ui;
}

void FormOptions::on_pushButton_close_clicked()
{
    close();
}

void FormOptions::on_pushButton_clearBase_clicked()
{
    // очистка базы

    // подтверждение удаление
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Внимание!"),
                                                 tr("Уверены в удалении данных?")))  return;

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
