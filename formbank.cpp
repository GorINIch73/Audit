#include "formbank.h"
#include "ui_formbank.h"
#include "mousewheelwidgetadjustmentguard.h" // класс блокировщик событий прокрутки мыши

#include <QDebug>
#include <QCompleter>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlRecord>

#include <QSqlError>
//#include <QStatusBar>

#include "dialognewcontract.h"


FormBank::FormBank(QSqlDatabase db, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormBank)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);



    base=db;
    //создание обьектов таблиц
    modelBank = new QSqlRelationalTableModel(this,base);
    modelBank_decryption = new QSqlRelationalTableModel(this,base);
    modelArticles = new QSqlTableModel(this,base);
//    modelContracts = new QSqlTableModel(this,base);
    modelContracts = new QSqlQueryModel(this);

    modelCounterparties = new QSqlTableModel(this,base);
//    delegate = new QSqlRelationalDelegateFlt(this);
    delegate = new QSqlRelationalDelegate(this);
    a_delegate = new QSqlRelationalDelegateFlt(this);
    mapper = new QDataWidgetMapper(this);
    completer = new QCompleter(this);
    completer_articles = new QCompleter(this);
    completer_counterparties = new QCompleter(this);
    completer_contracts = new QCompleter(this);

    //Настраиваем модели
    SetupTable();

    //modelQuestionnaire->setFilter("questionnaire.id='250'");

    modelBank->select();
    modelBank_decryption->select();
    modelArticles->select();
    modelCounterparties->select();

    ui->comboBox_flt_counterparties->setCurrentIndex(-1); //убираем значения по умолчанию в комбобоксах после селекта
    ui->comboBox_articles->setCurrentIndex(-1); //убираем значения по умолчанию в комбобоксах после селекта
    ui->comboBox_contracts->setCurrentIndex(-1); //убираем значения по умолчанию в комбобоксах после селекта



    // сигнал изменения строки выделения в tableVewBank
    connect(ui->tableView_bank->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                 SLOT(slotSelectionChange(const QItemSelection &, const QItemSelection &)));

    // сигнал создания запроса во вкладках
    connect(this, SIGNAL(signalFromQuery(QString)),parent, SLOT(slot_goQuery(QString)));
    ui->tableView_bank->selectRow(0);

}

FormBank::~FormBank()
{
    delete ui;
}

void FormBank::on_pushButton_close_clicked()
{
    // на всякий случай
    mapper->submit();
    modelBank->submit();


    close();
}

void FormBank::slotSelectionChange(const QItemSelection &current, const QItemSelection &previous)
{
    // настраиваем фильтр расшифровки в зависимости от выбранного платежа
    TunBank_decryption();

}

