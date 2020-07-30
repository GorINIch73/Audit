#ifndef FORMOPTIONS_H
#define FORMOPTIONS_H

#include <QWidget>

namespace Ui {
class FormOptions;
}

class FormOptions : public QWidget
{
    Q_OBJECT

public:
    explicit FormOptions(QWidget *parent = nullptr);
    ~FormOptions();

private:
    Ui::FormOptions *ui;
};

#endif // FORMOPTIONS_H
