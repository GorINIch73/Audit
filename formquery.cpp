#include "formquery.h"
#include "ui_formquery.h"

#include <QDebug>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>
#include <QClipboard>
#include <QAction>
#include <QMenu>
#include <QFile>
#include <QFileDialog>
#include <QSqlRecord>

FormQuery::FormQuery(QSqlDatabase db, QString sq, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormQuery)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    base=db;
    squery=sq;

    //создание обьектов таблиц
    model = new QSqlQueryModel(this);


    //create contextmenu
    copyAction = new QAction(tr("&Копировать"), this);
    copyAction->setStatusTip(tr("Копировать в буфер обмена"));
//    copyAction->setShortcut(tr("Ctrl+C"));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(aCopy()));

    saveAction = new QAction(tr("&Сохранить"), this);
    saveAction->setStatusTip(tr("Сохранить в TSV"));
//    saveAction->setShortcut(tr("Ctrl+S"));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(aSave()));

    //Настраиваем модели
    SetupTable();

    //контекстное меню
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, SIGNAL(customContextMenuRequested( const QPoint& )),
            this, SLOT(customContextMenu( const QPoint& )));


    ui->tableView->selectRow(0);
}

FormQuery::~FormQuery()
{
    delete ui;
}

void FormQuery::SetupTable()
{
    //настройка запроса
    if(!base.isOpen() || squery.isEmpty()) {
        qDebug() << "База не открыта или запрос пустой!";
        return;
    }
    //Таблица
    model->setQuery(squery,base);
    if (!model->lastError().text().isEmpty())
            qDebug() << model->lastError().text();

    // названия колонок
//    modelBank_decryption->setHeaderData(0,Qt::Horizontal,"Статья");
//    modelBank_decryption->setHeaderData(1,Qt::Horizontal,"Сумма");
//    modelBank_decryption->setHeaderData(2,Qt::Horizontal,"Дата платежа");
//    modelBank_decryption->setHeaderData(3,Qt::Horizontal,"Номер платежа");
//    modelBank_decryption->setHeaderData(4,Qt::Horizontal,"Назначение платежа");
//    modelBank_decryption->setHeaderData(5,Qt::Horizontal,"Подтверждение расхода");

    ui->tableView->setModel(model);
//    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);  //запрет редактирования
//      ui->tableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
      ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
//    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows); // Разрешаем выделение строк
//    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection); // Устанавливаем режим выделения лишь одно строки в таблице
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // по содержимому



}

void FormQuery::on_pushButton_close_clicked()
{
    close();
}


void FormQuery::on_pushButton_first_clicked()
{
    // перая запись
    ui->tableView->selectRow(0);
}

void FormQuery::on_pushButton_prev_clicked()
{
    // прыгаем на предыдущую запись
    ui->tableView->selectRow(ui->tableView->currentIndex().row()-1);
}

void FormQuery::on_pushButton_next_clicked()
{
    // прыгаем на следующую запись
    ui->tableView->selectRow(ui->tableView->currentIndex().row()+1);
}

void FormQuery::on_pushButton_last_clicked()
{
    // последняя запись
    ui->tableView->selectRow(model->rowCount()-1);
}

void FormQuery::on_pushButton_refr_clicked()
{
    int row = ui->tableView->currentIndex().row();
    model->setQuery(model->query().lastQuery(),base);
    // восстанавливаем строку
    ui->tableView->selectRow(row);
}

void FormQuery::customContextMenu(const QPoint &)
{
    QMenu menu(this);
    menu.addAction(copyAction);
    menu.addAction(saveAction);
    menu.exec(QCursor::pos());
}


void FormQuery::aCopy()
{
       qDebug() << "Copy";
        // странно работает! переделать!!
       QAbstractItemModel * model = ui->tableView->model();
       QItemSelectionModel * selection = ui->tableView->selectionModel();
       QModelIndexList indexes = selection->selectedIndexes();

       QString selected_text;
       // Запоминаем первое значение
       QModelIndex previous = indexes.first();
        // записываем первое значение - оно всегда есть
       selected_text.append(model->data(previous).toString());
       //удаляем первое значение из последующей обработки
       indexes.removeFirst();
       foreach(const QModelIndex &current, indexes)
       {

           // если та же строка, что и предыдущая, то ставим табуляцию иначе конец строки
           if (current.row() != previous.row())
           {
               selected_text.append('\n');
           }
           else
           {
               selected_text.append('\t');
           }

           // добавляем текущую
           selected_text.append(model->data(current).toString());
           // запоминаем
           previous = current;
       }

//        qDebug() << selected_text;
       QClipboard *clipboard = QGuiApplication::clipboard();
       clipboard->setText(selected_text);

}

void FormQuery::aSave()
{
        qDebug() << "Save";



        // запросить новое имя
        QString  exportName = QFileDialog::getSaveFileName(this,tr("Сохранить как"),".//",tr("File TSV (*.tsv)"));
        // возможно надо проверить нет ли уже такого файла

        QFile file(exportName);
          if(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
          {
              file.resize(0); // чистим старый
              // We're going to streaming text to the file
              QTextStream stream(&file);

              int countRow = model->rowCount();
              int countCol = model->columnCount();
            // возможно надо добавить выгрузку заголовков!!!
              for(int i=0;i < countRow; i++)
              {
                  QString ss="";
                  for(int j=0;j < countCol; j++){
                      ss.append(model->record(i).value(j).toString());
                      ss.append('\t');
                  }
                  ss.chop(1);
                  stream << ss << '\n';
              }


              file.close();
              qDebug() << "Writing finished";
          }
          else {
                QMessageBox::critical(this,"ERROR","Ошибка записи в файл!");
                return;
          }

}