void FormBank::SetupTable()
{
    //Таблица пп
    modelBank->setTable("bank");
    modelBank->setSort(modelBank->fieldIndex("payment_date"),Qt::AscendingOrder);

    modelBank->setJoinMode(QSqlRelationalTableModel::LeftJoin); // что бы были видны пустые
    modelBank->setRelation(modelBank->fieldIndex("counterparty_id"), QSqlRelation("counterparties", "id", "counterparty"));


    // названия колонок
    modelBank->setHeaderData(modelBank->fieldIndex("payment_number"),Qt::Horizontal,"Номер ПП");
    modelBank->setHeaderData(modelBank->fieldIndex("payment_date"),Qt::Horizontal,"Дата ПП");
    modelBank->setHeaderData(modelBank->fieldIndex("counterparty_id"),Qt::Horizontal,"Контрагент");
    modelBank->setHeaderData(modelBank->fieldIndex("decryption_of_payment"),Qt::Horizontal,"Назначение платежа");
    modelBank->setHeaderData(modelBank->fieldIndex("amount_of_payment"),Qt::Horizontal,"Сумма платежа");
    modelBank->setHeaderData(modelBank->fieldIndex("this_receipt"),Qt::Horizontal,"Поступление");
    modelBank->setHeaderData(modelBank->fieldIndex("article"),Qt::Horizontal,"статья");
    modelBank->setHeaderData(modelBank->fieldIndex("note"),Qt::Horizontal,"примечание");

//    modelBank->select();

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
//    mapper->addMapping(ui->lineEdit_date, modelBank->fieldIndex("payment_date")); //настроить маску!
    mapper->addMapping(ui->dateEdit_Date, modelBank->fieldIndex("payment_date")); //настроить маску!
    mapper->addMapping(ui->comboBox_counterparty, modelBank->fieldIndex("counterparty_id"));
    mapper->addMapping(ui->plainTextEdit_decryption, modelBank->fieldIndex("decryption_of_payment"));
//    mapper->addMapping(ui->lineEdit_summa, modelBank->fieldIndex("amount_of_payment")); //настроить маску!
    mapper->addMapping(ui->doubleSpinBox_summa, modelBank->fieldIndex("amount_of_payment"));
    mapper->addMapping(ui->checkBox_receipt, modelBank->fieldIndex("this_receipt"));
    mapper->addMapping(ui->lineEdit_article, modelBank->fieldIndex("article"));
    mapper->addMapping(ui->lineEdit_note, modelBank->fieldIndex("note"));


    ui->doubleSpinBox_summa->installEventFilter(new MouseWheelWidgetAdjustmentGuard(ui->doubleSpinBox_summa)); //блокируем прокрутку
    ui->doubleSpinBox_summa->setGroupSeparatorShown(true); //разделитель групп

    ui->dateEdit_Date->installEventFilter(new MouseWheelWidgetAdjustmentGuard(ui->dateEdit_Date)); //блокируем прокрутку


    //комбобокс для контрагентов   сбрасывает на первый элемент при клике - хз что ч этим делать - типа не успевает найти индекс и тупит
    ui->comboBox_counterparty->setModel(modelBank->relationModel(modelBank->fieldIndex("counterparty_id")));
    ui->comboBox_counterparty->setModelColumn(modelBank->relationModel(modelBank->fieldIndex("counterparty_id"))->fieldIndex("counterparty"));
    ui->comboBox_counterparty->setEditable(true);
    ui->comboBox_counterparty->setFocusPolicy(Qt::StrongFocus);
    ui->comboBox_counterparty->installEventFilter(new MouseWheelWidgetAdjustmentGuard(ui->comboBox_counterparty)); //блокируем прокрутку
    ui->comboBox_counterparty->insertItem(0,QString::fromUtf8(NULL)); // добавляем пустой элемент
//    ui->comboBox_counterparty->setCurrentIndex(0); // устанавливаем на пустой элемент

    //     настраиваем комплитер
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
 //    completer->setCompletionMode(QCompleter::InlineCompletion);
 //    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setModel(modelBank->relationModel(modelBank->fieldIndex("counterparty_id")));
    completer->setCompletionColumn(modelBank->relationModel(modelBank->fieldIndex("counterparty_id"))->fieldIndex("counterparty")); // номер колонки с данными подстановки
    ui->comboBox_counterparty->setCompleter(completer);
    //надо настроить прижим текста влево хз как

//    qDebug() << modelBank->fieldIndex("counterparty_id");


    mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

//    modelBank->select();


    //Таблица расшифровок
    modelBank_decryption->setTable("bank_decryption");
    modelBank_decryption->setJoinMode(QSqlRelationalTableModel::LeftJoin); // что бы были видны пустые
    modelBank_decryption->setRelation(modelBank_decryption->fieldIndex("article_id"), QSqlRelation("articles", "id", "article"));
    modelBank_decryption->setRelation(modelBank_decryption->fieldIndex("contract_id"), QSqlRelation("contracts", "id", "contract_number")); // сделать смешанное поле номер+дата
    modelBank_decryption->setEditStrategy(QSqlTableModel::OnFieldChange); // субмитить сразу
//    modelBank_decryption->setEditStrategy(QSqlTableModel::OnRowChange);QSqlTableModel::OnFieldChange

//    modelQuestionnaire->setRelation(modelQuestionnaire->fieldIndex("place_id"), QSqlRelation("place", "id", "name, place_id")); // дополнительное поле индекса

    // названия колонок
    modelBank_decryption->setHeaderData(modelBank_decryption->fieldIndex("sum"),Qt::Horizontal,"Сумма");
    modelBank_decryption->setHeaderData(modelBank_decryption->fieldIndex("article_id"),Qt::Horizontal,"Статья");
    modelBank_decryption->setHeaderData(modelBank_decryption->fieldIndex("contract_id"),Qt::Horizontal,"Контракт");
    modelBank_decryption->setHeaderData(modelBank_decryption->fieldIndex("expense_confirmation"),Qt::Horizontal,"Исполнен");

//    modelBank_decryption->select();

    ui->tableView_decryption->setModel(modelBank_decryption);
    ui->tableView_decryption->setItemDelegate(a_delegate);


    ui->tableView_decryption->setColumnHidden(0, true);    // Скрываем колонку с id записей
    ui->tableView_decryption->setColumnHidden(1, true);
    //ui->tableView_decryption->setEditTriggers(QAbstractItemView::NoEditTriggers);  //запрет редактирования
    ui->tableView_decryption->setSelectionBehavior(QAbstractItemView::SelectRows); // Разрешаем выделение строк
    ui->tableView_decryption->setSelectionMode(QAbstractItemView::SingleSelection); // Устанавливаем режим выделения лишь одно строки в таблице
    ui->tableView_decryption->horizontalHeader()->setStretchLastSection(true);
//    ui->tableView_decryption->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // по содержимому
    ui->tableView_decryption->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // по содержимому
    ui->tableView_decryption->setColumnWidth(3,250);
    ui->tableView_decryption->setColumnWidth(4,200);


    //комбобокс для статей
    modelArticles->setTable("articles");
    ui->comboBox_articles->setModel(modelArticles);
    ui->comboBox_articles->setModelColumn(modelArticles->fieldIndex("article"));
    ui->comboBox_articles->setEditable(true);
    // настраиваем комплитер
    completer_articles->setCaseSensitivity(Qt::CaseInsensitive);
    completer_articles->setFilterMode(Qt::MatchContains);
    completer_articles->setCompletionMode(QCompleter::PopupCompletion);
    //completer_articles->setCompletionMode(QCompleter::InlineCompletion);
    //completer_articles->setCompletionMode(QCompleter::UnfilteredPopupCompletion);

    completer_articles->setModel(modelArticles);
    completer_articles->setCompletionColumn(modelArticles->fieldIndex("article")); // номер колонки с данными подстановки
    ui->comboBox_articles->setCompleter(completer_articles);

    //modelArticles->select();

    //комбобокс для фильтра когтрагентов
    modelCounterparties->setTable("counterparties");
    modelCounterparties->setSort(modelCounterparties->fieldIndex("counterparty"),Qt::AscendingOrder);
    modelCounterparties->select();

    ui->comboBox_flt_counterparties->setModel(modelCounterparties);
    ui->comboBox_flt_counterparties->setModelColumn(modelCounterparties->fieldIndex("counterparty"));
    ui->comboBox_flt_counterparties->setEditable(true);
    // настраиваем комплитер
    completer_counterparties->setCaseSensitivity(Qt::CaseInsensitive);
    completer_counterparties->setFilterMode(Qt::MatchContains);
//    completer_counterparties->setCompletionMode(QCompleter::InlineCompletion);
//    completer_counterparties->setCompletionMode(QCompleter::UnfilteredPopupCompletion);

    completer_counterparties->setModel(modelCounterparties);
    completer_counterparties->setCompletionColumn(modelCounterparties->fieldIndex("counterparty")); // номер колонки с данными подстановки
    ui->comboBox_flt_counterparties->setCompleter(completer_counterparties);

//    ui->comboBox_flt_counterparties->insertItem(0,QString::fromUtf8(NULL)); // добавляем пустой элемент
//    ui->comboBox_flt_counterparties->setCurrentIndex(0);


//    modelCounterparties->select();


    //комбобокс для контрактов
    modelContracts->setQuery("SELECT id, contracts.contract_number || '  ' || contracts.contract_date AS name FROM contracts",base);
//    modelContracts->setSort(modelContracts->fieldIndex("contract_number"),Qt::AscendingOrder);

    ui->comboBox_contracts->setModel(modelContracts);
    ui->comboBox_contracts->setModelColumn(1);
    ui->comboBox_contracts->setEditable(true);
    // настраиваем комплитер
    completer_contracts->setCaseSensitivity(Qt::CaseInsensitive);
    completer_contracts->setFilterMode(Qt::MatchContains);
//    completer_counterparties->setCompletionMode(QCompleter::InlineCompletion);
//    completer_contracts->setCompletionMode(QCompleter::UnfilteredPopupCompletion);

    completer_contracts->setModel(modelContracts);
    completer_contracts->setCompletionColumn(1); // номер колонки с данными подстановки
    ui->comboBox_contracts->setCompleter(completer_contracts);
    ui->comboBox_contracts->setCurrentText("");



}

