#ifndef FORMBANK_H
#define FORMBANK_H



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
class FormBank;
}

class FormBank : public QWidget
{
    Q_OBJECT

public:
    explicit FormBank(QSqlDatabase db, QWidget *parent = nullptr);
    ~FormBank();

signals:

    void signalFromQuery(QString sq); // сигнал для запроса


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

    void on_lineEdit_flt_all_textChanged(const QString &arg1);

    void on_pushButton_flt_clr_clicked();

    void on_pushButton_add_dec_clicked();

    void on_lineEdit_flt_art_textChanged(const QString &arg1);

    void on_pushButton_article_add_dec_clicked();

    void on_pushButton_clear_dec_clicked();

    void on_lineEdit_flt_num_textChanged(const QString &arg1);


    void on_pushButton_next_con_clicked();

    void on_pushButton_prev_con_clicked();

    void on_comboBox_flt_counterparties_currentIndexChanged(int index);

    void on_pushButton_contracts_add_new_clicked();

    void on_pushButton_contracts_add_dec_clicked();

    void on_pushButton_contracts_clear_dec_clicked();

    void on_pushButton_del_dec_clicked();

//    void on_comboBox_counterparty_editTextChanged(const QString &arg1);

    void on_pushButton_article_repl_dec_clicked();

    void on_checkBox_flt_nodec_stateChanged(int arg1);


    void on_pushButton_clearCnt_clicked();

    void on_pushButton_rep_b_clicked();

    void on_pushButton_Rep_Err_clicked();

    void on_pushButton_rep_nd_clicked();

    void on_checkBox_flt_noContr_stateChanged(int arg1);

    void on_pushButton_noContr_clicked();

    void on_FormBank_destroyed();

    void on_pushButton_rep_p_clicked();

private:
    Ui::FormBank *ui;

    QSqlDatabase base;
    QSqlRelationalTableModel *modelBank;
    QSqlRelationalTableModel *modelBank_decryption;
    QSqlTableModel *modelArticles;
    QSqlTableModel *modelCounterparties;
//    QSqlTableModel *modelContracts;
    QSqlQueryModel *modelContracts; // попробуем по другому
    QDataWidgetMapper *mapper;
//    QSqlRelationalDelegateFlt *delegate;
    QSqlRelationalDelegate *delegate;
    QSqlRelationalDelegateFlt *a_delegate;
    QCompleter *completer;
    QCompleter *completer_articles;
    QCompleter *completer_counterparties;
    QCompleter *completer_contracts;

    void SetupTable();
    void TunBank_decryption();
};

#endif // FORMBANK_H
