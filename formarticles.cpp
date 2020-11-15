#include "formarticles.h"
#include "ui_formarticles.h"

#include <QDebug>
#include <QSqlQuery>
#include <QMessageBox>

#include <QSqlError>
//#include <QSqlRecord>

FormArticles::FormArticles(QSqlDatabase db,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormArticles)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    base=db;

    //создание обьектов таблиц
    modelArticles = new QSqlTableModel(this,base);
    modelBank_decryption = new QSqlQueryModel(this);
    mapper = new QDataWidgetMapper(this);

    //Настраиваем модели
    SetupTable();

    modelArticles->select();

    // сигнал изменения строки выделения в tableVew
    connect(ui->tableView_articles->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                 SLOT(slotSelectionChange(const QItemSelection &, const QItemSelection &)));

    // сигнал создания запроса во вкладках
    connect(this, SIGNAL(signalFromQuery(QString)),parent, SLOT(slot_goQuery(QString)));
    ui->tableView_articles->selectRow(0);

}

FormArticles::~FormArticles()
{
    delete ui;
}

void FormArticles::slotSelectionChange(const QItemSelection &current, const QItemSelection &previous)
{
    Tune();
}

void FormArticles::on_pushButton_close_clicked()
{
    // на всякий случай
    mapper->submit();
    modelArticles->submit();

    close();
}

void FormArticles::SetupTable()
{
    //Таблица сиаией
    modelArticles->setTable("articles");
    modelArticles->setSort(modelArticles->fieldIndex("article"),Qt::AscendingOrder);

    // названия колонок
    modelArticles->setHeaderData(modelArticles->fieldIndex("article"),Qt::Horizontal,"Название статьи");
    modelArticles->setHeaderData(modelArticles->fieldIndex("code"),Qt::Horizontal,"Код статьи");
    modelArticles->setHeaderData(modelArticles->fieldIndex("subcode"),Qt::Horizontal,"Суб код");


    ui->tableView_articles->setModel(modelArticles);
    ui->tableView_articles->setColumnHidden(0, true);    // Скрываем колонку с id записей
    ui->tableView_articles->setEditTriggers(QAbstractItemView::NoEditTriggers);  //запрет редактирования
    ui->tableView_articles->setSelectionBehavior(QAbstractItemView::SelectRows); // Разрешаем выделение строк
    ui->tableView_articles->setSelectionMode(QAbstractItemView::SingleSelection); // Устанавливаем режим выделения лишь одно строки в таблице
    ui->tableView_articles->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_articles->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // по содержимому

    // настриаваем маписн на поля редактирования вопроса
    mapper->setModel(modelArticles);

    mapper->addMapping(ui->lineEdit_id, modelArticles->fieldIndex("id"));
    mapper->addMapping(ui->lineEdit_article, modelArticles->fieldIndex("article"));
    mapper->addMapping(ui->lineEdit_code, modelArticles->fieldIndex("code"));
    mapper->addMapping(ui->lineEdit_subcode, modelArticles->fieldIndex("subcode"));
    mapper->addMapping(ui->lineEdit_f14, modelArticles->fieldIndex("f14"));
    mapper->addMapping(ui->plainTextEdit_note, modelArticles->fieldIndex("note"));
    mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);


    //Таблица расшифровок
    modelBank_decryption->setQuery("SELECT * FROM bank_decryption",base);
//    modelBank_decryption->setQuery("SELECT counterparties.counterparty, ROUND(sum,2), bank.payment_date, bank.payment_number, expense_confirmation, bank.decryption_of_payment  FROM bank_decryption inner join bank on bank_decryption.bank_id=bank.id inner join counterparties on bank.counterparty_id=counterparties.id ORDER BY counterparties.counterparty",base);

    // названия колонок
    modelBank_decryption->setHeaderData(0,Qt::Horizontal,"Контрагент");
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