void FormBank::TunBank_decryption()
{
    // настраиваем фильтр расшифровки в зависимости от выбранного платежа
    QString ff = QString(" bank_id = \%1 ").arg(modelBank->data(modelBank->index(ui->tableView_bank->currentIndex().row(), 0)).toString());

    modelBank_decryption->setFilter(ff);
    modelBank_decryption->select();

    // при изменение строки в таблвьюве устанавливаем маппер на соответствующую запись
    mapper->setCurrentIndex(ui->tableView_bank->currentIndex().row());
    // руками настраиваем индекс комбобокса
    ui->comboBox_counterparty->setCurrentIndex(ui->comboBox_counterparty->findText(ui->comboBox_counterparty->currentText())); // хз только так корректно работает - принудительно ищем индекс


    // посчтитать итого расшифровок

    QSqlQuery query(base);

    QString ss= QString("SELECT round(SUM(sum),2) FROM bank_decryption WHERE bank_id = \%1 ").arg(modelBank->data(modelBank->index(ui->tableView_bank->currentIndex().row(), 0)).toString());
    if(!query.exec(ss)) {
       qDebug() << "ERROR SELECT bank_decryption: " << query.lastError().text();
       return;
    }
    query.first();
//    ui->lineEdit_d_sum->setText(query.value(0).toString());
    ui->lineEdit_d_sum->setText(QString("%L1").arg(query.value(0).toDouble(),-0,'f',2));

    // расчитать сумму ИТОГО банка

    if (!modelBank->filter().isEmpty()) {
        ss= QString("SELECT round(SUM(amount_of_payment),2) FROM bank WHERE (NOT this_receipt = true) AND ( \%1 ) ").arg(modelBank->filter());
        //qDebug() << ss;

        if(!query.exec(ss)) {
           qDebug() << "ERROR SELECT bank: " << query.lastError().text();
           return;
        }
        query.first();
        //qDebug() << "Сумма" << query.value(0).toString();
        ui->lineEdit_summa_b->setText(QString("%L1").arg(query.value(0).toDouble(),-0,'f',2));
    }

}

void FormBank::on_comboBox_counterparty_currentIndexChanged(int index)
{
    //иначе не сохраняет удрал из за скорости
//    mapper->submit();
//    modelBank->submit();
    //qDebug() << "индекс изменен";

}


void FormBank::on_pushButton_first_clicked()
{
//    mapper->submit();
    modelBank->submit();
    // прыгаем на первую
    ui->tableView_bank->selectRow(0);
}

void FormBank::on_pushButton_prev_clicked()
{
//    mapper->submit();
    modelBank->submit();
    // прыгаем на предыдущую запись
    ui->tableView_bank->selectRow(ui->tableView_bank->currentIndex().row()-1);
}

void FormBank::on_pushButton_next_clicked()
{
//    mapper->submit();
    modelBank->submit();
    // прыгаем на следующую запись
    ui->tableView_bank->selectRow(ui->tableView_bank->currentIndex().row()+1);
}

void FormBank::on_pushButton_last_clicked()
{
//    mapper->submit();
    modelBank->submit();
    // последняя запись
    ui->tableView_bank->selectRow(modelBank->rowCount()-1);

}

void FormBank::on_pushButton_refr_clicked()
{

    // субмитим изменения
//    mapper->submit();
    modelBank->submit();
    modelBank_decryption->submit();

    //восстановление курсора
    int row = ui->tableView_bank->currentIndex().row();
    //QString flt = modelBank->filter();

    // перенастраиваем
    SetupTable();

    //селектим
    modelBank->select();
    modelBank_decryption->select();
    modelArticles->select();
    modelCounterparties->select();
    modelContracts->setQuery(modelContracts->query().lastQuery(),base); // хз работает только так

    ui->comboBox_flt_counterparties->setCurrentIndex(-1); //убираем значения по умолчанию в комбобоксах после селекта
    ui->comboBox_articles->setCurrentIndex(-1); //убираем значения по умолчанию в комбобоксах после селекта
    ui->comboBox_contracts->setCurrentIndex(-1); //убираем значения по умолчанию в комбобоксах после селекта

    // восстанавливаем строку
    ui->tableView_bank->selectRow(row);
    QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Обновлено.")));


}

void FormBank::on_pushButton_add_clicked()
{
    modelBank->submit(); // субмитим

    // добавление ПП
//    int row=modelBank->rowCount(); // определяем количество записей
//    int row= 1; // вславляет всегда вротую строку - удобнее всего!
    int row= ui->tableView_bank->currentIndex().row()+1; // вславляем следующую

    // вставляем
    modelBank->insertRow(row);
    modelBank->setData(modelBank->index(row,modelBank->fieldIndex("payment_number")),""); // добавляем пустой номер для возможности сабмита
    modelBank->submit(); // субмитим

    // устанавливаем курсор на строку редактирования
    ui->tableView_bank->selectRow(row);
    // устанавливаем курсор на редактирование номерв
    ui->lineEdit_number->setFocus();
    QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Добавлено.")));


}

