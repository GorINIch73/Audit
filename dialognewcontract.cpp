#include "dialognewcontract.h"
#include "ui_dialognewcontract.h"

#include <QRegExp>
#include <QDebug>
#include <QSqlQuery>

DialogNewContract::DialogNewContract(QSqlDatabase db,QString decryption, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewContract)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Создание контракта"));

    date="";
    number="";
    state=false;
    base=db;
    ui->checkBoxState->setChecked(false);
    ui->plainTextEdit->setPlainText(decryption);

    // дешифруем назначение платежа, выбираем номер и дату контракта
    Decrypt(decryption);
    setColor();
    ui->lineEditNumber->setFocus();

}

DialogNewContract::~DialogNewContract()
{
    delete ui;
}

QString DialogNewContract::getDate()
{
    return date;
}

QString DialogNewContract::getNumber()
{
    return number;
}

bool DialogNewContract::getState()
{
    return state;
}

void DialogNewContract::on_buttonBox_accepted()
{
    // подтверждение сознания - запоминаем значения
    date=ui->dateEditDate->date().toString("yyyy-MM-dd");
    number= ui->lineEditNumber->text();
    state = ui->checkBoxState->isChecked();
}

void DialogNewContract::Decrypt(QString decryption)
{
    // расшифровка
    // \d{1,2}[-/\.]\d{1,2}[-/\.]\d{4} 01.12.1997 без проверок
    // ((0[1-9]|[12][0-9]|3[01])[-\.](0[1-9]|1[012])[-\.]((19|20)(\d{2})|\d{2}))\D 01.12.1998г
    // (кон|конт|контр)\s(\S{1,})\sот\s((0[1-9]|[12][0-9]|3[01])[-\.](0[1-9]|1[012])[-\.]((19|20)(\d{2})|\d{2}))\D
    QString srx="(кон|конт|контр|контракт|дог|догов|договор)(.|)\\s{0,}(N|)\\s{0,}(\\S{1,})\\s{0,}от\\s{0,}((0[1-9]|[12][0-9]|3[01])[-\\.](0[1-9]|1[012])[-\\.]((19|20)(\\d{2})|\\d{2}))\\D";
    //создание регулярного выражения с заданным шаблоном
    QRegExp rx(srx);
    rx.setCaseSensitivity(Qt::CaseInsensitive); // не чувствительность к регистру


    //проверка на корректность
    if (!rx.isValid() && rx.isEmpty() && rx.exactMatch("")){
        qDebug() << "Error! Regex is not valid.";
        return;
    }

    //установка свойства жадости
    //rx.setMinimal(false);
    //позиция курсора поиска

    if(rx.indexIn(decryption) >-1 ) {

        // ищем первое вхождение
//        qDebug() << "cap0";
//        qDebug() << rx.cap(0);
//        qDebug() << rx.cap(1);
//        qDebug() << rx.cap(2);
//        qDebug() << rx.cap(3);
//        qDebug() << rx.cap(4);
//        qDebug() << rx.cap(5);
//        qDebug() << rx.cap(6);
//        qDebug() << rx.cap(7);

        ui->lineEditNumber->setText(rx.cap(4));
        if(rx.cap(9).isNull()) {
            //QString ss = QString("%1.%2.20%3").arg(rx.cap(5)).arg(rx.cap(6)).arg(rx.cap(7));
            ui->dateEditDate->setDate(QDate::fromString(QString("%1.%2.20%3").arg(rx.cap(6)).arg(rx.cap(7)).arg(rx.cap(8)),"dd.MM.yyyy")); //приведение даты к длинному формату
        }
        else
            ui->dateEditDate->setDate(QDate::fromString(rx.cap(5),"dd.MM.yyyy"));

        ui->checkBoxState->setChecked(false);
        // сомнительное определение - часто просто пишут контракт думаю не нужно полагаться на это
        if(rx.cap(1).left(1)=="к")
            ui->checkBoxState->setChecked(true);


    }

}

void DialogNewContract::on_lineEditNumber_textChanged(const QString &arg1)
{
    // проверяем на присутствие такого же имени и даты в бвзе
    setColor();

}

void DialogNewContract::setColor()
{
     if(base.isOpen() && !ui->lineEditNumber->text().isEmpty()) {
        QSqlQuery query(base);
//        QString ss=QString("SELECT * FROM contracts  WHERE contract_number='%1' AND contract_date='%2'").arg(ui->lineEditNumber->text()).arg(ui->dateEditDate->date().toString("yyyy-MM-dd"));
        QString ss=QString("SELECT contract_number, contract_date, counterparties.counterparty FROM contracts left join counterparties on contracts.counterparty_id=counterparties.id WHERE contract_number='%1' AND contract_date='%2'").arg(ui->lineEditNumber->text()).arg(ui->dateEditDate->date().toString("yyyy-MM-dd"));
//        qDebug() << ss;
        query.exec(ss);

        QPalette palette = ui->lineEditNumber->palette();

        if(query.first()) {
            // устанавливаем цвет в красный
//            qDebug() << "Dublikate";
            palette.setColor(QPalette::Base, Qt::red);
            ui->lineEditNumber->setPalette(palette);
            ui->plainTextEdit_m->setPlainText(QString("Дубликат: %1 %2 %3").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString()));
        }
        else {
            // устанавливаем цвет в белый
            //qDebug() << "Color reset";
            ui->plainTextEdit_m->setPlainText("");
            palette.setColor(QPalette::Base, Qt::white);
            ui->lineEditNumber->setPalette(palette);
        }
    }

}
