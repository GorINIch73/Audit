#include "formcounterparties.h"
#include "ui_formcounterparties.h"


#include <QDebug>
#include <QSqlQuery>
#include <QMessageBox>

#include <QSqlError>


FormCounterparties::FormCounterparties(QSqlDatabase db,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormCounterparties)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    base=db;

    //создание обьектов таблиц
    modelCounterparties = new QSqlTableModel(this,base);
    modelBank_decryption = new QSqlQueryModel(this);
    mapper = new QDataWidgetMapper(this);

    //Настраиваем модели
    SetupTable();

    modelCounterparties->select();

    // сигнал изменения строки выделения в tableVew
    connect(ui->tableView_counterparties->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                 SLOT(slotSelectionChange(const QItemSelection &, const QItemSelection &)));

    ui->tableView_counterparties->selectRow(0);

}

FormCounterparties::~FormCounterparties()
{
    delete ui;
}

void FormCounterparties::slotSelectionChange(const QItemSelection &current, const QItemSelection &previous)
{
    Tune();
}

void FormCounterparties::SetupTable()
{
    //Таблица сиаией
    modelCounterparties->setTable("counterparties");
    modelCounterparties->setSort(modelCounterparties->fieldIndex("counterparty"),Qt::AscendingOrder);

    // названия колонок
    modelCounterparties->setHeaderData(modelCounterparties->fieldIndex("counterparty"),Qt::Horizontal,"Наименование контрагента");
    modelCounterparties->setHeaderData(modelCounterparties->fieldIndex("note"),Qt::Horizontal,"Примечание");


    ui->tableView_counterparties->setModel(modelCounterparties);
    ui->tableView_counterparties->setColumnHidden(0, true);    // Скрываем колонку с id записей
    ui->tableView_counterparties->setEditTriggers(QAbstractItemView::NoEditTriggers);  //запрет редактирования
    ui->tableView_counterparties->setSelectionBehavior(QAbstractItemView::SelectRows); // Разрешаем выделение строк
    ui->tableView_counterparties->setSelectionMode(QAbstractItemView::SingleSelection); // Устанавливаем режим выделения лишь одно строки в таблице
    ui->tableView_counterparties->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_counterparties->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // по содержимому

    // настриаваем маписн на поля редактирования вопроса
    mapper->setModel(modelCounterparties);

    mapper->addMapping(ui->lineEdit_id, modelCounterparties->fieldIndex("id"));
    mapper->addMapping(ui->plainTextEdit_counterparty, modelCounterparties->fieldIndex("counterparty"));
    mapper->addMapping(ui->lineEdit_note, modelCounterparties->fieldIndex("note"));
    mapper->addMapping(ui->checkBox_noContracts, modelCounterparties->fieldIndex("nocontract"));
    mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);


    //Таблица расшифровок
    modelBank_decryption->setQuery("SELECT * FROM bank_decryption",base);

    // названия колонок
    modelBank_decryption->setHeaderData(0,Qt::Horizontal,"Статья");
    modelBank_decryption->setHeaderData(1,Qt::Horizontal,"Сумма");
    modelBank_decryption->setHeaderData(2,Qt::Horizontal,"Дата платежа");
    modelBank_decryption->setHeaderData(3,Qt::Horizontal,"Номер платежа");
    modelBank_decryption->setHeaderData(4,Qt::Horizontal,"Назначение платежа");
    modelBank_decryption->setHeaderData(5,Qt::Horizontal,"Подтверждение расхода");

    ui->tableView_decryption->setModel(modelBank_decryption);
    ui->tableView_decryption->setEditTriggers(QAbstractItemView::NoEditTriggers);  //запрет редактирования
    ui->tableView_decryption->setSelectionBehavior(QAbstractItemView::SelectRows); // Разрешаем выделение строк
    ui->tableView_decryption->setSelectionMode(QAbstractItemView::SingleSelection); // Устанавливаем режим выделения лишь одно строки в таблице
    ui->tableView_decryption->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_decryption->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // по содержимому
}

