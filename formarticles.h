#ifndef FORMARTICLES_H
#define FORMARTICLES_H

#include <QWidget>

namespace Ui {
class FormArticles;
}

class FormArticles : public QWidget
{
    Q_OBJECT

public:
    explicit FormArticles(QWidget *parent = nullptr);
    ~FormArticles();

private:
    Ui::FormArticles *ui;
};

#endif // FORMARTICLES_H
