#ifndef FORMCOUNTERPARTIES_H
#define FORMCOUNTERPARTIES_H

#include <QWidget>

#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QDataWidgetMapper>
#include <QItemSelection>


namespace Ui {
class FormCounterparties;
}

class FormCounterparties : public QWidget
{
    Q_OBJECT

public:
    explicit FormCounterparties(QSqlDatabase db, QWidget *parent = nullptr);
    ~FormCounterparties();

private slots:
    void slotSelectionChange(const QItemSelection &current, const QItemSelection &previous);

    void on_pushButton_close_clicked();

    void on_lineEdit_flt_all_textChanged(const QString &arg1);

    void on_pushButton_first_clicked();

    void on_pushButton_prev_clicked();

    void on_pushButton_next_clicked();

    void on_pushButton_last_clicked();

    void on_pushButton_refr_clicked();

    void on_pushButton_add_clicked();

    void on_pushButton_del_clicked();

    void on_pushButton_flt_clr_clicked();

private:
    Ui::FormCounterparties *ui;

    QSqlDatabase base;

    QSqlTableModel *modelCounterparties;
    QSqlQueryModel *modelBank_decryption;
    QDataWidgetMapper *mapper;

    void SetupTable();
    void Tune();
};

#endif // FORMCOUNTERPARTIES_H