void FormCounterparties::Tune()
{

    mapper->submit();
    modelCounterparties->submit();

    // смена контракта
    if (modelCounterparties->data(modelCounterparties->index(ui->tableView_counterparties->currentIndex().row(), 0)).toString().isEmpty()) {
        //если пустая запись
        qDebug() << "данные пустые, но маппер не умеет очищать виджеты, оставим на будущее!";

    }

        //запрос
        QString ff = QString("SELECT articles.article, ROUND(sum,2), bank.payment_date, bank.payment_number, bank.decryption_of_payment, expense_confirmation FROM bank_decryption inner join bank on bank_decryption.bank_id=bank.id inner join articles on bank_decryption.article_id=articles.id WHERE counterparty_id = \%1 ORDER BY articles.article").arg(modelCounterparties->data(modelCounterparties->index(ui->tableView_counterparties->currentIndex().row(), 0)).toString());
//        qDebug() << ff;
        modelBank_decryption->setQuery(ff,base);

        // при изменение строки в таблвьюве устанавливаем маппер на соответствующую запись
        mapper->setCurrentIndex(ui->tableView_counterparties->currentIndex().row());

}

void FormCounterparties::on_pushButton_close_clicked()
{
    // на всякий случай
    mapper->submit();
    modelCounterparties->submit();

    close();
}

void FormCounterparties::on_lineEdit_flt_all_textChanged(const QString &arg1)
{
    mapper->submit();
    modelCounterparties->submit();

    //фильтр c\статей
    if (!arg1.isEmpty()) {
        QString ff = QString("counterparties.counterparty Like '\%%1\%' OR counterparties.note Like '\%%1\%'").arg(arg1);

        modelCounterparties->setFilter(ff);
        modelCounterparties->select();
        ui->tableView_counterparties->selectRow(0);

        Tune(); // дергаем сменой строки принудительно на случай пустого результата

    }
    else {
        modelCounterparties->setFilter("");
        modelCounterparties->select();
        ui->tableView_counterparties->selectRow(0);
    }
}

void FormCounterparties::on_pushButton_first_clicked()
{
    // перая запись
    ui->tableView_counterparties->selectRow(0);
}

void FormCounterparties::on_pushButton_prev_clicked()
{
    // прыгаем на предыдущую запись
    ui->tableView_counterparties->selectRow(ui->tableView_counterparties->currentIndex().row()-1);
}

void FormCounterparties::on_pushButton_next_clicked()
{
    // прыгаем на следующую запись
    ui->tableView_counterparties->selectRow(ui->tableView_counterparties->currentIndex().row()+1);
}

void FormCounterparties::on_pushButton_last_clicked()
{
    // последняя запись
    ui->tableView_counterparties->selectRow(modelCounterparties->rowCount()-1);
}

void FormCounterparties::on_pushButton_refr_clicked()
{
    mapper->submit();
    modelCounterparties->submit();

    //восстановление курсора
    int row = ui->tableView_counterparties->currentIndex().row();
    //обновить по простому
    modelCounterparties->select();
    // восстанавливаем строку
    ui->tableView_counterparties->selectRow(row);
}

void FormCounterparties::on_pushButton_add_clicked()
{
    // добавление
    modelCounterparties->submit(); // субмитим

    int row= ui->tableView_counterparties->currentIndex().row()+1; // выбираем следующую

    // вставляем
    modelCounterparties->insertRow(row);
    modelCounterparties->setData(modelCounterparties->index(row,modelCounterparties->fieldIndex("counterparty")),""); // добавляем пустой для возможности сабмита

    modelCounterparties->submit(); // субмитим
    // устанавливаем курсор на строку редактирования
    ui->tableView_counterparties->selectRow(row);
    // устанавливаем курсор на редактирование имени
    ui->plainTextEdit_counterparty->setFocus();
}

void FormCounterparties::on_pushButton_del_clicked()
{
    // удаление
    // подтверждение
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Внимание!"),
                                                 tr("Уверены в удалении контрагента?")))  return;
    // если есть связи не удаляем!
    QSqlQuery query(base);

    QString tt = QString("SELECT id FROM bank WHERE counterparty_id = \%1").arg(modelCounterparties->data(modelCounterparties->index(ui->tableView_counterparties->currentIndex().row(), 0)).toString());
    query.exec(tt);
    if (query.first()) {
        qDebug() << "Имеются связанные расшифровки. Удаление невозможно!";
        QMessageBox::critical(this,"Error","Имеются связанные расшифровки. Удаление невозможно!");
        return;
    }

    modelCounterparties->removeRow(ui->tableView_counterparties->currentIndex().row());
    // прыгаем на предыдущую запись
    ui->tableView_counterparties->selectRow(ui->tableView_counterparties->currentIndex().row()-1);
}

void FormCounterparties::on_pushButton_flt_clr_clicked()
{
    mapper->submit();
    modelCounterparties->submit();

    ui->lineEdit_flt_all->setText("");

    modelCounterparties->setFilter("");
    modelCounterparties->select();
    ui->tableView_counterparties->selectRow(0);
}
