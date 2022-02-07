#include "formcontract.h"
#include "ui_formcontract.h"

#include "mousewheelwidgetadjustmentguard.h" // класс блокировщик событий прокрутки мыши

#include <QDebug>
#include <QCompleter>
#include <QSqlQuery>
#include <QMessageBox>

#include <QSqlError>
#include <QSqlRecord>


FormContract::FormContract(QSqlDatabase db,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormContract)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    base=db;

    //создание обьектов таблиц
    modelContracts = new QSqlRelationalTableModel(this,base);
    modelBank_decryption = new QSqlQueryModel(this);
    modelCounterparties = new QSqlQueryModel(this);

    delegate = new QSqlRelationalDelegate(this);
    mapper = new QDataWidgetMapper(this);
    completer_counterparties = new QCompleter(this);
    completer_flt_counterparties = new QCompleter(this);

    //Настраиваем модели
    SetupTable();

    modelContracts->select();

    // сигнал изменения строки выделения в tableVew
    connect(ui->tableView_contracts->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                 SLOT(slotSelectionChange(const QItemSelection &, const QItemSelection &)));

    // сигнал создания запроса во вкладках
    connect(this, SIGNAL(signalFromQuery(QString)),parent, SLOT(slot_goQuery(QString)));

    ui->tableView_contracts->selectRow(0);

}

FormContract::~FormContract()
{
    delete ui;
}

void FormContract::slotSelectionChange(const QItemSelection &current, const QItemSelection &previous)
{
    // смена контракта

    seekTable();

}

void FormContract::seekTable()
{
    // смена контракта
    if (modelContracts->data(modelContracts->index(ui->tableView_contracts->currentIndex().row(), 0)).toString().isEmpty()) {
        //если пустая запись
        qDebug() << "данные пустые, но маппер не умеет очищать виджеты, оставим на будущее!";

    }

        // настраиваем фильтр расшифровки в зависимости от выбранного контракта
//        QString ff = QString("SELECT sum, articles.article, bank.payment_date, bank.payment_number, expense_confirmation  FROM bank_decryption inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id WHERE contract_id = \%1 order by bank.payment_date").arg(modelContracts->data(modelContracts->index(ui->tableView_contracts->currentIndex().row(), 0)).toString());
        QString ff = QString("SELECT ROUND(sum,2), bank.payment_date, bank.payment_number, articles.article, bank.decryption_of_payment, expense_confirmation, bank.note  FROM bank_decryption inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id WHERE contract_id = \%1 order by bank.payment_date").arg(modelContracts->data(modelContracts->index(ui->tableView_contracts->currentIndex().row(), 0)).toString());
        modelBank_decryption->setQuery(ff,base);

        // при изменение строки в таблвьюве устанавливаем маппер на соответствующую запись
        mapper->setCurrentIndex(ui->tableView_contracts->currentIndex().row());

        // руками настраиваем индекс комбобокса
//        qDebug() << ui->comboBox_counterparty_id->currentText();
//        qDebug() << ui->comboBox_counterparty_id->findText(ui->comboBox_counterparty_id->currentText());

        ui->comboBox_counterparty_id->setCurrentIndex(ui->comboBox_counterparty_id->findText(ui->comboBox_counterparty_id->currentText())); // хз только так корректно работает - принудительно ищем индекс


        // посчтитать итого

        QSqlQuery query(base);


        QString ss= QString("SELECT round(SUM(sum),2) FROM bank_decryption WHERE contract_id = \%1 ").arg(modelContracts->data(modelContracts->index(ui->tableView_contracts->currentIndex().row(), 0)).toString());
        if(!query.exec(ss)) {
           qDebug() << "ERROR SELECT bank_decryption seek: " << query.lastError().text();  // хз почему делает при открытии 3 раза 2 из которых с ошибкой из за пустого списка
           return;
        }
        query.first();
    //    ui->lineEdit_b_sum->setText(QString::number(query.value(0).toDouble(),'g',20));
        ui->lineEdit_b_sum->setText(QString("%L1").arg(query.value(0).toDouble(),-0,'f',2));

//        qDebug() << "bank_decryption seek ok";

        //сравнение суммы
        QPalette palette = ui->lineEdit_b_sum->palette();

        double sR=query.value(0).toDouble();
        double sB=modelContracts->data(modelContracts->index(ui->tableView_contracts->currentIndex().row(), modelContracts->fieldIndex("price"))).toDouble();


        if(sB ==0) {
            // устанавливаем цвет
            palette.setColor(QPalette::Base, Qt::lightGray);
            //palette.setColor(QPalette::Base, Qt::blue);
            ui->lineEdit_b_sum->setPalette(palette);

        }
        else {
            if(sR > sB) {
                // устанавливаем цвет в красный
                // palette.setColor(QPalette::Base, Qt::lightGray);
                palette.setColor(QPalette::Base, Qt::red);
                ui->lineEdit_b_sum->setPalette(palette);
            }
            else {
                // устанавливаем цвет в красный
                palette.setColor(QPalette::Base, Qt::transparent);
                ui->lineEdit_b_sum->setPalette(palette);
            }
        }


}