void FormBank::on_pushButton_del_clicked()
{
    // удаление ПП
    // вероятно надо добавить групповое удаление ХЗ

    // подтверждение
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Внимание!"),
                                                 tr("Уверены в удалении платежа?")))  return;

    // удаляем расшифровки
    QSqlQuery query(base);

    QString ff= QString("DELETE FROM bank_decryption WHERE bank_id IN (SELECT id FROM bank WHERE bank_id='%1')").arg(modelBank->data(modelBank->index(ui->tableView_bank->currentIndex().row(),modelBank->fieldIndex("id"))).toString() );
    if(!query.exec(ff)) {
       qDebug() << "ERROR DELEDE bank_decryption: " << query.lastError().text();
    }
    //обновить окно расшифровок
    modelBank_decryption->select();

    modelBank->removeRow(ui->tableView_bank->currentIndex().row());
    // прыгаем на предыдущую запись
    ui->tableView_bank->selectRow(ui->tableView_bank->currentIndex().row()-1);
    QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Удалено.")));


}

void FormBank::on_lineEdit_flt_all_textChanged(const QString &arg1)
{
    //общий фильтр по назначению платежа примечанию сумме
    modelBank->submit();
    if (!arg1.isEmpty()) {


        QString ff="";
        QString flt="";
        if (ui->checkBox_MF->isChecked())
        {
            // разбираем по фразам  ----------------------------------------------------------------------------------------------------------------------------------------------
//            QStringList list = arg1.split(",", QString::SkipEmptyParts);
            QStringList list = arg1.split(",");

      //      qDebug() << "запрос" << list;

            for(int i = 0; i < list.size(); i++)
            {
      //          qDebug() << list[i];
                flt.append("(decryption_of_payment Like '\%");
                flt.append(list[i]);
                flt.append("\%' OR bank.note Like '\%");
                flt.append(list[i]);
                flt.append("\%' OR bank.amount_of_payment Like '\%");
                flt.append(list[i]);
                flt.append("\%' OR bank.payment_date Like '\%");
                flt.append(list[i]);
                flt.append("\%' OR bank.counterparty_id IN (SELECT id FROM counterparties WHERE counterparty Like '\%");
                flt.append(list[i]);
                flt.append("\%')) ");

                flt.append(" AND ");

            }
            flt.chop(4);


            ff.append(" ");
            ff.append(flt);
        }
        else
        {
            // обычное условие
            ff = QString("(decryption_of_payment Like '\%%1\%' OR bank.note Like '\%%1\%' OR bank.amount_of_payment Like '\%%1\%' OR bank.payment_date Like '\%%1\%' OR bank.counterparty_id IN (SELECT id FROM counterparties WHERE counterparty Like '\%%1\%'))").arg(arg1);

        }
        // обычное условие
        //ff = QString("(decryption_of_payment %1 OR bank.note %1 OR bank.amount_of_payment %1 OR bank.payment_date %1 OR bank.counterparty_id IN (SELECT id FROM counterparties WHERE counterparty %1))").arg(flt);
        //ff = QString("(%1 OR bank.counterparty_id IN (SELECT id FROM counterparties WHERE counterparty %2))").arg(flt1).arg(flt2);
//        ff = QString("(%1)").arg(flt1);

        //        QString ff = QString("(decryption_of_payment Like '\%%1\%' OR bank.note Like '\%%1\%' OR bank.amount_of_payment Like '\%%1\%' OR bank.payment_date Like '\%%1\%')").arg(arg1);

        // если установлено условие использования в фильтре контрагента то добавляем его
        if (ui->checkBox_use_counterparties->isChecked() && !ui->comboBox_flt_counterparties->currentText().isEmpty())
            ff.append(QString("AND counterparty_id ='%1\'").arg(modelCounterparties->data(modelCounterparties->index(ui->comboBox_flt_counterparties->currentIndex(),modelCounterparties->fieldIndex("id"))).toString()));

        modelBank->setFilter(ff);
        modelBank->select();
        ui->tableView_bank->selectRow(0);
        TunBank_decryption(); // на случай если результат пустой
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Активен фильт по назначению, сумме, примечанию, дате.")));


    }
    else {
        QString ff="";
        // если установлено условие использования в фильтре контрагента то отсавляем только контрагента
        if (ui->checkBox_use_counterparties->isChecked() && !ui->comboBox_flt_counterparties->currentText().isEmpty())
            ff.append(QString(" counterparty_id ='%1\'").arg(modelCounterparties->data(modelCounterparties->index(ui->comboBox_flt_counterparties->currentIndex(),modelCounterparties->fieldIndex("id"))).toString()));

        modelBank->setFilter(ff);
        modelBank->select();
        ui->tableView_bank->selectRow(0);
    }

}

void FormBank::on_pushButton_flt_clr_clicked()
{
    modelBank->submit();
    //очистка фильтра
    ui->lineEdit_flt_num->setText("");
    ui->lineEdit_flt_all->setText("");
    ui->lineEdit_flt_art->setText("");
    ui->checkBox_flt_nodec->setChecked(false);
    ui->checkBox_flt_noContr->setChecked(false);
    ui->checkBox_use_counterparties->setChecked(false);

    modelBank->setFilter("");
    modelBank->select();
    ui->tableView_bank->selectRow(0);
    QCoreApplication::postEvent(this, new QStatusTipEvent(QString("")));

}

