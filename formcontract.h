#ifndef FORMCONTRACT_H
#define FORMCONTRACT_H

#include <QWidget>

namespace Ui {
class FormContract;
}

class FormContract : public QWidget
{
    Q_OBJECT

public:
    explicit FormContract(QWidget *parent = nullptr);
    ~FormContract();

private:
    Ui::FormContract *ui;
};

#endif // FORMCONTRACT_H