void FormContract::on_lineEdit_flt_all_textChanged(const QString &arg1)
{
    mapper->submit(); // субмитим поля - вдруг изменились
    modelContracts->submit();

    //фильтр
    if (!arg1.isEmpty()) {
        QString ff = QString("contracts.contract_number Like '\%%1\%' OR contracts.contract_date Like '\%%1\%' OR contracts.price Like '\%%1\%' OR contracts.note Like '\%%1\%' OR contracts.counterparty_id IN (SELECT id FROM counterparties WHERE counterparty LIKE '\%%1\%')").arg(arg1);

        modelContracts->setFilter(ff);
        modelContracts->select();
        ui->tableView_contracts->selectRow(0);

        seekTable(); // дергаем сменой строки принудительно на случай пустого результата

    }
    else {
        modelContracts->setFilter("");
        modelContracts->select();
        ui->tableView_contracts->selectRow(0);
    }

}

void FormContract::SetupTable()
{
    //Таблица контрактов
    modelContracts->setTable("contracts");
    modelContracts->setSort(modelContracts->fieldIndex("contract_date"),Qt::AscendingOrder);

    modelContracts->setJoinMode(QSqlRelationalTableModel::LeftJoin); // что бы были видны пустые
    modelContracts->setRelation(modelContracts->fieldIndex("counterparty_id"), QSqlRelation("counterparties", "id", "counterparty"));


    // названия колонок
    modelContracts->setHeaderData(modelContracts->fieldIndex("contract_number"),Qt::Horizontal,"Номер");
    modelContracts->setHeaderData(modelContracts->fieldIndex("contract_date"),Qt::Horizontal,"Дата");
    modelContracts->setHeaderData(modelContracts->fieldIndex("due_date"),Qt::Horizontal,"Дата исполнения");
    modelContracts->setHeaderData(modelContracts->fieldIndex("counterparty_id"),Qt::Horizontal,"Контрагент");
    modelContracts->setHeaderData(modelContracts->fieldIndex("price"),Qt::Horizontal,"Цена контракта");
    modelContracts->setHeaderData(modelContracts->fieldIndex("state_contract"),Qt::Horizontal,"Госконтракт");
    modelContracts->setHeaderData(modelContracts->fieldIndex("completed"),Qt::Horizontal,"Завершен");
    modelContracts->setHeaderData(modelContracts->fieldIndex("found"),Qt::Horizontal,"Найден");
    modelContracts->setHeaderData(modelContracts->fieldIndex("for_audit"),Qt::Horizontal,"Для проверки");
    modelContracts->setHeaderData(modelContracts->fieldIndex("note"),Qt::Horizontal,"Примечание");


    ui->tableView_contracts->setModel(modelContracts);
    ui->tableView_contracts->setColumnHidden(0, true);    // Скрываем колонку с id записей
    ui->tableView_contracts->setEditTriggers(QAbstractItemView::NoEditTriggers);  //запрет редактирования
    ui->tableView_contracts->setSelectionBehavior(QAbstractItemView::SelectRows); // Разрешаем выделение строк
    ui->tableView_contracts->setSelectionMode(QAbstractItemView::SingleSelection); // Устанавливаем режим выделения лишь одно строки в таблице
    ui->tableView_contracts->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_contracts->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // по содержимому

    // настриаваем маписн на поля редактирования вопроса
    mapper->setModel(modelContracts);
    mapper->setItemDelegate(delegate);

    mapper->addMapping(ui->lineEdit_id, modelContracts->fieldIndex("id"));
    mapper->addMapping(ui->lineEdit_contract_number, modelContracts->fieldIndex("contract_number"));
    mapper->addMapping(ui->dateEdit_contract_date, modelContracts->fieldIndex("contract_date"));
    mapper->addMapping(ui->dateEdit_due_date, modelContracts->fieldIndex("due_date"));
    mapper->addMapping(ui->comboBox_counterparty_id, modelContracts->fieldIndex("counterparty_id"));
    mapper->addMapping(ui->doubleSpinBox_price, modelContracts->fieldIndex("price"));
    mapper->addMapping(ui->checkBox_state_contract, modelContracts->fieldIndex("state_contract"));
    mapper->addMapping(ui->checkBox_completed, modelContracts->fieldIndex("completed"));
    mapper->addMapping(ui->checkBox_found, modelContracts->fieldIndex("found"));
    mapper->addMapping(ui->checkBox_for_audit, modelContracts->fieldIndex("for_audit"));
    mapper->addMapping(ui->plainTextEdit_note, modelContracts->fieldIndex("note"));
    mapper->addMapping(ui->checkBox_for_check, modelContracts->fieldIndex("for_check"));

    mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);


    ui->doubleSpinBox_price->installEventFilter(new MouseWheelWidgetAdjustmentGuard(ui->doubleSpinBox_price)); //блокируем прокрутку
    ui->doubleSpinBox_price->setGroupSeparatorShown(true); //разделитель групп

    ui->dateEdit_contract_date->installEventFilter(new MouseWheelWidgetAdjustmentGuard(ui->dateEdit_contract_date)); //блокируем прокрутку
    ui->dateEdit_due_date->installEventFilter(new MouseWheelWidgetAdjustmentGuard(ui->dateEdit_due_date)); //блокируем прокрутку


    //комбобокс для контрагентов
    ui->comboBox_counterparty_id->setModel(modelContracts->relationModel(modelContracts->fieldIndex("counterparty_id")));
    ui->comboBox_counterparty_id->setModelColumn(modelContracts->relationModel(modelContracts->fieldIndex("counterparty_id"))->fieldIndex("counterparty"));
    ui->comboBox_counterparty_id->setEditable(true);
    ui->comboBox_counterparty_id->setFocusPolicy(Qt::StrongFocus);
    ui->comboBox_counterparty_id->installEventFilter(new MouseWheelWidgetAdjustmentGuard(ui->comboBox_counterparty_id)); //блокируем прокрутку
    ui->comboBox_counterparty_id->insertItem(0,QString::fromUtf8(NULL)); // добавляем пустой элемент
    // настраиваем комплитер
    completer_counterparties->setCaseSensitivity(Qt::CaseInsensitive);
    completer_counterparties->setFilterMode(Qt::MatchContains);
