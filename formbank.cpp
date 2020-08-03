#include "formbank.h"
#include "ui_formbank.h"
#include "mousewheelwidgetadjustmentguard.h" // класс блокировщик событий прокрутки мыши

#include <QDebug>
#include <QCompleter>
#include <QSqlQuery>
#include <QMessageBox>

#include <QSqlError>


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
    modelContracts = new QSqlTableModel(this,base);
    modelCounterparties = new QSqlTableModel(this,base);
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


    // сигнал изменения строки выделения в tableVewBank
    connect(ui->tableView_bank->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                 SLOT(slotSelectionChange(const QItemSelection &, const QItemSelection &)));

    ui->tableView_bank->selectRow(0);

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
    modelBank->setSort(modelBank->fieldIndex("payment_date"),Qt::AscendingOrder);

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
    ui->dateEdit_Date->installEventFilter(new MouseWheelWidgetAdjustmentGuard(ui->dateEdit_Date)); //блокируем прокрутку


    //комбобокс для контрагентов
    ui->comboBox_counterparty->setModel(modelBank->relationModel(modelBank->fieldIndex("counterparty_id")));
    ui->comboBox_counterparty->setModelColumn(modelBank->relationModel(modelBank->fieldIndex("counterparty_id"))->fieldIndex("counterparty"));
    ui->comboBox_counterparty->setEditable(true);
    ui->comboBox_counterparty->setFocusPolicy(Qt::StrongFocus);
    ui->comboBox_counterparty->installEventFilter(new MouseWheelWidgetAdjustmentGuard(ui->comboBox_counterparty)); //блокируем прокрутку
    // настраиваем комплитер
    // надо переделать на фильтрацию по тексту
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    completer->setModel(modelBank->relationModel(modelBank->fieldIndex("counterparty_id")));
    completer->setCompletionColumn(modelBank->relationModel(modelBank->fieldIndex("counterparty_id"))->fieldIndex("counterparty")); // номер колонки с данными подстановки
    ui->comboBox_counterparty->setCompleter(completer);
    //надо настроить прижим текста влево хз как
//    ui->comboBox_counterparty->lineEdit()->setAlignment(Qt::AlignHCenter); // выраснивание лево не работает
//    ui->comboBox_counterparty->view()->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
//    ui->comboBox_counterparty->view()->setAutoScroll(true);
//    ui->comboBox_counterparty->view()->setTextElideMode(Qt::ElideLeft);
//    ui->comboBox_counterparty->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
//    ui->comboBox_counterparty->view()->autoScrollMargin();

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



    //комбобокс для статей
    modelArticles->setTable("articles");
    ui->comboBox_articles->setModel(modelArticles);
    ui->comboBox_articles->setModelColumn(modelArticles->fieldIndex("article"));
    ui->comboBox_articles->setEditable(true);
    // настраиваем комплитер
    completer_articles->setCaseSensitivity(Qt::CaseInsensitive);
    completer_articles->setFilterMode(Qt::MatchContains);
    completer_articles->setCompletionMode(QCompleter::InlineCompletion);
    completer_articles->setModel(modelArticles);
    completer_articles->setCompletionColumn(modelArticles->fieldIndex("article")); // номер колонки с данными подстановки
    ui->comboBox_articles->setCompleter(completer_articles);


    //комбобокс для фильтра когтрагентов
    modelCounterparties->setTable("counterparties");
    modelCounterparties->setSort(modelCounterparties->fieldIndex("counterparty"),Qt::AscendingOrder);

    ui->comboBox_flt_counterparties->setModel(modelCounterparties);
    ui->comboBox_flt_counterparties->setModelColumn(modelCounterparties->fieldIndex("counterparty"));
    ui->comboBox_flt_counterparties->setEditable(true);
    // настраиваем комплитер
    completer_counterparties->setCaseSensitivity(Qt::CaseInsensitive);
    completer_counterparties->setFilterMode(Qt::MatchContains);
    //completer_counterparties->setCompletionMode(QCompleter::InlineCompletion);
    completer_counterparties->setModel(modelCounterparties);
    completer_counterparties->setCompletionColumn(modelCounterparties->fieldIndex("counterparty")); // номер колонки с данными подстановки
    ui->comboBox_flt_counterparties->setCompleter(completer_counterparties);

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
    // добавить восстановление курсора
    //обновить
    //QCoreApplication::postEvent(this, new QStatusTipEvent("Обновление"));
    modelBank->select();
    modelBank_decryption->select();
    modelArticles->select();
}

void FormBank::on_pushButton_add_clicked()
{

}

void FormBank::on_pushButton_del_clicked()
{

}

void FormBank::on_lineEdit_flt_all_textChanged(const QString &arg1)
{
    //фильтр по назначению платежа примечанию сумме
    if (!arg1.isEmpty()) {
        QString ff = QString("decryption_of_payment Like '\%%1\%' OR bank.note Like '\%%1\%' OR bank.amount_of_payment Like '\%%1\%' OR bank.payment_date Like '\%%1\%'").arg(arg1);
//        QString ff = QString("decryption_of_payment Like '\%%1\%' OR bank.note Like '\%%1\%' OR bank.amount_of_payment Like '\%%1\%'").arg(arg1);
        modelBank->setFilter(ff);
        modelBank->select();
        ui->tableView_bank->selectRow(0);

    }
    else {
        modelBank->setFilter("");
        modelBank->select();
        ui->tableView_bank->selectRow(0);
    }

}

void FormBank::on_pushButton_flt_clr_clicked()
{
    ui->lineEdit_flt_num->setText("");
    ui->lineEdit_flt_all->setText("");
    ui->lineEdit_flt_art->setText("");

    modelBank->setFilter("");
    modelBank->select();
    ui->tableView_bank->selectRow(0);

}

