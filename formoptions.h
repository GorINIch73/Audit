#ifndef FORMOPTIONS_H
#define FORMOPTIONS_H

#include <QWidget>
#include <QSqlDatabase>

namespace Ui {
class FormOptions;
}

class FormOptions : public QWidget
{
    Q_OBJECT

public:
    explicit FormOptions(QSqlDatabase db,QWidget *parent = nullptr);
    ~FormOptions();

private slots:
    void on_pushButton_close_clicked();

    void on_pushButton_clearBase_clicked();

    void on_checkBox_counterparties_stateChanged(int arg1);

    void on_checkBox_bank_stateChanged(int arg1);

private:
    Ui::FormOptions *ui;
    QSqlDatabase base;
};

#endif // FORMOPTIONS_H