//    completer->setCompletionMode(QCompleter::InlineCompletion);
//    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer_counterparties->setModel(modelContracts->relationModel(modelContracts->fieldIndex("counterparty_id")));
    completer_counterparties->setCompletionColumn(modelContracts->relationModel(modelContracts->fieldIndex("counterparty_id"))->fieldIndex("counterparty")); // номер колонки с данными подстановки
    ui->comboBox_counterparty_id->setCompleter(completer_counterparties);
    //надо настроить прижим текста влево хз как


    //Таблица расшифровок
    //modelBank_decryption->setTable("bank_decryption");

//    QString ff = QString("SELECT ROUND(sum,2), bank.payment_date, bank.payment_number, articles.article, bank.decryption_of_payment, expense_confirmation  FROM bank_decryption inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id WHERE contract_id = \%1 order by bank.payment_date").arg(modelContracts->data(modelContracts->index(ui->tableView_contracts->currentIndex().row(), 0)).toString());

    modelBank_decryption->setQuery("SELECT * FROM bank_decryption",base);

    // названия колонок
    modelBank_decryption->setHeaderData(0,Qt::Horizontal,"Сумма");
    modelBank_decryption->setHeaderData(1,Qt::Horizontal,"Дата платежа");
    modelBank_decryption->setHeaderData(2,Qt::Horizontal,"Номер ПП");
    modelBank_decryption->setHeaderData(3,Qt::Horizontal,"Статья");
    modelBank_decryption->setHeaderData(4,Qt::Horizontal,"Назначение платежа");
    modelBank_decryption->setHeaderData(5,Qt::Horizontal,"Подтверждение расхода");
 //   modelBank_decryption->setHeaderData(6,Qt::Horizontal,"Примечание");

    ui->tableView_decryption->setModel(modelBank_decryption);
    ui->tableView_decryption->setEditTriggers(QAbstractItemView::NoEditTriggers);  //запрет редактирования
    ui->tableView_decryption->setSelectionBehavior(QAbstractItemView::SelectRows); // Разрешаем выделение строк
    ui->tableView_decryption->setSelectionMode(QAbstractItemView::SingleSelection); // Устанавливаем режим выделения лишь одно строки в таблице
    ui->tableView_decryption->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_decryption->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // по содержимому


    //комбобокс для фильтра когтрагентов
    modelCounterparties->setQuery("SELECT id, counterparty FROM counterparties ORDER BY counterparty",base);

    ui->comboBox_flt_counterparties->setModel(modelCounterparties);
    ui->comboBox_flt_counterparties->setModelColumn(1);
    ui->comboBox_flt_counterparties->setEditable(true);
    ui->comboBox_counterparty_id->insertItem(0,QString::fromUtf8(NULL)); // добавляем пустой элемент
    ui->comboBox_flt_counterparties->setCurrentIndex(-1); // прыгаем на пустой
    // настраиваем комплитер
    completer_flt_counterparties->setCaseSensitivity(Qt::CaseInsensitive);
    completer_flt_counterparties->setFilterMode(Qt::MatchContains);