void FormBank::on_pushButton_add_dec_clicked()
{
    //добавление расшифровки по сумме ПП


    // запрос на оперделение уже учтенной суммы платежа, для вычисления остатка
    double summ=modelBank->data(modelBank->index(ui->tableView_bank->currentIndex().row(),modelBank->fieldIndex("amount_of_payment"))).toDouble();
    QSqlQuery query_s(base);
    QString gg= QString("SELECT SUM(sum) FROM bank_decryption WHERE bank_id=%1").arg(modelBank->data(modelBank->index(ui->tableView_bank->currentIndex().row(),modelBank->fieldIndex("id"))).toString());
    if(!query_s.exec(gg)) {
       qDebug() << "ERROR SELECT bank: " << query_s.lastError().text();
    }
    if(query_s.next())
        summ =summ - query_s.value(0).toDouble();
    // если есть что вставлять
    if (summ >0) {
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
    }

}

void FormBank::on_lineEdit_flt_art_textChanged(const QString &arg1)
{
    //фильтр КОСГУ если проставлено
    if (!arg1.isEmpty()) {
//        QString ff = QString("article Like '\%%1\%' OR bank.note Like '\%%1\%' OR bank.amount_of_payment Like '\%%1\%'").arg(arg1);
        QString ff = QString("article ='%1\'").arg(arg1);
        modelBank->setFilter(ff);
        modelBank->select();
        ui->tableView_bank->selectRow(0);

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
           qDebug() << "в переменной после " << summ;

           qDebug() << summ;
           // если есть что вставлять
           if (summ >0) {
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

    //для всех записей банка согласно фильтра добавить расшифровки со статьей из комбабокса статей и суммой из платежа

    // подтверждение заполнения
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Внимание!"),
                                                 tr("Уверены в очистке расшифровок?")))  return;


    QSqlQuery query(base);
//    QSqlQuery query_bank(base);
    QCoreApplication::postEvent(this, new QStatusTipEvent("Очистка расшифровок!"));

//    // формируем список для банка
//    //query_bank.prepare("SELECT id, amount_of_payment FROM bank WHERE :d");
//    qDebug() << "FLT: " << modelBank->filter();
//    QString ff= QString("SELECT id, amount_of_payment FROM bank WHERE %1").arg(modelBank->filter());
//    if(!query_bank.exec(ff)) {
//       qDebug() << "ERROR SELECT bank: " << query_bank.lastError().text();
//       return;
//    }



    QString ff= QString("DELETE FROM bank_decryption WHERE bank_id IN (SELECT id FROM bank WHERE %1)").arg(modelBank->filter());
    if(!query.exec(ff)) {
       qDebug() << "ERROR SELECT bank: " << query.lastError().text();
       return;
    }



//    int count=0; // счетчик
//    int count_res=0; // счетчик сбросов

//    // Cтрока в которую будем формировать запросы
//    QString req = "";

//    //невозможно запихнуть более 1000 OR в запрос придется разбирать

//    while(query_bank.next()) {

//           req.append(" bank_id = ");
//           req.append(query_bank.value(0).toString());
//           req.append(" OR");

//           count++;
//           if (count==500) {
//               // промежуточное удаление
//               req.chop(1);
//               req.chop(1);
//               req = QString("DELETE FROM bank_decryption WHERE %1").arg(req);
//               qDebug() << req;

//               if(!query.exec(req))
//               {
//                   qDebug() << req;
//                   qDebug() << "ERROR Delete: " << query.lastError().text();
//               }

//               count=0;
//               count_res++;
//               req = "";
//               qDebug() << "del: " << count_res;

//           }
//           // дергаем интерфейс, что бы не зависал
//            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
//            // следующая строка
//    }
// //       qDebug()<<req;
//    if (!req.isEmpty()) {
//     // кончился - запрос сформирован - удаляем остаток
//       req.chop(1);
//       req.chop(1);
//       req = QString("DELETE FROM bank_decryption WHERE %1").arg(req);

//       //qDebug()<<req;

//       if(!query.exec(req))
//       {
//           qDebug() << req;
//           qDebug() << "ERROR Delete: " << query.lastError().text();
//       }
//    }

//       QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Удалено %1 расшифровок.").arg(count_res*500+count)));
       QCoreApplication::postEvent(this, new QStatusTipEvent(QString("Удаление расшифровок завершено.")));

       // обновить окно расшифровок
       modelBank_decryption->select();

}

void FormBank::on_lineEdit_flt_num_textChanged(const QString &arg1)
{
    //фильтр номеру ПП
    if (!arg1.isEmpty()) {
        QString ff = QString("payment_number ='%1\'").arg(arg1);
        modelBank->setFilter(ff);
        modelBank->select();
        ui->tableView_bank->selectRow(0);

    }
    else {
        modelBank->setFilter("");
        modelBank->select();
        ui->tableView_bank->selectRow(0);
    }

}

void FormBank::on_pushButton_next_con_clicked()
{
    // прыгаем на предыдущую запись
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

    if (!ui->comboBox_flt_counterparties->currentText().isEmpty()) {
        QString ff = QString("counterparty_id ='%1\'").arg(modelCounterparties->data(modelCounterparties->index(index,modelCounterparties->fieldIndex("id"))).toString());
        modelBank->setFilter(ff);
        modelBank->select();
        ui->tableView_bank->selectRow(0);
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("В фильтре установлен контрагент номер: %1.").arg(modelCounterparties->data(modelCounterparties->index(index,modelCounterparties->fieldIndex("id"))).toString())));
    }
    else {
        modelBank->setFilter("");
        modelBank->select();
        ui->tableView_bank->selectRow(0);
        QCoreApplication::postEvent(this, new QStatusTipEvent(QString("")));

    }

}
