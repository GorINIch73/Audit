#ifndef DIALOGNEWCONTRACT_H
#define DIALOGNEWCONTRACT_H

#include <QDialog>
#include <QSqlDatabase>

namespace Ui {
class DialogNewContract;
}

class DialogNewContract : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewContract(QSqlDatabase db, QString decryption, QWidget *parent = nullptr);
    ~DialogNewContract();

    QString getDate();
    QString getNumber();
    bool getState();

private slots:
    void on_buttonBox_accepted();
    void Decrypt(QString decryption);

    void on_lineEditNumber_textChanged(const QString &arg1);
    void setColor();

private:
    Ui::DialogNewContract *ui;

    QString date;
    QString number;
    bool state;
    QSqlDatabase base;

};

#endif // DIALOGNEWCONTRACT_H