void FormBank::on_pushButton_add_dec_clicked()
{
    //добавление расшифровки по сумме ПП
    // субмитим
    modelBank->submit();
    modelBank_decryption->submit();

    // запрос на оперделение уже учтенной суммы платежа, для вычисления остатка
    double summ=modelBank->data(modelBank->index(ui->tableView_bank->currentIndex().row(),modelBank->fieldIndex("amount_of_payment"))).toDouble();
    QSqlQuery query_s(base);
    QString gg= QString("SELECT SUM(sum) FROM bank_decryption WHERE bank_id=%1").arg(modelBank->data(modelBank->index(ui->tableView_bank->currentIndex().row(),modelBank->fieldIndex("id"))).toString());
    if(!query_s.exec(gg)) {
       qDebug() << "ERROR SELECT bank: " << query_s.lastError().text();
    }
    if(query_s.next())
        summ =summ - query_s.value(0).toDouble();
    // если есть что вставлять - если ноль все равно надо вставить!
//    if (summ >0) {
        // определяем количество записей
        int row=modelBank_decryption->rowCount();
        // вставляем следующую
        modelBank_decryption->insertRow(row);

        //заполняем id и и остаток суммы
        modelBank_decryption->setData(modelBank_decryption->index(row,modelBank_decryption->fieldIndex("bank_id")),modelBank->data(modelBank->index(ui->tableView_bank->currentIndex().row(),modelBank->fieldIndex("id"))).toInt());
//        modelBank_decryption->setData(modelBank_decryption->index(row,modelBank_decryption->fieldIndex("sum")),modelBank->data(modelBank->index(ui->tableView_bank->currentIndex().row(),modelBank->fieldIndex("amount_of_payment"))).toDouble());
        modelBank_decryption->setData(modelBank_decryption->index(row,modelBank_decryption->fieldIndex("sum")),summ);
        // устанавливаем курсор на строку редактирования
        ui->tableView_decryption->selectRow(0);
//    }

      QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Расшифровка добавлена.")));

}

void FormBank::on_lineEdit_flt_art_textChanged(const QString &arg1)
{
    //фильтр КОСГУ если проставлено
    modelBank->submit();

    if (!arg1.isEmpty()) {
//        QString ff = QString("article Like '\%%1\%' OR bank.note Like '\%%1\%' OR bank.amount_of_payment Like '\%%1\%'").arg(arg1);
        QString ff = QString("article ='%1\'").arg(arg1);
        modelBank->setFilter(ff);
        modelBank->select();
        ui->tableView_bank->selectRow(0);
        TunBank_decryption(); // на случай если результат пустой
//        QStatusBar::showMessage("",100);
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Активен фильт по статье.")));

    }
    else {
        modelBank->setFilter("");
        modelBank->select();
        ui->tableView_bank->selectRow(0);
    }

}

void FormBank::on_pushButton_article_add_dec_clicked()
{
    //групповое добавление расшифровок с выбранной статьей
    //для всех записей банка согласно фильтра добавить расшифровки со статьей из комбабокса статей и суммой из платежа

    // подтверждение заполнения
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Внимание!"),
                                                 tr("Уверены в автозаполнении расшифровок?")))  return;

    //проверка на выбранную статью
    if (modelArticles->data(modelArticles->index(ui->comboBox_articles->currentIndex(),modelArticles->fieldIndex("id"))).toString().isEmpty()) {
        qDebug() << "Не выбрана статья!";
        QMessageBox::critical(this,"Error",QString("Не выбрана статья!"));
        return;
    }
    // только если задан фильтр -  защита от дурака!
    if(modelBank->filter().isEmpty()) {
        qDebug() << "Не задан фильтр!";
        QMessageBox::critical(this,"Error","Не задан фильтр!");
        return;
    }

    QSqlQuery query(base);
    QSqlQuery query_bank(base);
    QCoreApplication::postEvent(this, new QStatusTipEvent("Автозаполнение расшифровок!"));
//           ui->statusBar()->showMessage(tr("Ready"), 2000);

    // формируем список для банка
    //query_bank.prepare("SELECT id, amount_of_payment FROM bank WHERE :d");
    qDebug() << "FLT: " << modelBank->filter();
    QString ff= QString("SELECT id, amount_of_payment FROM bank WHERE %1").arg(modelBank->filter());
//    qDebug() << ff;
    if(!query_bank.exec(ff)) {
       qDebug() << "ERROR SELECT bank: " << query_bank.lastError().text();
       return;
    }

    int count=0; // счетчик
    // Cтрока в которую будем формировать запросы
    QString req = "INSERT INTO bank_decryption (bank_id, sum, article_id) VALUES ";

    while(query_bank.next()) {

           // запрос на оперделение уже учтенной суммы платежа, для вычисления остатка
           // надо не надо хз?
//           double summ=query_bank.value(1).toDouble();
           double summ=QString(query_bank.value(1).toString()).toDouble();
           QSqlQuery query_s(base);
           QString gg= QString("SELECT SUM(sum) FROM bank_decryption WHERE bank_id=%1").arg(query_bank.value(0).toString());
           if(!query_s.exec(gg)) {
              qDebug() << "ERROR SELECT bank: " << query_s.lastError().text();
           }
//           qDebug() << "-----------------------------------------------------------------------------------------------";
//           qDebug() << "id "  << query_bank.value(0).toString();
//           qDebug() << "из базы String "  << query_bank.value(1).toString();
//           qDebug() << "из базы Double"  << query_bank.value(1).toDouble();
//           qDebug() << "в переменной " << summ;
//           qDebug() << "стринг " << QString::number(summ);

           if(query_s.next())
               summ = summ - query_s.value(0).toDouble();
//           qDebug() << "в переменной после " << summ;

//           qDebug() << summ;
           // если есть что вставлять
           if (summ !=0) {
               req.append("('");
               req.append(query_bank.value(0).toString());
               req.append("','");
               req.append(QString::number(summ,'g',20));
               req.append("','");
               req.append(modelArticles->data(modelArticles->index(ui->comboBox_articles->currentIndex(),modelArticles->fieldIndex("id"))).toString());
               req.append("'),");
               count++;
           }

           // дергаем интерфейс, что бы не зависал
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            // следующая строка
    }

       // файл кончился - запрос сформирован
       req.chop(1);
       req.append(";");