//    completer_counterparties->setCompletionMode(QCompleter::InlineCompletion);
//    completer_counterparties->setCompletionMode(QCompleter::UnfilteredPopupCompletion);

    completer_flt_counterparties->setModel(modelCounterparties);
    completer_flt_counterparties->setCompletionColumn(1); // номер колонки с данными подстановки
    ui->comboBox_flt_counterparties->setCompleter(completer_flt_counterparties);


}


void FormContract::on_pushButton_close_clicked()
{
    // на всякий случай
    mapper->submit();
    modelContracts->submit();

    close();
}

void FormContract::on_pushButton_first_clicked()
{
//    mapper->submit(); // субмитим поля - вдруг изменились
    modelContracts->submit();

    // перая запись
    ui->tableView_contracts->selectRow(0);

}

void FormContract::on_pushButton_prev_clicked()
{
//    mapper->submit(); // субмитим поля - вдруг изменились
    modelContracts->submit();
    // прыгаем на предыдущую запись
    ui->tableView_contracts->selectRow(ui->tableView_contracts->currentIndex().row()-1);
}

void FormContract::on_pushButton_next_clicked()
{
//    mapper->submit(); // субмитим поля - вдруг изменились
    modelContracts->submit();
    // прыгаем на следующую запись
    ui->tableView_contracts->selectRow(ui->tableView_contracts->currentIndex().row()+1);

}