void FormArticles::Tune()
{

    // смена контракта
    if (modelArticles->data(modelArticles->index(ui->tableView_articles->currentIndex().row(), 0)).toString().isEmpty()) {
        //если пустая запись
        qDebug() << "данные пустые, но маппер не умеет очищать виджеты, оставим на будущее!";

    }

        //доп фильтр если не пустой
        QString ss = "";
        if (!ui->lineEdit_flt_dec->text().isEmpty()) {
            ss = QString("AND (sum Like '\%%1\%' OR bank.payment_date Like '\%%1\%' OR bank.payment_number Like '\%%1\%' OR counterparties.counterparty Like '\%%1\%' OR bank.decryption_of_payment Like '\%%1\%')").arg(ui->lineEdit_flt_dec->text()); // ЛОХ
//            qDebug() << "подстройка фильтра" << ss;
        }
        //запрос
//        QString ff = QString("SELECT counterparties.counterparty, sum, bank.payment_date, bank.payment_number, bank.decryption_of_payment, expense_confirmation  FROM bank_decryption inner join bank on bank_decryption.bank_id=bank.id inner join counterparties on bank.counterparty_id=counterparties.id WHERE article_id = \%1 \%2 ORDER BY counterparties.counterparty").arg(modelArticles->data(modelArticles->index(ui->tableView_articles->currentIndex().row(), 0)).toString()).arg(ss);
        QString ff = QString("SELECT counterparties.counterparty, ROUND(sum,2), bank.payment_date, bank.payment_number, bank.decryption_of_payment, expense_confirmation  FROM bank_decryption inner join bank on bank_decryption.bank_id=bank.id inner join counterparties on bank.counterparty_id=counterparties.id WHERE article_id = \%1 \%2 ORDER BY counterparties.counterparty, bank.payment_date");
        ff=ff.arg(modelArticles->data(modelArticles->index(ui->tableView_articles->currentIndex().row(), 0)).toString()).arg(ss);
//        qDebug() << ff;
        modelBank_decryption->setQuery(ff,base);

        // при изменение строки в таблвьюве устанавливаем маппер на соответствующую запись
        mapper->setCurrentIndex(ui->tableView_articles->currentIndex().row());

        // расчет итога

        QSqlQuery query(base);

        QString zz = QString("SELECT round(SUM(sum),2) FROM bank_decryption inner join bank on bank_decryption.bank_id=bank.id inner join counterparties on bank.counterparty_id=counterparties.id WHERE article_id = \%1 \%2");
        zz=zz.arg(modelArticles->data(modelArticles->index(ui->tableView_articles->currentIndex().row(), 0)).toString()).arg(ss);
        if(!query.exec(zz)) {
           qDebug() << "ERROR SELECT summ_decryption: " << query.lastError().text();
           return;
        }
        query.first();
    //    ui->lineEdit_b_sum->setText(QString::number(query.value(0).toDouble(),'g',20));
//        ui->lineEdit__sum_dec->setText(query.value(0).toString());
        ui->lineEdit__sum_dec->setText(QString("%L1").arg(query.value(0).toDouble(),-0,'f',2));


}

void FormArticles::on_lineEdit_flt_all_textChanged(const QString &arg1)
{
    //фильтр c\статей
    if (!arg1.isEmpty()) {
        QString ff = QString("articles.article Like '\%%1\%' OR articles.code Like '\%%1\%' OR articles.subcode Like '\%%1\%'").arg(arg1);

        modelArticles->setFilter(ff);
        modelArticles->select();
        ui->tableView_articles->selectRow(0);

        Tune(); // дергаем сменой строки принудительно на случай пустого результата

    }
    else {
        modelArticles->setFilter("");
        modelArticles->select();
        ui->tableView_articles->selectRow(0);
    }
}

void FormArticles::on_pushButton_flt_clr_clicked()
{
    ui->lineEdit_flt_all->setText("");

    modelArticles->setFilter("");
    modelArticles->select();
    ui->tableView_articles->selectRow(0);
}

void FormArticles::on_lineEdit_flt_dec_textChanged(const QString &arg1)
{
    // перевыбрать расшифровку
    Tune();

}

void FormArticles::on_pushButton_clr_flt_dec_clicked()
{
    ui->lineEdit_flt_dec->setText("");

    // перевыбрать расшифровку
    Tune();
}

void FormArticles::on_pushButton_first_clicked()
{
    // перая запись
    ui->tableView_articles->selectRow(0);
}

void FormArticles::on_pushButton_prev_clicked()
{
    // прыгаем на предыдущую запись
    ui->tableView_articles->selectRow(ui->tableView_articles->currentIndex().row()-1);
}

