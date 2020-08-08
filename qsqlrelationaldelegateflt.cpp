#include "qsqlrelationaldelegateflt.h"

#include <QCompleter>
#include <QtDebug>
#include <QSqlRecord>
#include <QSqlField>

QSqlRelationalDelegateFlt::QSqlRelationalDelegateFlt(QObject * parent) : QSqlRelationalDelegate(parent)
{

}

QWidget *QSqlRelationalDelegateFlt::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    // стащил из родителя
    // выковыриваем модель
    const QSqlRelationalTableModel *sqlModel = qobject_cast<const QSqlRelationalTableModel *>(index.model());
    // выковыриваем связанную модель
    QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : nullptr;

    if (!childModel)
        return QSqlRelationalDelegate::createEditor(parent, option, index);

    const QSqlDriver *const driver = childModel->database().driver();

    // настраиваем комбобокс
    QComboBox *combo = new QComboBox(parent);
    combo->setModel(childModel);
    // получаем номер колонки
    QString fn = sqlModel->relation(index.column()).displayColumn();
    QString stripped = driver->isIdentifierEscaped(fn, QSqlDriver::FieldName) // хз что оно делает
            ? driver->stripDelimiters(fn, QSqlDriver::FieldName)
            : fn;

    combo->setModelColumn(childModel->fieldIndex(stripped));
    //combo->installEventFilter(const_cast<QSqlFLTRelationalDelegate *>(this)); // хз что это ))
    combo->setEditable(true);
    // настраиваем комплитер
    combo->insertItem(0,QString::fromUtf8(NULL)); // добавляем пустой элемент
    combo->setCurrentIndex(combo->findText(sqlModel->data(sqlModel->index(index.row(),index.column())).toString()));  // руками проставляем нужный индекс

    QCompleter *mycompletear = new QCompleter(parent);
    mycompletear->setCaseSensitivity(Qt::CaseInsensitive);
    mycompletear->setFilterMode(Qt::MatchContains);
//    mycompletear->setCompletionMode(QCompleter::InlineCompletion);

    mycompletear->setModel(childModel);
    mycompletear->setCompletionColumn(1);
    combo->setCompleter(mycompletear);


    return combo;

}


