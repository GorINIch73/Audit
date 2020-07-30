#ifndef FORMBANK_H
#define FORMBANK_H



#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QDataWidgetMapper>
#include <QCompleter>


#include "qsqlrelationaldelegateflt.h"

namespace Ui {
class FormBank;
}

class FormBank : public QWidget
{
    Q_OBJECT

public:
    explicit FormBank(QSqlDatabase db, QWidget *parent = nullptr);
    ~FormBank();

private slots:
    void on_pushButton_close_clicked();
    void slotSelectionChange(const QItemSelection &current, const QItemSelection &previous);

    void on_comboBox_counterparty_currentIndexChanged(int index);

    void on_pushButton_first_clicked();

    void on_pushButton_prev_clicked();

    void on_pushButton_next_clicked();

    void on_pushButton_last_clicked();

    void on_pushButton_refr_clicked();

    void on_pushButton_add_clicked();

    void on_pushButton_del_clicked();

private:
    Ui::FormBank *ui;

    QSqlDatabase base;
    QSqlRelationalTableModel *modelBank;
    QSqlRelationalTableModel *modelBank_decryption;
    QDataWidgetMapper *mapper;
    QSqlRelationalDelegate *delegate;
    QSqlRelationalDelegateFlt *a_delegate;
    QCompleter *completer;

    void SetupTable();
    void TunBank_decryption();
};

#endif // FORMBANK_H
