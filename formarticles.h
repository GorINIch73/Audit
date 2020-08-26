#ifndef FORMARTICLES_H
#define FORMARTICLES_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QDataWidgetMapper>
#include <QItemSelection>


namespace Ui {
class FormArticles;
}

class FormArticles : public QWidget
{
    Q_OBJECT

public:
    explicit FormArticles(QSqlDatabase db,QWidget *parent = nullptr);
    ~FormArticles();


signals:

    void signalFromQuery(QString sq); // сигнал для запроса

private slots:
    void slotSelectionChange(const QItemSelection &current, const QItemSelection &previous);

    void on_pushButton_close_clicked();

    void on_pushButton_flt_clr_clicked();

    void on_pushButton_clr_flt_dec_clicked();

    void on_lineEdit_flt_all_textChanged(const QString &arg1);

    void on_lineEdit_flt_dec_textChanged(const QString &arg1);

    void on_pushButton_first_clicked();

    void on_pushButton_prev_clicked();

    void on_pushButton_next_clicked();

    void on_pushButton_last_clicked();

    void on_pushButton_refr_clicked();

    void on_pushButton_add_clicked();

    void on_pushButton_del_clicked();

    void on_pushButton_lst_clicked();

private:
    Ui::FormArticles *ui;

    QSqlDatabase base;
    QSqlTableModel *modelArticles;
    QSqlQueryModel *modelBank_decryption;
    QDataWidgetMapper *mapper;

    void SetupTable();
    void Tune();

};

#endif // FORMARTICLES_H
