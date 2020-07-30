#ifndef FORMCOUNTERPARTIES_H
#define FORMCOUNTERPARTIES_H

#include <QWidget>

namespace Ui {
class FormCounterparties;
}

class FormCounterparties : public QWidget
{
    Q_OBJECT

public:
    explicit FormCounterparties(QWidget *parent = nullptr);
    ~FormCounterparties();

private:
    Ui::FormCounterparties *ui;
};

#endif // FORMCOUNTERPARTIES_H
