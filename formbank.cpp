#include "formbank.h"
#include "ui_formbank.h"

#include <QDebug>
#include <QCompleter>

FormBank::FormBank(QSqlDatabase db, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormBank)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    base=db;

    modelBank = new QSqlRelationalTableModel(this,base);
    modelBank_decryption = new QSqlRelationalTableModel(this,base);
    delegate = new QSqlRelationalDelegate(this);
    a_delegate = new QSqlRelationalDelegateFlt(this);
    mapper = new QDataWidgetMapper(this);
    completer = new QCompleter(this);

    //Настраиваем модели
    SetupTable();

    //modelQuestionnaire->setFilter("questionnaire.id='250'");

    modelBank->select();
    modelBank_decryption->select();


    // сигнал изменения строки выделения в tableVewBank
    connect(ui->tableView_bank->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                 SLOT(slotSelectionChange(const QItemSelection &, const QItemSelection &)));

    ui->tableView_bank->selectRow(0) ;

}

FormBank::~FormBank()
{
    delete ui;
}

void FormBank::on_pushButton_close_clicked()
{
    close();
}

void FormBank::slotSelectionChange(const QItemSelection &current, const QItemSelection &previous)
{
    // настраиваем фильтр расшифровки в зависимости от выбранного платежа
    QString ff = QString(" bank_id = \%1 ").arg(modelBank->data(modelBank->index(ui->tableView_bank->currentIndex().row(), 0)).toString());

    modelBank_decryption->setFilter(ff);
    modelBank_decryption->select();

    // при изменение строки в таблвьюве устанавливаем маппер на соответствующую запись
    mapper->setCurrentIndex(ui->tableView_bank->currentIndex().row());
}

void FormBank::SetupTable()
{
    //Таблица пп
    modelBank->setTable("bank");
    modelBank->setJoinMode(QSqlRelationalTableModel::LeftJoin); // что бы были видны пустые
    modelBank->setRelation(modelBank->fieldIndex("counterparty_id"), QSqlRelation("counterparties", "id", "counterparty"));


    // названия колонок
//    modelBank->setHeaderData(1,Qt::Horizontal,"Вопрос");
    ui->tableView_bank->setModel(modelBank);
    ui->tableView_bank->setColumnHidden(0, true);    // Скрываем колонку с id записей
    ui->tableView_bank->setEditTriggers(QAbstractItemView::NoEditTriggers);  //запрет редактирования
    ui->tableView_bank->setSelectionBehavior(QAbstractItemView::SelectRows); // Разрешаем выделение строк
    ui->tableView_bank->setSelectionMode(QAbstractItemView::SingleSelection); // Устанавливаем режим выделения лишь одно строки в таблице
    ui->tableView_bank->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_bank->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // по содержимому

    // настриаваем маписн на поля редактирования вопроса
    mapper->setModel(modelBank);
    mapper->setItemDelegate(delegate);

    mapper->addMapping(ui->lineEdit_id, modelBank->fieldIndex("id"));
    mapper->addMapping(ui->lineEdit_number, modelBank->fieldIndex("payment_number"));
    mapper->addMapping(ui->lineEdit_date, modelBank->fieldIndex("payment_date")); //настроить маску!
    mapper->addMapping(ui->comboBox_counterparty, modelBank->fieldIndex("counterparty_id"));
    mapper->addMapping(ui->plainTextEdit_decryption, modelBank->fieldIndex("decryption_of_payment"));
    mapper->addMapping(ui->lineEdit_summa, modelBank->fieldIndex("amount_of_payment")); //настроить маску!
    mapper->addMapping(ui->checkBox_receipt, modelBank->fieldIndex("this_receipt"));
    mapper->addMapping(ui->lineEdit_article, modelBank->fieldIndex("article"));
    mapper->addMapping(ui->lineEdit_note, modelBank->fieldIndex("note"));

    //комбобокс для контрагентов
    ui->comboBox_counterparty->setModel(modelBank->relationModel(modelBank->fieldIndex("counterparty_id")));
    ui->comboBox_counterparty->setModelColumn(modelBank->relationModel(modelBank->fieldIndex("counterparty_id"))->fieldIndex("counterparty"));
    // настраиваем комплитер
    // надо переделать на фильтрацию по тексту
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    completer->setModel(modelBank->relationModel(modelBank->fieldIndex("counterparty_id")));
    completer->setCompletionColumn(modelBank->relationModel(modelBank->fieldIndex("counterparty_id"))->fieldIndex("counterparty")); // номер колонки с данными подстановки
    ui->comboBox_counterparty->setCompleter(completer);
    //    ui->comboBox_counterparty->setModelColumn(0);
//    qDebug() << modelBank->fieldIndex("counterparty_id");


    mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);



    //Таблица расшифровок
    modelBank_decryption->setTable("bank_decryption");
    modelBank_decryption->setJoinMode(QSqlRelationalTableModel::LeftJoin); // что бы были видны пустые
    modelBank_decryption->setRelation(modelBank_decryption->fieldIndex("article_id"), QSqlRelation("articles", "id", "article"));
    modelBank_decryption->setRelation(modelBank_decryption->fieldIndex("contract_id"), QSqlRelation("contracts", "id", "contract_number")); // сделать смешанное поле номер+дата

//    modelQuestionnaire->setRelation(modelQuestionnaire->fieldIndex("place_id"), QSqlRelation("place", "id", "name, place_id")); // дополнительное поле индекса

    // названия колонок
//    modelBank_decryption->setHeaderData(1,Qt::Horizontal,"Вопрос");
//    modelBank_decryption->setHeaderData(2,Qt::Horizontal,"Ответ");

    ui->tableView_decryption->setModel(modelBank_decryption);
    ui->tableView_decryption->setItemDelegate(a_delegate);

//    ui->tableView_decryption->setColumnHidden(0, true);    // Скрываем колонку с id записей
//    ui->tableView_decryption->setColumnHidden(1, true);
    //ui->tableView_decryption->setEditTriggers(QAbstractItemView::NoEditTriggers);  //запрет редактирования
    ui->tableView_decryption->setSelectionBehavior(QAbstractItemView::SelectRows); // Разрешаем выделение строк
    ui->tableView_decryption->setSelectionMode(QAbstractItemView::SingleSelection); // Устанавливаем режим выделения лишь одно строки в таблице
    ui->tableView_decryption->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_decryption->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // по содержимому
}

void FormBank::TunBank_decryption()
{

}

void FormBank::on_comboBox_counterparty_currentIndexChanged(int index)
{
    //иначе не сохраняет
    mapper->submit();
    modelBank->submit();
}


void FormBank::on_pushButton_first_clicked()
{
    // прыгаем на первую
    ui->tableView_bank->selectRow(0);
}

void FormBank::on_pushButton_prev_clicked()
{
    // прыгаем на предыдущую запись
    ui->tableView_bank->selectRow(ui->tableView_bank->currentIndex().row()-1);
}

void FormBank::on_pushButton_next_clicked()
{
    // прыгаем на следующую запись
    ui->tableView_bank->selectRow(ui->tableView_bank->currentIndex().row()+1);
}

void FormBank::on_pushButton_last_clicked()
{
    // последняя запись
    ui->tableView_bank->selectRow(modelBank->rowCount()-1);

}

void FormBank::on_pushButton_refr_clicked()
{

}

void FormBank::on_pushButton_add_clicked()
{

}

void FormBank::on_pushButton_del_clicked()
{

}