void FormContract::on_pushButton_last_clicked()
{
//    mapper->submit(); // субмитим поля - вдруг изменились
    modelContracts->submit();
    // последняя запись
    qDebug() << modelContracts->rowCount(); // иногда дает количество больше чем записей, и плодит дубликаты во вьюве!!!!!
    ui->tableView_contracts->selectRow(modelContracts->rowCount()-1);
}

void FormContract::on_pushButton_refr_clicked()
{
    mapper->submit(); // субмитим поля - вдруг изменились
    //восстановление курсора
    int row = ui->tableView_contracts->currentIndex().row();

    //обновить по простому
    modelContracts->select();
    modelBank_decryption->setQuery(modelBank_decryption->query().lastQuery(),base);
    modelCounterparties->setQuery(modelCounterparties->query().lastQuery(),base);

    ui->comboBox_flt_counterparties->setCurrentIndex(-1); //обираем выбор

    // восстанавливаем строку
    ui->tableView_contracts->selectRow(row);
}

void FormContract::on_pushButton_add_clicked()
{
    // добавление
    modelContracts->submit(); // субмитим

    //int row=modelContracts->rowCount();     // определяем количество записей

    // будем вставлять на следующую строку - как я понял играет только ыизуальную роль
    int row= ui->tableView_contracts->currentIndex().row()+1; // выбираем следующую

    // вставляем
    modelContracts->insertRow(row);
    modelContracts->setData(modelContracts->index(row,modelContracts->fieldIndex("contract_number")),""); // добавляем пустой номер для возможности сабмита
    //qDebug () << modelContracts->record(row);

    // принудительно обнуляем значения в спинере само не однуляется! ОМСГ Задал в бвзе для этих величин значение по умолчанию
    //    ui->doubleSpinBox_price->setValue(0);
    //    ui->dateEdit_contract_date->setDate(QDate(2000,01,01));
    //    ui->dateEdit_due_date->setDate(QDate(2000,01,01));

    modelContracts->submit(); // субмитим
    //qDebug () << modelContracts->lastError();
    // устанавливаем курсор на строку редактирования
    ui->tableView_contracts->selectRow(row);
    // устанавливаем курсор на редактирование имени
    ui->lineEdit_contract_number->setFocus();

}

void FormContract::on_pushButton_del_clicked()
{
    // удаление ПП
    // вероятно надо добавить групповое удаление ХЗ

    // подтверждение
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Внимание!"),
                                                 tr("Уверены в удалении контракта?")))  return;

    // если есть связи не удаляем!
    QSqlQuery query(base);

    QString tt = QString("SELECT id FROM bank_decryption WHERE contract_id = \%1").arg(modelContracts->data(modelContracts->index(ui->tableView_contracts->currentIndex().row(), 0)).toString());
    query.exec(tt);
    if (query.first()) {
        qDebug() << "Имеются связанные расшифровки. Удаление невозможно!";
        QMessageBox::critical(this,"Error","Имеются связанные расшифровки. Удаление невозможно!");
        return;
    }

    modelContracts->removeRow(ui->tableView_contracts->currentIndex().row());
    // прыгаем на предыдущую запись
    ui->tableView_contracts->selectRow(ui->tableView_contracts->currentIndex().row()-1);

}

void FormContract::on_pushButton_flt_clr_clicked()
{
//    mapper->submit(); // субмитим поля - вдруг изменились
    modelContracts->submit();

    ui->lineEdit_flt_all->setText("");
    ui->checkBox_flt_audit->setChecked(false);

    modelContracts->setFilter("");
    modelContracts->select();
    ui->tableView_contracts->selectRow(0);
}



void FormContract::on_pushButton_prev_con_clicked()
{
    mapper->submit(); // субмитим поля - вдруг изменились
    // прыгаем на предыдущую запись
    ui->comboBox_flt_counterparties->setCurrentIndex(ui->comboBox_flt_counterparties->currentIndex()-1);
}