//       qDebug()<<req;

       if(!query.exec(req))
       {
           qDebug() << req;
           qDebug() << "ERROR Insert: " << query.lastError().text();
       }

       QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Добпалено %1 расшифровок.").arg(count)));

       // обновить окно расшифровок
       modelBank_decryption->select();

}

void FormBank::on_pushButton_clear_dec_clicked()
{
    //очистка расшифровок для фильтованой таблицы банка

    //для всех записей банка согласно фильтра

    // подтверждение
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Внимание!"),
                                                 tr("Уверены в очистке расшифровок?")))  return;

    // только если задан фильтр -  защита от дурака!
    if(modelBank->filter().isEmpty()) {
        qDebug() << "Не задан фильтр!";
        QMessageBox::critical(this,"Error","Не задан фильтр!");
        return;
    }

    QSqlQuery query(base);
    QCoreApplication::postEvent(this, new QStatusTipEvent("Очистка расшифровок..."));

    QString ff= QString("DELETE FROM bank_decryption WHERE bank_id IN (SELECT id FROM bank WHERE %1)").arg(modelBank->filter());
    if(!query.exec(ff)) {
       qDebug() << "ERROR SELECT bank: " << query.lastError().text();
       return;
    }

       QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Удаление расшифровок завершено.")));

       // обновить окно расшифровок
       modelBank_decryption->select();

}

void FormBank::on_lineEdit_flt_num_textChanged(const QString &arg1)
{
    //фильтр номеру ПП
    modelBank->submit();

    if (!arg1.isEmpty()) {
        QString ff = QString("payment_number ='%1\'").arg(arg1);
        modelBank->setFilter(ff);
        modelBank->select();
        ui->tableView_bank->selectRow(0);
        TunBank_decryption(); // на случай если результат пустой
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Активет фильт по номеру.")));

    }
    else {
        modelBank->setFilter("");
        modelBank->select();
        ui->tableView_bank->selectRow(0);
    }

}

void FormBank::on_pushButton_next_con_clicked()
{
    // прыгаем на следующую запись
    ui->comboBox_flt_counterparties->setCurrentIndex(ui->comboBox_flt_counterparties->currentIndex()+1);
}

void FormBank::on_pushButton_prev_con_clicked()
{
    // прыгаем на предыдущую запись
    ui->comboBox_flt_counterparties->setCurrentIndex(ui->comboBox_flt_counterparties->currentIndex()-1);
}

void FormBank::on_comboBox_flt_counterparties_currentIndexChanged(int index)
{
    //фильтр номеру контрагенту
    modelBank->submit();
    if (!ui->comboBox_flt_counterparties->currentText().isEmpty()) {
        QString ff = QString("counterparty_id ='%1\'").arg(modelCounterparties->data(modelCounterparties->index(index,modelCounterparties->fieldIndex("id"))).toString());
        modelBank->setFilter(ff);
        modelBank->select();
        ui->tableView_bank->selectRow(0);
        TunBank_decryption(); // на случай если результат пустой
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("В фильтре установлен контрагент номер: %1.").arg(modelCounterparties->data(modelCounterparties->index(index,modelCounterparties->fieldIndex("id"))).toString())));
    }
    else {
        modelBank->setFilter("");
        modelBank->select();
        ui->tableView_bank->selectRow(0);
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("")));

    }

}

void FormBank::on_checkBox_flt_nodec_stateChanged(int arg1)
{
    // фильтр на нерасшифрованых
    modelBank->submit();
    if (ui->checkBox_flt_nodec->isChecked()) {
        qDebug() << "нераспределенные";
        QString ff = QString(" bank.id NOT IN (SELECT bank_id FROM bank_decryption) ");
        modelBank->setFilter(ff);
        modelBank->select();
        ui->tableView_bank->selectRow(0);
        TunBank_decryption(); // на случай если результат пустой
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Активет фильт по нерасцифрованным.")));

    }
    else {
        modelBank->setFilter("");
        modelBank->select();
        ui->tableView_bank->selectRow(0);
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("")));

    }

}

void FormBank::on_checkBox_flt_noContr_stateChanged(int arg1)
{
    // фильтр на безконтрактных
    modelBank->submit();
    if (ui->checkBox_flt_noContr->isChecked()) {
//        QString ff = QString(" bank.id IN (SELECT bank_id FROM bank_decryption WHERE contract_id IS NULL) ");
        QString ff = QString(" bank.id IN (SELECT bank_id FROM bank_decryption left join bank on bank.id = bank_decryption.bank_id left join counterparties on bank.counterparty_id = counterparties.id WHERE not counterparties.nocontract AND contract_id IS NULL)");
        modelBank->setFilter(ff);
        modelBank->select();
        ui->tableView_bank->selectRow(0);
        TunBank_decryption(); // на случай если результат пустой
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Активен фильт по записям без контрактов.")));

    }
    else {
        modelBank->setFilter("");
        modelBank->select();
        ui->tableView_bank->selectRow(0);
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("")));

    }

}

