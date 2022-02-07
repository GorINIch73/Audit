#ifndef QSQLRELATIONALDELEGATEFLT_H
#define QSQLRELATIONALDELEGATEFLT_H

#include <QSqlRelationalDelegate>


//
// Класс делегат для редактирования ввязанной таблицы в комбобоксе с комплитером
//
class QSqlRelationalDelegateFlt : public QSqlRelationalDelegate
{
public:
    QSqlRelationalDelegateFlt(QObject * parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

};

#endif // QSQLRELATIONALDELEGATEFLT_H
