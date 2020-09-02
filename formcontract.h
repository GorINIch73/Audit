#ifndef FORMCONTRACT_H
#define FORMCONTRACT_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QDataWidgetMapper>
#include <QCompleter>


#include "qsqlrelationaldelegateflt.h"

namespace Ui {
class FormContract;
}

class FormContract : public QWidget
{
    Q_OBJECT

public:
    explicit FormContract(QSqlDatabase db,QWidget *parent = nullptr);
    ~FormContract();


signals:

    void signalFromQuery(QString sq); // сигнал для запроса


private slots:

    void slotSelectionChange(const QItemSelection &current, const QItemSelection &previous);

    void on_pushButton_close_clicked();

    void on_pushButton_first_clicked();

    void on_pushButton_prev_clicked();

    void on_pushButton_next_clicked();

    void on_pushButton_last_clicked();

    void on_pushButton_refr_clicked();

    void on_pushButton_add_clicked();

    void on_pushButton_del_clicked();

    void on_pushButton_flt_clr_clicked();

    void on_lineEdit_flt_all_textChanged(const QString &arg1);

    void on_pushButton_prev_con_clicked();

    void on_pushButton_next_con_clicked();

    void on_comboBox_flt_counterparties_currentIndexChanged(int index);

    void on_pushButton_rep_list_clicked();

    void on_pushButton_rep_for_audit_clicked();

private:
    Ui::FormContract *ui;

    QSqlDatabase base;

    QSqlRelationalTableModel *modelContracts;
    QSqlQueryModel *modelBank_decryption;
    QSqlQueryModel *modelCounterparties;
    QDataWidgetMapper *mapper;
    QSqlRelationalDelegate *delegate;
    QCompleter *completer_counterparties;
    QCompleter *completer_flt_counterparties;

    void SetupTable();

    void seekTable();

};

#endif // FORMCONTRACT_H
