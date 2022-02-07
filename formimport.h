#ifndef FORMIMPORT_H
#define FORMIMPORT_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQuery>

//extern void aRunImport(QSqlDatabase db,);

namespace Ui {
class FormImport;
}

class FormImport : public QWidget
{
    Q_OBJECT

public:
    explicit FormImport(QSqlDatabase db, QWidget *parent = nullptr);
    ~FormImport();

private slots:

    void on_pushButton_getFile_clicked();

    void on_pushButton_close_clicked();


    void on_pushButton_ImportZ_clicked();


    void on_checkBox_recovery_stateChanged(int arg1);

private:
    Ui::FormImport *ui;

    QSqlDatabase base;
    QString importName;


};

#endif // FORMIMPORT_H