void FormArticles::on_pushButton_next_clicked()
{
    // прыгаем на следующую запись
    ui->tableView_articles->selectRow(ui->tableView_articles->currentIndex().row()+1);
}

void FormArticles::on_pushButton_last_clicked()
{
    // последняя запись
    ui->tableView_articles->selectRow(modelArticles->rowCount()-1);
}

void FormArticles::on_pushButton_refr_clicked()
{
    //восстановление курсора
    int row = ui->tableView_articles->currentIndex().row();

    //обновить по простому
    modelArticles->select();
//    modelBank_decryption->setQuery(modelBank_decryption->query().lastQuery(),base);
    // восстанавливаем строку
    ui->tableView_articles->selectRow(row);
}

void FormArticles::on_pushButton_add_clicked()
{
    modelArticles->submit(); // субмитим

    // добавление
    int row= ui->tableView_articles->currentIndex().row()+1; // выбираем следующую

    // вставляем
    modelArticles->insertRow(row);
    modelArticles->setData(modelArticles->index(row,modelArticles->fieldIndex("article")),""); // добавляем пустой для возможности сабмита

    modelArticles->submit(); // субмитим
    // устанавливаем курсор на строку редактирования
    ui->tableView_articles->selectRow(row);
    // устанавливаем курсор на редактирование имени
    ui->lineEdit_article->setFocus();
}

void FormArticles::on_pushButton_del_clicked()
{
    // удаление
    // подтверждение
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Внимание!"),
                                                 tr("Уверены в удалении статьи?")))  return;
    // если есть связи не удаляем!
    QSqlQuery query(base);

    QString tt = QString("SELECT id FROM bank_decryption WHERE article_id = \%1").arg(modelArticles->data(modelArticles->index(ui->tableView_articles->currentIndex().row(), 0)).toString());
    query.exec(tt);
    if (query.first()) {
        qDebug() << "Имеются связанные расшифровки. Удаление невозможно!";
        QMessageBox::critical(this,"Error","Имеются связанные расшифровки. Удаление невозможно!");
        return;
    }

    modelArticles->removeRow(ui->tableView_articles->currentIndex().row());
    // прыгаем на предыдущую запись
    ui->tableView_articles->selectRow(ui->tableView_articles->currentIndex().row()-1);
}

void FormArticles::on_pushButton_lst_clicked()
{
    // запрос на список полный платежей со статьями
    // запрос не редактирован - переделать на правильный!
//       emit signalFromQuery("SELECT articles.article, bank.payment_date, bank.payment_number, counterparties.counterparty, ROUND(bank.amount_of_payment,2) AS summa_bank, ROUND(sum,2) AS summa_description, bank.decryption_of_payment FROM bank_decryption inner join bank on bank_decryption.bank_id=bank.id inner join counterparties on bank.counterparty_id=counterparties.id inner join articles on bank_decryption.article_id=articles.id ORDER BY articles.article, bank.payment_date, counterparties.counterparty");
    // фильтр или по строке фильтра или по выбранномой статье
    QString flt= "";
    QString sID = modelArticles->data(modelArticles->index(ui->tableView_articles->currentIndex().row(), 0)).toString();

    if (ui->lineEdit_flt_dec->text().isEmpty())
        flt= QString(" WHERE article_id = \%1").arg(sID);
    else
        flt = QString("WHERE article_id = \%1 AND (sum Like '\%%2\%' OR bank.payment_date Like '\%%2\%' OR bank.payment_number Like '\%%2\%' OR counterparties.counterparty Like '\%%2\%' OR bank.decryption_of_payment Like '\%%2\%')").arg(sID).arg(ui->lineEdit_flt_dec->text());

    qDebug() << flt;

    emit signalFromQuery(QString("SELECT articles.article, bank.payment_date, bank.payment_number, counterparties.counterparty, ROUND(bank.amount_of_payment,2) AS summa_bank, ROUND(sum,2) AS summa_description, bank.decryption_of_payment FROM bank_decryption inner join bank on bank_decryption.bank_id=bank.id inner join counterparties on bank.counterparty_id=counterparties.id inner join articles on bank_decryption.article_id=articles.id  \%1 ORDER BY articles.article, bank.payment_date, counterparties.counterparty").arg(flt));
}
