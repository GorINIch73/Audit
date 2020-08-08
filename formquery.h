#ifndef FORMQUERY_H
#define FORMQUERY_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>

namespace Ui {
class FormQuery;
}

class FormQuery : public QWidget
{
    Q_OBJECT

public:
    explicit FormQuery(QSqlDatabase db, QString sq, QWidget *parent = nullptr);
    ~FormQuery();

private slots:
    void on_pushButton_close_clicked();

    void on_pushButton_first_clicked();

    void on_pushButton_prev_clicked();

    void on_pushButton_next_clicked();

    void on_pushButton_last_clicked();

    void on_pushButton_refr_clicked();


    void customContextMenu(const QPoint& );

    void aCopy();
    void aSave();


private:
    Ui::FormQuery *ui;

    QSqlDatabase base;
    QSqlQueryModel *model;
    QString squery;

    QAction *copyAction;
    QAction *saveAction;

    void SetupTable();
};

#endif // FORMQUERY_H