void FormBank::on_pushButton_contracts_add_new_clicked()
{
    // окно создания контракта (запрос номера и даты)
//    QString dd = modelBank->data(modelBank->index(ui->tableView_bank->currentIndex().row(), modelBank->fieldIndex("counterparty_id"))).toString();
//    qDebug() << ui->comboBox_counterparty->currentIndex();

    DialogNewContract *dialog = new DialogNewContract(base, ui->plainTextEdit_decryption->toPlainText(),this);
    if(dialog->exec() == QDialog::Accepted) {
//        qDebug() << "-----------------------------------------------------------";
//        qDebug() << dialog->getDate();
//        qDebug() << dialog->getNumber();
//        qDebug() << dialog->getState();
    // добавляем контракт
        QString idc="";
        QSqlQuery query(base);
        //получаем id контрагента по тексту в комбобоксе - мжет надо брать из модеи?
        //modelBank->data(modelBank->index(ui->tableView_bank->currentIndex().row(), modelBank->fieldIndex("counterparty_id"))).toString()

        if(!query.exec(QString("SELECT id, counterparty FROM counterparties WHERE counterparty = '%1'").arg(ui->comboBox_counterparty->currentText())))
             qDebug() << "ERROR serch counterparties : " << query.lastError().text();
        if(query.first())
            idc = query.value(0).toString();

        QString ss= QString("INSERT INTO contracts (contract_number, contract_date,counterparty_id, state_contract) VALUES (");
        ss.append("'");
        ss.append(dialog->getNumber());
        ss.append("','");
        ss.append(dialog->getDate());
        ss.append("','");
        //ss.append(modelBank->data(modelBank->index(ui->tableView_bank->currentIndex().row(), modelBank->fieldIndex("counterparty_id"))).toString());
        ss.append(idc);
        ss.append("','");
        ss.append(dialog->getState()?'1':'0'); //"true":"false"
        ss.append("')");

//        qDebug() << ss;
    //    qDebug() << ff;
        if(!query.exec(ss)) {
           qDebug() << "ERROR INSERT kontract: " << query.lastError().text();
           return;
        }

        modelContracts->setQuery(modelContracts->query().lastQuery(),base);

    }

}

void FormBank::on_pushButton_contracts_add_dec_clicked()
{
    // простановка в уже существующие расшифровки номера контракта

    //для всех записей банка согласно фильтра добавить в расшифровки договор из комбобокса

    // подтверждение заполнения
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Внимание!"),
                                                 tr("Уверены в простановке контракта?")))  return;

    //проверка на выбранный контракт
    if (modelContracts->data(modelContracts->index(ui->comboBox_contracts->currentIndex(),0)).toString().isEmpty()) {
        qDebug() << "Контракт не выбран!";
        QMessageBox::critical(this,"Error",QString("Контракт не выбран!"));
        return;
    }


    // только если задан фильтр -  защита от дурака!
    if(modelBank->filter().isEmpty()) {
        qDebug() << "Не задан фильтр!";
        QMessageBox::critical(this,"Error","Не задан фильтр!");
        return;
    }

    QSqlQuery query(base);
    QCoreApplication::postEvent(this, new QStatusTipEvent("Простановка контрактов..."));

    QString ff= QString("UPDATE bank_decryption SET contract_id = '%1' WHERE bank_id IN (SELECT id FROM bank WHERE %2)").arg(modelContracts->data(modelContracts->index(ui->comboBox_contracts->currentIndex(),0)).toString()).arg(modelBank->filter());
    if(!query.exec(ff)) {
       qDebug() << "ERROR SELECT bank: " << query.lastError().text();
       return;
    }

       QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Простановка контрактов завершено.")));

       // обновить окно расшифровок
       modelBank_decryption->select();

}

void FormBank::on_pushButton_contracts_clear_dec_clicked()
{
    // очистка контрактов
    //для всех записей банка согласно фильтра

    // подтверждение
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Внимание!"),
                                                 tr("Уверены в очистке контрактов?")))  return;

    // только если задан фильтр -  защита от дурака!
    if(modelBank->filter().isEmpty()) {
        qDebug() << "Не задан фильтр!";
        QMessageBox::critical(this,"Error","Не задан фильтр!");
        return;
    }

    QSqlQuery query(base);
    QCoreApplication::postEvent(this, new QStatusTipEvent("Очистка контрактов..."));

    QString ff= QString("UPDATE bank_decryption SET contract_id = '' WHERE bank_id IN (SELECT id FROM bank WHERE %1)").arg(modelBank->filter());
    if(!query.exec(ff)) {
       qDebug() << "ERROR SELECT bank: " << query.lastError().text();
       return;
    }

       QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Очистка данных о контрактах из расшифровок завершено.")));

       // обновить окно расшифровок
       modelBank_decryption->select();

}

void FormBank::on_pushButton_del_dec_clicked()
{
    //удаление расшифровки

    // подтверждение
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Внимание!"),
                                                 tr("Уверены в удалении расшифровки?")))  return;

    modelBank_decryption->removeRow(ui->tableView_decryption->currentIndex().row());
    // прыгаем на предыдущую запись
    ui->tableView_decryption->selectRow(ui->tableView_decryption->currentIndex().row()-1);


}



void FormBank::on_pushButton_article_repl_dec_clicked()
{
    //групповая замена статьи

    //для всех записей банка согласно фильтра добавить в расшифровки статью из комбобокса

    // подтверждение заполнения
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Внимание!"),
                                                 tr("Уверены в простановке статьи?")))  return;

    //проверка на выбранный контракт
    if (modelArticles->data(modelArticles->index(ui->comboBox_articles->currentIndex(),0)).toString().isEmpty()) {
        qDebug() << "Статья не выбрана!";
        QMessageBox::critical(this,"Error",QString("Статья не выбрана!"));
        return;
    }


    // только если задан фильтр -  защита от дурака!
    if(modelBank->filter().isEmpty()) {
        qDebug() << "Не задан фильтр!";
        QMessageBox::critical(this,"Error","Не задан фильтр!");
        return;
    }

    QSqlQuery query(base);

    QString ff= QString("UPDATE bank_decryption SET article_id = '%1' WHERE bank_id IN (SELECT id FROM bank WHERE %2)").arg(modelArticles->data(modelArticles->index(ui->comboBox_articles->currentIndex(),0)).toString()).arg(modelBank->filter());
    if(!query.exec(ff)) {
       qDebug() << "ERROR SELECT bank: " << query.lastError().text();
       return;
    }


   // обновить окно расшифровок
   modelBank_decryption->select();


}