void FormContract::on_pushButton_next_con_clicked()
{
    mapper->submit(); // субмитим поля - вдруг изменились
    // прыгаем на следующую запись
    ui->comboBox_flt_counterparties->setCurrentIndex(ui->comboBox_flt_counterparties->currentIndex()+1);

}

void FormContract::on_comboBox_flt_counterparties_currentIndexChanged(int index)
{
//    mapper->submit(); // субмитим поля - вдруг изменились
    modelContracts->submit();

    if (!ui->comboBox_flt_counterparties->currentText().isEmpty()) {
        QString ff = QString("counterparty_id ='%1\'").arg(modelCounterparties->data(modelCounterparties->index(index,0)).toString());
        modelContracts->setFilter(ff);
        modelContracts->select();
        ui->tableView_contracts->selectRow(0);
        // при отсутствии результата не дается сигнал смены строки
        seekTable(); // дергаем сменой строки принудительно на случай пустого результата
    }
    else {
        modelContracts->setFilter("");
        modelContracts->select();
        ui->tableView_contracts->selectRow(0);

    }

}

void FormContract::on_pushButton_rep_list_clicked()
{
    // запрос на создание списка
    emit signalFromQuery("SELECT contracts.for_audit, counterparties.counterparty, 'N ' || contracts.contract_number, contracts.contract_date, contracts.state_contract, ROUND(SUM(sum),2), COUNT(DISTINCT bank.id), articles.article, contracts.note  FROM bank_decryption inner join contracts on bank_decryption.contract_id=contracts.id inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id inner join counterparties on contracts.counterparty_id =counterparties.id GROUP BY counterparties.counterparty, contracts.contract_number, contracts.contract_date, articles.article ORDER BY contracts.for_audit, contracts.contract_date, contracts.contract_number");
}

void FormContract::on_pushButton_rep_for_audit_clicked()
{
    // запрос на создание списка для проверки
//    emit signalFromQuery("SELECT counterparties.counterparty, contracts.contract_number, contracts.contract_date, contracts.state_contract, ROUND(SUM(sum),2), COUNT(DISTINCT bank.id), articles.article, contracts.note  FROM bank_decryption inner join contracts on bank_decryption.contract_id=contracts.id inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id inner join counterparties on contracts.counterparty_id =counterparties.id WHERE contracts.for_audit GROUP BY counterparties.counterparty, contracts.contract_number, contracts.contract_date, articles.article ORDER BY contracts.contract_date, contracts.contract_number");
    emit signalFromQuery("SELECT counterparties.counterparty, 'N ' || contracts.contract_number, contracts.contract_date, contracts.state_contract, ROUND(SUM(sum),2), COUNT(DISTINCT bank.id), articles.article, contracts.note  FROM bank_decryption inner join contracts on bank_decryption.contract_id=contracts.id inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id inner join counterparties on contracts.counterparty_id =counterparties.id WHERE contracts.for_audit GROUP BY counterparties.counterparty, contracts.contract_number, contracts.contract_date, articles.article ORDER BY contracts.contract_date, contracts.contract_number");

}

void FormContract::on_pushButton_Dubl_rep_clicked()
{
    // запрос на создание списка дубликатов номеров
    emit signalFromQuery("SELECT contract_number, COUNT(*) FROM contracts GROUP BY contract_number HAVING COUNT(*) > 1");

}

void FormContract::on_checkBox_flt_audit_stateChanged(int arg1)
{
    // фильтр на контракты для аудита
    modelContracts->submit();

    if (ui->checkBox_flt_audit->isChecked()) {
        qDebug() << "для проверки";
        QString ff = QString("for_audit = true");
        // QString ff = QString(" bank.id NOT IN (SELECT bank_id FROM bank_decryption) ");
        modelContracts->setFilter(ff);
        modelContracts->select();
        ui->tableView_contracts->selectRow(0);

        // при отсутствии результата не дается сигнал смены строки
        seekTable(); // дергаем сменой строки принудительно на случай пустого результата
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Активен фильт по выбранным для проверки.")));

    }
    else {
        modelContracts->setFilter("");
        modelContracts->select();
        ui->tableView_contracts->selectRow(0);
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("")));

    }

}

