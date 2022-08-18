#include "qsqlrelationaldelegateflt.h"

#include <QCompleter>
#include <QtDebug>
#include <QSqlRecord>
#include <QSqlField>
#include <QSpinBox>

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
    //если нет связи
    if (!childModel) {
        // если это сумма, то отдать спинер - сделано влоб, пока никак!
        if (index.column()==sqlModel->fieldIndex("sum")) {
            QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
            editor->setFrame(false);
            editor->setMinimum(-10000000000.00);
            editor->setMaximum(10000000000.00);
            editor->setDecimals(2);
            editor->setGroupSeparatorShown(true);

            return editor;
        }

        return QSqlRelationalDelegate::createEditor(parent, option, index);
    }

    //обновляем на случай изменения
    childModel->select();

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
    combo->installEventFilter(const_cast<QSqlRelationalDelegateFlt *>(this)); // хз что это ))
    combo->setEditable(true);
    combo->insertItem(0,QString::fromUtf8(NULL)); // добавляем пустой элемент // иногда дает удалить элемент иногда нет хз почему

//    combo->setMinimumWidth(120); // чтоб не схлопывалась

    // настраиваем комплитер
    combo->setCurrentIndex(combo->findText( sqlModel->data(sqlModel->index(index.row(),index.column())).toString()));  // руками проставляем нужный индекс

    QCompleter *mycompletear = new QCompleter(parent);
    mycompletear->setCaseSensitivity(Qt::CaseInsensitive);
    mycompletear->setFilterMode(Qt::MatchContains);
//    mycompletear->setFilterMode(Qt::MatchContains);
//  //    mycompletear->setCompletionMode(QCompleter::InlineCompletion);
// //    mycompletear->setCompletionMode(QCompleter::UnfilteredPopupCompletion);

    mycompletear->setModel(childModel);
    mycompletear->setCompletionColumn(childModel->fieldIndex(stripped));
    combo->setCompleter(mycompletear);


    return combo;

}


