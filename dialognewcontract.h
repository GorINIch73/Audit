#ifndef DIALOGNEWCONTRACT_H
#define DIALOGNEWCONTRACT_H

#include <QDialog>

namespace Ui {
class DialogNewContract;
}

class DialogNewContract : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewContract(QWidget *parent = nullptr);
    ~DialogNewContract();

private:
    Ui::DialogNewContract *ui;
};

#endif // DIALOGNEWCONTRACT_H