void FormContract::on_pushButton_pList_clicked()
{
    // Список ПП по выделенному договору
    // номер договора
    QString ff = QString("SELECT bank.payment_date, bank.payment_number, ROUND(sum,2), articles.article, counterparties.counterparty, bank.decryption_of_payment, bank.note, contracts.contract_number, contracts.contract_date, contracts.note  FROM bank_decryption inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id inner join contracts on bank_decryption.contract_id=contracts.id inner join counterparties on bank.counterparty_id=counterparties.id WHERE contract_id = \%1 order by bank.payment_date").arg(modelContracts->data(modelContracts->index(ui->tableView_contracts->currentIndex().row(), 0)).toString());

    emit signalFromQuery(ff);
}


void FormContract::on_pushButton__whith_note_clicked()
{
    // Список ПП по отобранным подробный
    // определяем фильтр
    QString flt= modelContracts->filter();
    qDebug() << flt;

    if (!flt.isEmpty())
       flt = QString("WHERE %1").arg(flt);

//    QString ff = QString("SELECT 'N ' || contracts.contract_number, contracts.contract_date, counterparties.counterparty, contracts.note, articles.article, bank.payment_date, bank.payment_number, ROUND(sum,2), bank.decryption_of_payment, bank.note  FROM bank_decryption inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id inner join contracts on bank_decryption.contract_id=contracts.id inner join counterparties on bank.counterparty_id=counterparties.id WHERE NOT contracts.note ='' %1 order by contracts.contract_number, contracts.contract_date, counterparties.counterparty, bank.payment_date").arg(flt);
    QString ff = QString("SELECT 'N ' || contracts.contract_number, contracts.contract_date, counterparties.counterparty, contracts.note, articles.article, bank.payment_date, bank.payment_number, ROUND(sum,2), bank.decryption_of_payment, bank.note  FROM bank_decryption inner join articles on bank_decryption.article_id=articles.id inner join bank on bank_decryption.bank_id=bank.id inner join contracts on bank_decryption.contract_id=contracts.id inner join counterparties on bank.counterparty_id=counterparties.id %1 order by contracts.contract_number, contracts.contract_date, counterparties.counterparty, bank.payment_date").arg(flt);

    emit signalFromQuery(ff);

}



void FormContract::on_checkBox__flt_note_stateChanged(int arg1)
{
    // фильтр на контракты с примечанием
    modelContracts->submit();

    if (ui->checkBox__flt_note->isChecked()) {
        qDebug() << "с примечанием";
        QString ff = QString("NOT contracts.note = ''");
        modelContracts->setFilter(ff);
        modelContracts->select();
        ui->tableView_contracts->selectRow(0);

        // при отсутствии результата не дается сигнал смены строки
        seekTable(); // дергаем сменой строки принудительно на случай пустого результата
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Активен фильт для контрактов с примечанием.")));

    }
    else {
        modelContracts->setFilter("");
        modelContracts->select();
        ui->tableView_contracts->selectRow(0);
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("")));

    }

}


void FormContract::on_checkBox__flt_for_check_stateChanged(int arg1)
{
    // фильтр на контракты на контроле
    modelContracts->submit();

    if (ui->checkBox__flt_for_check->isChecked()) {
        qDebug() << "на контроле";
        QString ff = QString("contracts.for_check = true");
        modelContracts->setFilter(ff);
        modelContracts->select();
        ui->tableView_contracts->selectRow(0);

        // при отсутствии результата не дается сигнал смены строки
        seekTable(); // дергаем сменой строки принудительно на случай пустого результата
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Активен фильт для контрактов для контроля.")));

    }
    else {
        modelContracts->setFilter("");
        modelContracts->select();
        ui->tableView_contracts->selectRow(0);
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("")));

    }
}