void FormBank::on_pushButton_clearCnt_clicked()
{
    // очистить контракт
//    qDebug() << modelBank_decryption->fieldIndex("contract_number");
    qDebug() << "Очижаем контакт";

//    qDebug() << modelBank_decryption->record(ui->tableView_decryption->currentIndex().row());
//    qDebug() << modelBank_decryption->data(modelBank_decryption->index(ui->tableView_decryption->currentIndex().row(),4));

//    modelBank_decryption->setData(modelBank_decryption->index(ui->tableView_decryption->currentIndex().row(),modelBank_decryption->fieldIndex("contract_number")),"");
//    modelBank_decryption->record(ui->tableView_decryption->currentIndex().row()).setValue(modelBank_decryption->fieldIndex("contract_number"),"");
//    modelBank_decryption->record(ui->tableView_decryption->currentIndex().row()).setValue(modelBank_decryption->fieldIndex("sum"),"10");
//    modelBank_decryption->submitAll();
//    qDebug() << modelBank_decryption->lastError();
//    qDebug() << modelBank_decryption->data(modelBank_decryption->index(ui->tableView_decryption->currentIndex().row(),4));
//    qDebug() << modelBank_decryption->lastError();
//    modelBank_decryption->select();


    //хз просто так не пашет придется делать запрос
    QSqlQuery query(base);

    QString ff= QString("UPDATE bank_decryption SET contract_id = NULL WHERE id= '%1' ").arg(modelBank_decryption->data(modelBank_decryption->index(ui->tableView_decryption->currentIndex().row(),modelBank_decryption->fieldIndex("id"))).toString());
    if(!query.exec(ff)) {
       qDebug() << "ERROR SELECT bank: " << query.lastError().text();
       return;
    }

    modelBank_decryption->select();
}

void FormBank::on_pushButton_rep_b_clicked()
{
    // запрос на создание списка для проверки

//    emit signalFromQuery("SELECT strftime('%Y',bank.payment_date), bank.this_receipt, articles.article, ROUND(SUM(sum),2), COUNT(bank.payment_number) FROM bank_decryption inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id GROUP BY strftime('%Y',bank.payment_date), bank.this_receipt, articles.article;");
    emit signalFromQuery("SELECT strftime('%Y',bank.payment_date), bank.this_receipt, articles.article, articles.f14, ROUND(SUM(sum),2), COUNT(bank.payment_number) FROM bank_decryption inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id GROUP BY strftime('%Y',bank.payment_date), bank.this_receipt, articles.article ORDER BY strftime('%Y',bank.payment_date), bank.this_receipt, articles.f14, articles.article;");

}

void FormBank::on_pushButton_Rep_Err_clicked()
{
    //запрос на
    emit signalFromQuery("SELECT * FROM (SELECT bank.id, bank.payment_number, bank.payment_date,  ROUND(bank.amount_of_payment,2), ROUND(SUM(sum),2) AS summa, COUNT(bank.payment_number) AS count FROM bank_decryption inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id GROUP BY bank.id, bank_decryption.bank_id) WHERE NOT (amount_of_payment = summa)");

}

void FormBank::on_pushButton_rep_nd_clicked()
{
    //запрос на нерасшифрованные платежи
    // bank.id NOT IN (SELECT bank_id FROM bank_decryption)
    emit signalFromQuery("SELECT bank.payment_date, bank.payment_number, counterparties.counterparty, bank.decryption_of_payment, ROUND(bank.amount_of_payment,2) FROM bank left join counterparties on bank.counterparty_id = counterparties.id WHERE bank.id NOT IN (SELECT bank_id FROM bank_decryption) ORDER BY bank.payment_date, bank.payment_number");

}



void FormBank::on_pushButton_noContr_clicked()
{
    //запрос на платежи без проставленных контрактов по статье
    emit signalFromQuery("SELECT articles.article, bank.payment_date, bank.payment_number, counterparties.counterparty, bank.decryption_of_payment, ROUND(bank.amount_of_payment,2) FROM bank left join counterparties on bank.counterparty_id = counterparties.id left join bank_decryption on bank.id = bank_decryption.bank_id left join articles on articles.id = bank_decryption.article_id WHERE (bank_decryption.contract_id IS NULL AND bank_decryption.bank_id IS NOT NULL)  ORDER BY articles.article, bank.payment_date, bank.payment_number");

}

void FormBank::on_FormBank_destroyed()
{
    // на всякий случай
    mapper->submit();
    modelBank->submit();
}


void FormBank::on_pushButton_rep_p_clicked()
{
    // запрос на список полный с учетом фильтра сортировка по КОСГУ, как для отчета тьлько весь список ПП

    QString flt= "";

    if (!modelBank->filter().isEmpty())
        flt= QString(" WHERE \%1").arg(modelBank->filter());

    qDebug() << flt;

//    emit signalFromQuery(QString("SELECT articles.article, bank.payment_date, bank.payment_number, counterparties.counterparty, ROUND(bank.amount_of_payment,2) AS summa_bank, ROUND(sum,2) AS summa_description, bank.decryption_of_payment, bank.note FROM bank_decryption inner join bank on bank_decryption.bank_id=bank.id inner join counterparties on bank.counterparty_id=counterparties.id inner join articles on bank_decryption.article_id=articles.id  \%1 ORDER BY articles.article, bank.payment_date, counterparties.counterparty").arg(flt));


     emit signalFromQuery(QString("SELECT strftime('%Y',bank.payment_date), bank.this_receipt, articles.article, articles.f14, bank.payment_date, bank.payment_number, bank.decryption_of_payment, ROUND(bank.amount_of_payment,2), ROUND(SUM(sum),2), COUNT(bank.payment_number) FROM bank_decryption inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id \%1 GROUP BY strftime('%Y',bank.payment_date), bank.id, bank.this_receipt, articles.article ORDER BY strftime('%Y',bank.payment_date), bank.this_receipt, articles.f14, articles.article;").arg(flt));
}

